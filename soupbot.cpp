#include "Globals.h"
#include "bncsutil/bncsutil.h"

nls_t *NLS;
extern sockaddr_in sName;
WNDPROC wpOrigRichTextProc, wpOrigListViewProc, wpOrigEditBoxProc;

BOOL CALLBACK AcctCreateDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool result = false;
	char ComparePassword[128];

	switch(uMsg){
	case WM_CLOSE:
		{
			EndDialog(hDlg, 0);
			return FALSE;
		}
	case WM_INITDIALOG:
		{	
			char Note[128];
			sprintf(Note, "The account you have specified ('%s') does not exist. What would you like to do?", szLocalAccountInfo.szUsername);
			SetDlgItemText(hDlg, IDC_CREATE_ACCT_NOTE, Note);
			break;
		}
	case WM_COMMAND:
		{
			switch(wParam)
			{
			case IDC_CREATE_ACCT_CREATE:
				{
					GetDlgItemText(hDlg, IDC_CREATE_ACCT_PASSWORD, ComparePassword, sizeof(ComparePassword));
					if(!strcmp(szLocalAccountInfo.szPassword, ComparePassword)) {
						SendCreateAccount();
						result = true;
						EndDialog(hDlg, 0);
					} else {
						CloseTCPSocket(sckBNCS);
						szLocalAccountInfo.Connected = false;
						AppendText(hBNChat, RED, "Passwords did not match\nConnection closed\n");
						result = false;
						EndDialog(hDlg, 0);
					}
					return result;
					break;
				}
			case IDC_CREATE_ACCT_CANCEL:
				{
					CloseTCPSocket(sckBNCS);
					szLocalAccountInfo.Connected = false;
					AppendText(hBNChat, RED, "Connection closed\n");
					result = false;
					EndDialog(hDlg, 0);
					return result;
					break;
				}
			}
			break;
		}
	}
	return result;
}

BOOL CALLBACK ProfileDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_CLOSE:
		{
			EndDialog(hDlg, 0);
			return TRUE;
		}
	case WM_INITDIALOG:
		{	
			if(strcmp(szProfileData.szAccount, szLocalAccountInfo.szUsername))
				EnableWindow(GetDlgItem(hDlg, IDC_PROFILE_OK), FALSE);

			SetDlgItemText(hDlg, IDC_PROFILE_NAME, szProfileData.szAccount);
			SetDlgItemText(hDlg, IDC_PROFILE_SEX, szProfileData.szSex);
			SetDlgItemText(hDlg, IDC_PROFILE_LOCATION, szProfileData.szLocation);
			SetDlgItemText(hDlg, IDC_PROFILE_DESCRIPTION, szProfileData.szDescription);
			break;
		}
	case WM_COMMAND:
		{
			switch(wParam)
			{
			case IDC_PROFILE_OK:
				{
					bool changed = false;
					char sex[512], location[512], description[512];
					GetDlgItemText(hDlg, IDC_PROFILE_SEX, sex, 511);
					GetDlgItemText(hDlg, IDC_PROFILE_LOCATION, location, 511);
					GetDlgItemText(hDlg, IDC_PROFILE_DESCRIPTION, description, 511);

					if(strcmp(sex, szProfileData.szSex))
						changed = true;
					if(strcmp(location, szProfileData.szLocation))
						changed = true;
					if(strcmp(description, szProfileData.szDescription))
						changed = true;

					if(changed)
						SetProfile((char *)sex, (char *)location, (char *)description);

					EndDialog(hDlg, 0);
					break;
				}
			case IDC_PROFILE_CANCEL:
				{
					EndDialog(hDlg, 0);
					break;
				}
			}
			break;
		}
	}
	return FALSE;
}

LRESULT CALLBACK EditBoxSubclass(HWND EditBox, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_KEYUP:
		if(wParam == VK_RETURN){
			DoSendText();
			return 0;
		}
	case WM_KEYDOWN:
		if(wParam == VK_RETURN)
			return 0;
	case WM_CHAR:
		if(wParam == VK_RETURN)
			return 0;
	}
	return CallWindowProc(wpOrigEditBoxProc, EditBox, uMsg, wParam, lParam);
}

LRESULT APIENTRY ListViewSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
    if (uMsg == WM_GETDLGCODE) 
        return DLGC_WANTALLKEYS;  

	switch(uMsg) {
	case WM_INITDIALOG:
		{
			ShowScrollBar(GetDlgItem(hMainDlg, IDC_CHANLIST), SB_HORZ, FALSE);
			break;
		}
	case WM_CONTEXTMENU:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			HMENU hmMenu, hmSubMenu;
			hmMenu = LoadMenu( hDlgInst, MAKEINTRESOURCE( IDR_POPUP2 ));
			hmSubMenu = GetSubMenu( hmMenu, 0 );

			BOOL nRetID = TrackPopupMenuEx( hmSubMenu, TPM_LEFTALIGN |
			TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD |
			TPM_RIGHTBUTTON, x, y, hMainDlg, NULL );

			int selected = ListView_GetSelectionMark(GetDlgItem(hMainDlg, IDC_CHANLIST));
			if(selected == -1)
				return CallWindowProc(wpOrigListViewProc, hwnd, uMsg, 
									  wParam, lParam);

			char SelectedText[35];
			ListView_GetItemText(GetDlgItem(hMainDlg, IDC_CHANLIST), selected, 0, SelectedText, sizeof(SelectedText));
			if(strlen(SelectedText) <= 1)
				return CallWindowProc(wpOrigListViewProc, hwnd, uMsg, 
									  wParam, lParam);

			switch(nRetID) {
			case ID_POPUP2_SQUELCH:
				Send(sckBNCS, "/squelch %s", SelectedText);
				break;
			case ID_POPUP2_UNSQUELCH:
				Send(sckBNCS, "/unsquelch %s", SelectedText);
				break;
			case ID_POPUP2_WHOIS:
				Send(sckBNCS, "/whois %s", SelectedText);
				break;
			case ID_POPUP2_KICK:
				Send(sckBNCS, "/kick %s (Kicked)", SelectedText);
				break;
			case ID_POPUP2_BAN:
				Send(sckBNCS, "/ban %s (Banned)", SelectedText);
				break;
			case ID_POPUP2_DES:
				Send(sckBNCS, "/designate %s", SelectedText);
				break;
			case ID_POPUP2_PROFILE:
				GetProfile(SelectedText);
				break;
			case ID_POPUP2_RECORDDATA:
				GetRecordData(SelectedText);
				break;
			}

			DestroyMenu(hmSubMenu);
			DestroyMenu(hmMenu);
			break;
		}
	}

    return CallWindowProc(wpOrigListViewProc, hwnd, uMsg, 
        wParam, lParam); 
} 

LRESULT APIENTRY RichTextSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
    if (uMsg == WM_GETDLGCODE) 
        return DLGC_WANTALLKEYS;  

	switch(uMsg) {
	case WM_CONTEXTMENU:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);

			HMENU hmMenu, hmSubMenu;
			hmMenu = LoadMenu( hDlgInst, MAKEINTRESOURCE( IDR_POPUP ));
			hmSubMenu = GetSubMenu( hmMenu, 0 );

			BOOL nRetID = TrackPopupMenuEx( hmSubMenu, TPM_LEFTALIGN |
			TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD |
			TPM_RIGHTBUTTON, x, y, hMainDlg, NULL );

			switch(nRetID) {
			case ID_POPUP_COPY:
				SendDlgItemMessage(hMainDlg, IDC_BNCHAT, WM_COPY, 0, 0);
				break;
			case ID_POPUP_QUOTES:
				{
					CHARRANGE range;
					SendMessage(hBNChat, EM_EXGETSEL, NULL, (LPARAM)&range);
					LONG length = range.cpMax - range.cpMin;
					char *buffer = new char[length + 1];
					if (!buffer)
					  return 0;
					LONG lResult = SendMessage(hBNChat, EM_GETSELTEXT, NULL, (LPARAM)buffer);
					if (lResult != length)
					  return lResult;

					AddQuote(buffer);

					delete[] buffer;
				break;
				}
			case ID_POPUP_SENDQUOTE:
				{
					SendRandomQuote2();
					break;
				}
			case ID_POPUP_CLEAR:
				SendDlgItemMessage(hMainDlg, IDC_BNCHAT, WM_SETTEXT, 0, (LPARAM)"");
				break;
			case ID_POPUP_REJOIN:
				if(szLocalAccountInfo.Connected) {
					SendBNCSPacket(sckBNCS, SID_LEAVECHAT);
					dBuf.add((int)2);
					dBuf.add(szLocalAccountInfo.szCurrentChan);
					SendBNCSPacket(sckBNCS, SID_JOINCHANNEL);
				}
				break;
			}

			DestroyMenu(hmSubMenu);
			DestroyMenu(hmMenu);
			break;
		}
	}

    return CallWindowProc(wpOrigRichTextProc, hwnd, uMsg, 
        wParam, lParam); 
} 

BOOL CALLBACK MiscDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	case WM_INITDIALOG:
		{	
			char temp[64];
			sprintf(temp, "%d", szLocalAccountInfo.IdleTimer);
			SendDlgItemMessage(hDlg, IDC_MISC_USEIDLE, BM_SETCHECK, (WPARAM)(szLocalAccountInfo.IdleOn ? BST_CHECKED : BST_UNCHECKED), 0);
			SetDlgItemText(hDlg, IDC_MISC_IDLETIME, temp);
			SetDlgItemText(hDlg, IDC_MISC_IDLEMSG, szLocalAccountInfo.IdleMessage);
			if(szLocalAccountInfo.IdleQuotes)
				SendDlgItemMessage(hDlg, IDC_MISC_IDLEQUOTES, BM_SETCHECK, (LPARAM)BST_CHECKED, 0);
			if(szLocalAccountInfo.IdleSpec)
				SendDlgItemMessage(hDlg, IDC_MISC_IDLESPEC, BM_SETCHECK, (LPARAM)BST_CHECKED, 0);
			SendMessage(GetDlgItem(hDlg, IDC_MISC_IDLEMSG), EM_SETREADONLY, szLocalAccountInfo.IdleQuotes, NULL);
			SetDlgItemText(hDlg, IDC_MISC_STARTUPMSG, szLocalAccountInfo.StartupMsg);
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (wParam)
			{
				case IDC_MISC_IDLEQUOTES:
					SendMessage(GetDlgItem(hDlg, IDC_MISC_IDLEMSG), EM_SETREADONLY, TRUE, NULL);
					break;
				case IDC_MISC_IDLESPEC:
					SendMessage(GetDlgItem(hDlg, IDC_MISC_IDLEMSG), EM_SETREADONLY, FALSE, NULL);
					break;
			}
			switch(LOWORD(wParam))
			{
				case IDC_MISC_CLOSE:
					EndDialog(hDlg, 0);
					break;
				case IDC_MISC_SETSTART:
					{
						CRegisterWIN32* Save = new CRegisterWIN32;
						Save->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Miscellaneous", FALSE);
							GetDlgItemText(hDlg, IDC_MISC_STARTUPMSG, szLocalAccountInfo.StartupMsg, sizeof(szLocalAccountInfo.StartupMsg));
							Save->Write("Startup Message", szLocalAccountInfo.StartupMsg);
						Save->Close();
						delete Save;
						break;
					}
				case IDC_MISC_SETIDLE:
					{
						char temp[128];
						int idle;
						CRegisterWIN32* Save = new CRegisterWIN32;
						Save->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Miscellaneous\\Idle", FALSE);
							GetDlgItemText(hDlg, IDC_MISC_IDLEMSG, temp, sizeof(temp));
							strcpy(szLocalAccountInfo.IdleMessage, temp);
							Save->Write("Message", temp);

							GetDlgItemText(hDlg, IDC_MISC_IDLETIME, temp, sizeof(temp));
							sscanf(temp, "%i", &idle);
							szLocalAccountInfo.IdleTimer = idle;
							Save->WriteDword("Idle delay", idle);

							switch(IsDlgButtonChecked(hDlg, IDC_MISC_USEIDLE))
							{
							case BST_CHECKED:
								szLocalAccountInfo.IdleOn = true;
								break;
							case BST_UNCHECKED:
								szLocalAccountInfo.IdleOn = false;
								break;
							default:
								szLocalAccountInfo.IdleOn = true;
								break;
							}
							Save->WriteDword("Use Idle", szLocalAccountInfo.IdleOn);

							switch(IsDlgButtonChecked(hDlg, IDC_MISC_IDLESPEC))
							{
							case BST_CHECKED:
								szLocalAccountInfo.IdleSpec = true;
								szLocalAccountInfo.IdleQuotes = false;
								break;
							case BST_UNCHECKED:
								szLocalAccountInfo.IdleSpec = false;
								szLocalAccountInfo.IdleQuotes = true;
								break;
							default:
								szLocalAccountInfo.IdleSpec = true;
								szLocalAccountInfo.IdleQuotes = false;
								break;
							}
							Save->WriteDword("Use Message", szLocalAccountInfo.IdleSpec);
							Save->WriteDword("Use Quotes", szLocalAccountInfo.IdleQuotes);
						Save->Close();
						delete Save;
						IdleTicker = (szLocalAccountInfo.IdleTimer / 30);
						break;
					}
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CALLBACK FilePathDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	case WM_INITDIALOG:
		{
		SendDlgItemMessage(hDlg, IDC_PATHS_GAME, CB_ADDSTRING, 0, (LPARAM)"Starcraft/Brood War");
		SendDlgItemMessage(hDlg, IDC_PATHS_GAME, CB_ADDSTRING, 0, (LPARAM)"Warcraft II");
		SendDlgItemMessage(hDlg, IDC_PATHS_GAME, CB_ADDSTRING, 0, (LPARAM)"Diablo II");
		SendDlgItemMessage(hDlg, IDC_PATHS_GAME, CB_ADDSTRING, 0, (LPARAM)"Lord of Destruction");
		SendDlgItemMessage(hDlg, IDC_PATHS_GAME, CB_ADDSTRING, 0, (LPARAM)"Warcraft III");
		SendDlgItemMessage(hDlg, IDC_PATHS_GAME, CB_ADDSTRING, 0, (LPARAM)"The Frozen Throne");
		SendDlgItemMessage(hDlg, IDC_PATHS_GAME, CB_SELECTSTRING, -1, (LPARAM)"Diablo II");
		CRegisterWIN32* Regist = new CRegisterWIN32;
		Regist->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\D2DV", TRUE);
			SetDlgItemText(hDlg, IDC_PATHS_EXE, Regist->Read("Game EXE", "Game EXE"));
			SetDlgItemText(hDlg, IDC_PATHS_DLL, Regist->Read("Game DLL", "Game DLL"));
			SetDlgItemText(hDlg, IDC_PATHS_SNP, Regist->Read("Game SNP", "Game SNP"));
		Regist->Close();
		delete Regist;
		return TRUE;
		}
	case WM_COMMAND:
		switch(HIWORD(wParam))
		{
			case CBN_SELCHANGE:
				CRegisterWIN32* Paths = new CRegisterWIN32;
				char szGameName[128];
				
				GetDlgItemText(hDlg, IDC_PATHS_GAME, szGameName, sizeof(szGameName));
				if(!strcmp(szGameName, "Starcraft/Brood War"))
				{
					Paths->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\STAR", TRUE);
						SetDlgItemText(hDlg, IDC_PATHS_EXE, Paths->Read("Game EXE", "Game EXE"));
						SetDlgItemText(hDlg, IDC_PATHS_DLL, Paths->Read("Game DLL", "Game DLL"));
						SetDlgItemText(hDlg, IDC_PATHS_SNP, Paths->Read("Game SNP", "Game SNP"));
					Paths->Close();
				}
				if(!strcmp(szGameName, "Diablo II"))
				{
					Paths->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\D2DV", TRUE);
						SetDlgItemText(hDlg, IDC_PATHS_EXE, Paths->Read("Game EXE", "Game EXE"));
						SetDlgItemText(hDlg, IDC_PATHS_DLL, Paths->Read("Game DLL", "Game DLL"));
						SetDlgItemText(hDlg, IDC_PATHS_SNP, Paths->Read("Game SNP", "Game SNP"));
					Paths->Close();
				}
				if(!strcmp(szGameName, "Warcraft II"))
				{
					Paths->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\W2BN", TRUE);
						SetDlgItemText(hDlg, IDC_PATHS_EXE, Paths->Read("Game EXE", "Game EXE"));
						SetDlgItemText(hDlg, IDC_PATHS_DLL, Paths->Read("Game DLL", "Game DLL"));
						SetDlgItemText(hDlg, IDC_PATHS_SNP, Paths->Read("Game SNP", "Game SNP"));
					Paths->Close();
				}
				if(!strcmp(szGameName, "Lord of Destruction"))
				{
					Paths->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\D2XP", TRUE);
						SetDlgItemText(hDlg, IDC_PATHS_EXE, Paths->Read("Game EXE", "Game EXE"));
						SetDlgItemText(hDlg, IDC_PATHS_DLL, Paths->Read("Game DLL", "Game DLL"));
						SetDlgItemText(hDlg, IDC_PATHS_SNP, Paths->Read("Game SNP", "Game SNP"));
					Paths->Close();
				}
				if(!strcmp(szGameName, "Warcraft III"))
				{
					Paths->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\WAR3", TRUE);
						SetDlgItemText(hDlg, IDC_PATHS_EXE, Paths->Read("Game EXE", "Game EXE"));
						SetDlgItemText(hDlg, IDC_PATHS_DLL, Paths->Read("Game DLL", "Game DLL"));
						SetDlgItemText(hDlg, IDC_PATHS_SNP, Paths->Read("Game SNP", "Game SNP"));
					Paths->Close();
				}
				if(!strcmp(szGameName, "The Frozen Throne"))
				{
					Paths->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\W3XP", TRUE);
						SetDlgItemText(hDlg, IDC_PATHS_EXE, Paths->Read("Game EXE", "Game EXE"));
						SetDlgItemText(hDlg, IDC_PATHS_DLL, Paths->Read("Game DLL", "Game DLL"));
						SetDlgItemText(hDlg, IDC_PATHS_SNP, Paths->Read("Game SNP", "Game SNP"));
					Paths->Close();
				}

				delete Paths;
				break;
		}

		switch(LOWORD(wParam))
		{
		case IDC_PATHS_BROWSE_EXE:
			{
			char szPath[256];
			char szGame[32];
			GetDlgItemText(hDlg, IDC_PATHS_GAME, szGame, sizeof(szGame));
			GetFilePath(szPath, szGame, "EXE");
			SetDlgItemText(hDlg, IDC_PATHS_EXE, szPath);
			break;
			}
		case IDC_PATHS_BROWSE_DLL:
			{
			char szPath[256];
			char szGame[32];
			GetDlgItemText(hDlg, IDC_PATHS_GAME, szGame, sizeof(szGame));
			GetFilePath(szPath, szGame, "DLL");
			SetDlgItemText(hDlg, IDC_PATHS_DLL, szPath);
			break;
			}
		case IDC_PATHS_BROWSE_SNP:
			{
			char szPath[256];
			char szGame[32];
			GetDlgItemText(hDlg, IDC_PATHS_GAME, szGame, sizeof(szGame));
			GetFilePath(szPath, szGame, "SNP");
			SetDlgItemText(hDlg, IDC_PATHS_SNP, szPath);
			break;
			}
		case IDC_PATHS_CLOSE:
			EndDialog(hDlg, 0);
			break;
		case IDC_PATHS_SAVE:
			char szGameName[32];
			char szGameAbbrv[12];
			char szRegPath[64];
			char szGameDLL[128], szGameSNP[128], szGameEXE[128];
			
			GetDlgItemText(hDlg, IDC_PATHS_EXE, szGameEXE, sizeof(szGameEXE));
			GetDlgItemText(hDlg, IDC_PATHS_DLL, szGameDLL, sizeof(szGameDLL));
			GetDlgItemText(hDlg, IDC_PATHS_SNP, szGameSNP, sizeof(szGameDLL));

			CRegisterWIN32* Save = new CRegisterWIN32;
			GetDlgItemText(hDlg, IDC_PATHS_GAME, szGameName, sizeof(szGameName));
			GameNameToAbbrv(szGameName, szGameAbbrv);
			sprintf(szRegPath, "Software\\Beta Productions\\SoupBot2\\File Paths\\%s", szGameAbbrv);
			Save->Open(HKEY_CURRENT_USER, szRegPath, FALSE);
				Save->Write("Game DLL", szGameDLL);
				Save->Write("Game EXE", szGameEXE);
				Save->Write("Game SNP", szGameSNP);
			Save->Close();
			delete Save;
			break;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK CdkeyDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	case WM_INITDIALOG:
		{
			SendDlgItemMessage(hDlg, IDC_CDKEYS_SC, BM_SETCHECK, (LPARAM)BST_CHECKED, 0);
			WriteCDkeyList(hDlg, IDC_CDKEYS_CDKEY);
		return TRUE;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_CDKEYS_CLOSE:
				EndDialog(hDlg, 0);
				break;
			case IDC_CDKEYS_DELETE:
				{
					char szCDKey[32];
					int CDKeyType = FindCheckedRadioButton();

					GetDlgItemText(hDlg, IDC_CDKEYS_CDKEY, szCDKey, sizeof(szCDKey));
					
					switch(CDKeyType)
					{
					case CDKEY_STAR:
						DeleteCDkey(CDkeyExists(szCDKey, "STAR"), "STAR");
						break;
					case CDKEY_W2BN:
						DeleteCDkey(CDkeyExists(szCDKey, "W2BN"), "W2BN");
						break;
					case CDKEY_D2DV:
						DeleteCDkey(CDkeyExists(szCDKey, "D2DV"), "D2DV");
						break;
					case CDKEY_D2XP:
						DeleteCDkey(CDkeyExists(szCDKey, "D2XP"), "D2XP");
						break;
					case CDKEY_WAR3:
						DeleteCDkey(CDkeyExists(szCDKey, "WAR3"), "WAR3");
						break;
					case CDKEY_W3XP:
						DeleteCDkey(CDkeyExists(szCDKey, "W3XP"), "W3XP");
						break;
					}
					SetDlgItemText(hDlg, IDC_CDKEYS_CDKEY, "");
					WriteCDkeyList(hDlg, IDC_CDKEYS_CDKEY);
				break;
				}
			case IDC_CDKEYS_ADD:
				{
				char szCDKey[32];
				int CDKeyType = FindCheckedRadioButton();
				
				GetDlgItemText(hDlg, IDC_CDKEYS_CDKEY, szCDKey, sizeof(szCDKey));

				switch(CDKeyType)
				{
				case CDKEY_STAR:
					{
					CRegisterWIN32* Add = new CRegisterWIN32;
					char szTempString[128];
					int x = 1;
					int lenofstr = 0;
					int numCdkeys = 1;
						Add->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\STAR", FALSE);
							for(numCdkeys = 1; x <= 10; numCdkeys++)
							{
								sprintf(szTempString, "CDKey#%i", numCdkeys);
								lenofstr = strlen(Add->Read(szTempString, "none"));
								if(lenofstr < 5)
								{
									x = 11;
								} else {
									x++;
								}
							}
							Add->Write(szTempString, szCDKey);
						Add->Close();
					delete Add;
					break;
					}
				case CDKEY_W2BN:
					{
					CRegisterWIN32* Add = new CRegisterWIN32;
					char szTempString[128];
					int x = 1;
					int lenofstr = 0;
					int numCdkeys = 1;
						Add->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\W2BN", FALSE);
							for(numCdkeys = 1; x <= 10; numCdkeys++)
							{
								sprintf(szTempString, "CDKey#%i", numCdkeys);
								lenofstr = strlen(Add->Read(szTempString, "none"));
								if(lenofstr < 5)
								{
									x = 11;
								} else {
									x++;
								}
							}
							Add->Write(szTempString, szCDKey);
						Add->Close();
					delete Add;
					break;
					}
				case CDKEY_D2DV:
					{
					CRegisterWIN32* Add = new CRegisterWIN32;
					char szTempString[128];
					int x = 1;
					int lenofstr = 0;
					int numCdkeys = 1;
						Add->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\D2DV", FALSE);
							for(numCdkeys = 1; x <= 10; numCdkeys++)
							{
								sprintf(szTempString, "CDKey#%i", numCdkeys);
								lenofstr = strlen(Add->Read(szTempString, "none"));
								if(lenofstr < 5)
								{
									x = 11;
								} else {
									x++;
								}
							}
							Add->Write(szTempString, szCDKey);
						Add->Close();
					delete Add;
					break;
					}
				case CDKEY_D2XP:
					{
					CRegisterWIN32* Add = new CRegisterWIN32;
					char szTempString[128];
					int x = 1;
					int lenofstr = 0;
					int numCdkeys = 1;
						Add->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\D2XP", FALSE);
							for(numCdkeys = 1; x <= 10; numCdkeys++)
							{
								sprintf(szTempString, "CDKey#%i", numCdkeys);
								lenofstr = strlen(Add->Read(szTempString, "none"));
								if(lenofstr < 5)
								{
									x = 11;
								} else {
									x++;
								}
							}
							Add->Write(szTempString, szCDKey);
						Add->Close();
					delete Add;
					break;
					}
				case CDKEY_WAR3:
					{
					CRegisterWIN32* Add = new CRegisterWIN32;
					char szTempString[128];
					int x = 1;
					int lenofstr = 0;
					int numCdkeys = 1;
						Add->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\WAR3", FALSE);
							for(numCdkeys = 1; x <= 10; numCdkeys++)
							{
								sprintf(szTempString, "CDKey#%i", numCdkeys);
								lenofstr = strlen(Add->Read(szTempString, "none"));
								if(lenofstr < 5)
								{
									x = 11;
								} else {
									x++;
								}
							}
							Add->Write(szTempString, szCDKey);
						Add->Close();
					delete Add;
					break;
					}
				case CDKEY_W3XP:
					{
					CRegisterWIN32* Add = new CRegisterWIN32;
					char szTempString[128];
					int x = 1;
					int lenofstr = 0;
					int numCdkeys = 1;
						Add->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\W3XP", FALSE);
							for(numCdkeys = 1; x <= 10; numCdkeys++)
							{
								sprintf(szTempString, "CDKey#%i", numCdkeys);
								lenofstr = strlen(Add->Read(szTempString, "none"));
								if(lenofstr < 5)
								{
									x = 11;
								} else {
									x++;
								}
							}
							Add->Write(szTempString, szCDKey);
						Add->Close();
					delete Add;
					break;
					}
				}
				SetDlgItemText(hDlg, IDC_CDKEYS_CDKEY, "");
				WriteCDkeyList(hDlg, IDC_CDKEYS_CDKEY);
				break;
				}
				
			}
		return TRUE;
		}
	}
	return FALSE;
}

BOOL CALLBACK RenameProfileDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	case WM_INITDIALOG:
		{
			char szTempString[32];
				HWND ConfigDialogHandle = FindWindow(NULL, "Character Setup");
				GetWindowText(GetDlgItem(ConfigDialogHandle, IDC_CONFIG_PROFILE_NAME), szTempString, sizeof(szTempString));
				if(strlen(szTempString) >= 1) 
				{ 
					SetWindowText(GetDlgItem(hDlg, IDC_PROFILE_NAME), szTempString); 
					SetWindowText(GetDlgItem(hDlg, IDC_PROFILE_EDIT_NAME), szTempString);
				}	
		return TRUE;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_PROFILE_CANCEL:
				EndDialog(hDlg, 0);
				break;
			case IDC_PROFILE_SETNAME:
				char szTempString[32];
				char szTempString2[32];
					GetWindowText(GetDlgItem(hDlg, IDC_PROFILE_EDIT_NAME), szTempString, sizeof(szTempString));
					if(ProfileExists(szTempString) != 0)
					{ 
						MessageBox(hDlg, "That profile already exists.\nPlease choose a new name.", "Notice", MB_OK);
						SetFocus(GetDlgItem(hDlg, IDC_PROFILE_EDIT_NAME));
						break;
					}
					GetWindowText(GetDlgItem(hDlg, IDC_PROFILE_NAME), szTempString2, sizeof(szTempString2));
					RenameProfile((strstr(szTempString, ") ") ? (szTempString+4) : szTempString), (strstr(szTempString2, ") ") ? (szTempString2+4) : szTempString2));
					HWND ConfigDialogHandle = FindWindow(NULL, "Character Setup");
					SendDlgItemMessage(ConfigDialogHandle, IDC_CONFIG_PROFILE_NAME, CB_DELETESTRING, 0, (LPARAM)szTempString2);
					SendDlgItemMessage(ConfigDialogHandle, IDC_CONFIG_PROFILE_NAME, CB_ADDSTRING, 0, (LPARAM)szTempString);
					SendDlgItemMessage(ConfigDialogHandle, IDC_CONFIG_PROFILE_NAME, CB_SELECTSTRING, -1, (LPARAM)szTempString);
					EndDialog(hDlg, 0);
				break;
			}
		return TRUE;
		}
	}
	return FALSE;
}

BOOL CALLBACK NewProfileDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	case WM_INITDIALOG:
		{
			char szTempString[32];
				HWND ConfigDialogHandle = FindWindow(NULL, "Character Setup");
				GetWindowText(GetDlgItem(ConfigDialogHandle, IDC_CONFIG_PROFILE_NAME), szTempString, sizeof(szTempString));
				if(strlen(szTempString) >= 1) 
				{ 
					SetWindowText(GetDlgItem(hDlg, IDC_PROFILE_NAME), szTempString); 
					SetWindowText(GetDlgItem(hDlg, IDC_PROFILE_EDIT_NAME), szTempString);
				}	
		return TRUE;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_PROFILE_CANCEL:
				EndDialog(hDlg, 0);
				break;
			case IDC_PROFILE_SETNAME:
				char szTempString[32];
				char szTempString2[32];
					GetWindowText(GetDlgItem(hDlg, IDC_PROFILE_EDIT_NAME), szTempString, sizeof(szTempString));
					if(ProfileExists(szTempString) != 0)
					{ 
						MessageBox(hDlg, "That profile already exists.\nPlease choose a new name.", "Notice", MB_OK);
						SetFocus(GetDlgItem(hDlg, IDC_PROFILE_EDIT_NAME));
						break;
					}
					sprintf(szTempString2, "[Unsaved] %s", szTempString);
					HWND ConfigDialogHandle = FindWindow(NULL, "Character Setup");
					SendDlgItemMessage(ConfigDialogHandle, IDC_CONFIG_PROFILE_NAME, CB_ADDSTRING, 0, (LPARAM)szTempString2);
					SendDlgItemMessage(ConfigDialogHandle, IDC_CONFIG_PROFILE_NAME, CB_SELECTSTRING, -1, (LPARAM)szTempString2);
					SendDlgItemMessage(ConfigDialogHandle, IDC_CONFIG_CDKEY, CB_SELECTSTRING, -1, (LPARAM)"SC/BW Keys:");
					SetWindowText(GetDlgItem(ConfigDialogHandle, IDC_CONFIG_NAME), "Username");
					SetWindowText(GetDlgItem(ConfigDialogHandle, IDC_CONFIG_PASS), "Password");
					SetWindowText(GetDlgItem(ConfigDialogHandle, IDC_CONFIG_HOME), "Home Channel");
					SetWindowText(GetDlgItem(ConfigDialogHandle, IDC_CONFIG_SERVER), "Server");
					SetWindowText(GetDlgItem(ConfigDialogHandle, IDC_CONFIG_GAME), "Starcraft Retail");
					SetWindowText(GetDlgItem(ConfigDialogHandle, IDC_CONFIG_EMAIL), "user@betaproductions.net");
				EndDialog(hDlg, 0);
				break;
			}
		return TRUE;
		}
	}
	return FALSE;
}

BOOL CALLBACK ConfigureDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	case WM_INITDIALOG:
		{
			char szTempChar[128], szTempChar2[128];
			int ProfileNumb;
			SendDlgItemMessage( hDlg, IDC_CONFIG_GAME, CB_ADDSTRING, 0, (LPARAM)"Diablo II" );
			SendDlgItemMessage( hDlg, IDC_CONFIG_GAME, CB_ADDSTRING, 0, (LPARAM)"Diablo II LoD" );
			SendDlgItemMessage( hDlg, IDC_CONFIG_GAME, CB_ADDSTRING, 0, (LPARAM)"Starcraft Retail" );
			SendDlgItemMessage( hDlg, IDC_CONFIG_GAME, CB_ADDSTRING, 0, (LPARAM)"Starcraft Broodwar" );
			SendDlgItemMessage( hDlg, IDC_CONFIG_GAME, CB_ADDSTRING, 0, (LPARAM)"Warcraft II BNE" );
			SendDlgItemMessage( hDlg, IDC_CONFIG_GAME, CB_ADDSTRING, 0, (LPARAM)"Warcraft III" );
			SendDlgItemMessage( hDlg, IDC_CONFIG_GAME, CB_ADDSTRING, 0, (LPARAM)"Warcraft III TFT" );
			SendDlgItemMessage( hDlg, IDC_CONFIG_SERVER, CB_ADDSTRING, 0, (LPARAM)"useast.battle.net" );
			SendDlgItemMessage( hDlg, IDC_CONFIG_SERVER, CB_ADDSTRING, 0, (LPARAM)"uswest.battle.net" );
			SendDlgItemMessage( hDlg, IDC_CONFIG_SERVER, CB_ADDSTRING, 0, (LPARAM)"europe.battle.net" );
			SendDlgItemMessage( hDlg, IDC_CONFIG_SERVER, CB_ADDSTRING, 0, (LPARAM)"asia.battle.net" );
			EnableWindow(GetDlgItem(hDlg, IDC_CONFIG_EXPCDKEY), false);
			WriteCDkeyList(hDlg, IDC_CONFIG_CDKEY);
			WriteExpCDkeyList(hDlg, IDC_CONFIG_EXPCDKEY);
			LoadProfiles();
			CRegisterWIN32* Registry = new CRegisterWIN32;
				Registry->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2", TRUE);
					ProfileNumb = Registry->ReadInt("Last Profile", 1);
				Registry->Close();

				sprintf(szTempChar, "Software\\Beta Productions\\SoupBot2\\Profiles\\Profile%i", ProfileNumb);
				
				if(!Registry->Open(HKEY_CURRENT_USER, szTempChar, TRUE))
				{
					return TRUE;
				}

				sprintf(szTempChar, "(%i)", ProfileNumb);
				SendDlgItemMessage(hDlg, IDC_CONFIG_PROFILE_NAME, CB_SELECTSTRING, -1, (LPARAM)szTempChar);
				GetWindowText(GetDlgItem(hDlg, IDC_CONFIG_PROFILE_NAME), szTempChar, sizeof(szTempChar));
				sprintf(szTempChar2, "Software\\Beta Productions\\SoupBot2\\Profiles\\Profile%i", ProfileExists((strstr(szTempChar, ") ") ? (szTempChar+4) : szTempChar)));
				
					SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_NAME), Registry->Read("Username", "Default"));
					SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_PASS), Registry->Read("Password", "Default"));
					SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_HOME), Registry->Read("Home Channel", "Default"));
					SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_SERVER), Registry->Read("Server", "Default"));
					SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_EMAIL), Registry->Read("Email", "Default"));
					GameIndexToName(szTempChar, Registry->ReadInt("Game Type", 0), false);
					SendDlgItemMessage(hDlg, IDC_CONFIG_GAME, CB_SELECTSTRING, -1, (LPARAM)szTempChar);
					SendDlgItemMessage(hDlg, IDC_CONFIG_CDKEY, CB_SELECTSTRING, -1, (LPARAM)Registry->Read("CD Key"));
					if(Registry->ReadInt("Game Type", 0) == 1 || Registry->ReadInt("Game Type", 0) == 6) {
						EnableWindow(GetDlgItem(hDlg, IDC_CONFIG_EXPCDKEY), true);
						SendDlgItemMessage(hDlg, IDC_CONFIG_EXPCDKEY, CB_SELECTSTRING, -1, (LPARAM)Registry->Read("ExpCD Key"));
					}
					SendDlgItemMessage(hDlg, IDC_CONFIG_SPAWN, BM_SETCHECK, (WPARAM)(Registry->ReadInt("CD Spawn") ? BST_CHECKED : BST_UNCHECKED), 0);
				Registry->Close();
			delete Registry;
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch(HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				switch(LOWORD(wParam))
				{
				case IDC_CONFIG_GAME:
					{
						char szTempString[32];
						GetWindowText(GetDlgItem(hDlg, IDC_CONFIG_GAME), szTempString, sizeof(szTempString));

						if(GameNameToIndex(szTempString) == 1 || GameNameToIndex(szTempString) == 6) {
								EnableWindow(GetDlgItem(hDlg, IDC_CONFIG_EXPCDKEY), true);
						} else {
							EnableWindow(GetDlgItem(hDlg, IDC_CONFIG_EXPCDKEY), false);
						}
						break;
					}
				case IDC_CONFIG_PROFILE_NAME:
					{
						char szTempString[32], szTempString2[32];
						GetWindowText(GetDlgItem(hDlg, IDC_CONFIG_PROFILE_NAME), szTempString, sizeof(szTempString));
						if(ProfileExists((strstr(szTempString, ") ") ? (szTempString+4) : szTempString)) == 0) { break; }
						sprintf(szTempString2, "Software\\Beta Productions\\SoupBot2\\Profiles\\Profile%i", ProfileExists((strstr(szTempString, ") ") ? (szTempString+4) : szTempString)));
						EnableWindow(GetDlgItem(hDlg, IDC_CONFIG_EXPCDKEY), false);

						CRegisterWIN32* Registry = new CRegisterWIN32;
						Registry->Open(HKEY_CURRENT_USER, szTempString2, TRUE);
							SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_NAME), Registry->Read("Username", "Default"));
							SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_PASS), Registry->Read("Password", "Default"));
							SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_HOME), Registry->Read("Home Channel", "Default"));
							SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_SERVER), Registry->Read("Server", "Default"));
							SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_EMAIL), Registry->Read("Email", "Default"));
							GameIndexToName(szTempString, Registry->ReadInt("Game Type", 0), false);
							SendDlgItemMessage(hDlg, IDC_CONFIG_GAME, CB_SELECTSTRING, -1, (LPARAM)szTempString);
							SendDlgItemMessage(hDlg, IDC_CONFIG_CDKEY, CB_SELECTSTRING, -1, (LPARAM)Registry->Read("CD Key", "SC/BW KEYS:"));
							if(Registry->ReadInt("Game Type", 0) == 1 || Registry->ReadInt("Game Type", 0) == 6) {
								EnableWindow(GetDlgItem(hDlg, IDC_CONFIG_EXPCDKEY), true);
								SendDlgItemMessage(hDlg, IDC_CONFIG_EXPCDKEY, CB_SELECTSTRING, -1, (LPARAM)Registry->Read("ExpCD Key", "THE FROZEN THRONE KEYS:"));
							}
							SendDlgItemMessage(hDlg, IDC_CONFIG_SPAWN, BM_SETCHECK, (WPARAM)(Registry->ReadInt("CD Spawn") ? BST_CHECKED : BST_UNCHECKED), 0);
						Registry->Close();
						delete Registry;
						break;
					}
				}
				break;
			}

			switch(LOWORD(wParam))
			{
			case IDC_CONFIG_NEWPROFILE:
				CreateDialog(hDlgInst, MAKEINTRESOURCE(IDD_CREATE_PROFILE), hMainDlg, NewProfileDialogProc);
				break;
			case IDC_CONFIG_RENAMEPROFILE:
				CreateDialog(hDlgInst, MAKEINTRESOURCE(IDD_CREATE_PROFILE), hMainDlg, RenameProfileDialogProc);
				break;
			case IDC_CONFIG_DELETEPROFILE:
				char szTempString[32];
				GetWindowText(GetDlgItem(hDlg, IDC_CONFIG_PROFILE_NAME), szTempString, sizeof(szTempString));
				DeleteProfile(szTempString);
				break;
			case IDC_CONFIG_CLOSE:
				EndDialog(hDlg, 0);
				break;
			case IDC_CONFIG_UNDOCHANGES:
				{
				char szTempString[32], szTempString2[32];
				GetWindowText(GetDlgItem(hDlg, IDC_CONFIG_PROFILE_NAME), szTempString, sizeof(szTempString));
				if(ProfileExists((strstr(szTempString, ") ") ? (szTempString+4) : szTempString)) == 0) { break; }
				sprintf(szTempString2, "Software\\Beta Productions\\SoupBot2\\Profiles\\Profile%i", ProfileExists((strstr(szTempString, ") ") ? (szTempString+4) : szTempString)));
				
				CRegisterWIN32* Registry = new CRegisterWIN32;
				Registry->Open(HKEY_CURRENT_USER, szTempString2, FALSE);
					SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_NAME), Registry->Read("Username", "Default"));
					SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_PASS), Registry->Read("Password", "Default"));
					SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_HOME), Registry->Read("Home Channel", "Default"));
					SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_SERVER), Registry->Read("Server", "Default"));
					SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_EMAIL), Registry->Read("Email", "Default"));
					GameIndexToName(szTempString, Registry->ReadInt("Game Type", 0), false);
					SendDlgItemMessage(hDlg, IDC_CONFIG_GAME, CB_SELECTSTRING, -1, (LPARAM)szTempString);				
				Registry->Close();
				delete Registry;
				break;
				}
			case IDC_CONFIG_SAVECLOSE:
				{
				char szTempChar[32];
				char szUsername[32], szPassword[32], szHome[32], szServer[32], szProfName[32];
				char szGameName[32], szCDkey[32], szExpCDkey[32], szEmailAddy[128];
				bool Spawn = FALSE;
				GetDlgItemText(hDlg, IDC_CONFIG_PROFILE_NAME, szProfName, sizeof(szProfName));
				if(strstr(szProfName, "[Unsaved]"))
				{
					sscanf(szProfName, "[Unsaved] %s", szTempChar);
					strcpy(szProfName, szTempChar);
				}
				if(strstr(szProfName, ") "))
				{
					strcpy(szProfName, szProfName+4);
				}
				GetDlgItemText(hDlg, IDC_CONFIG_NAME, szUsername, sizeof(szUsername));
				GetDlgItemText(hDlg, IDC_CONFIG_PASS, szPassword, sizeof(szPassword));
				GetDlgItemText(hDlg, IDC_CONFIG_HOME, szHome, sizeof(szHome));
				GetDlgItemText(hDlg, IDC_CONFIG_SERVER, szServer, sizeof(szServer));
				GetDlgItemText(hDlg, IDC_CONFIG_GAME, szGameName, sizeof(szGameName));
				GetDlgItemText(hDlg, IDC_CONFIG_CDKEY, szCDkey, sizeof(szCDkey));
				GetDlgItemText(hDlg, IDC_CONFIG_EMAIL, szEmailAddy, sizeof(szEmailAddy));
				strcpy(szExpCDkey, "none");
				if(GameNameToIndex(szGameName) == 1 || 6) {
					GetDlgItemText(hDlg, IDC_CONFIG_EXPCDKEY, szExpCDkey, sizeof(szExpCDkey));
				}
				switch(IsDlgButtonChecked(hDlg, IDC_CONFIG_SPAWN))
				{
				case BST_CHECKED:
					Spawn = TRUE;
					break;
				case BST_UNCHECKED:
					Spawn = FALSE;
					break;
				default:
					Spawn = FALSE;
					break;
				}
				SaveProfile(szUsername, szPassword, szHome, szServer, szProfName, GameNameToIndex(szGameName), szCDkey, szExpCDkey, szEmailAddy, Spawn);
				strcpy(szLocalAccountInfo.szUsername, szUsername);
				strcpy(szLocalAccountInfo.szPassword, szPassword);
				strcpy(szLocalAccountInfo.szHomeChannel, szHome);
				strcpy(szLocalAccountInfo.szServer, szServer);
				strcpy(szLocalAccountInfo.szProfileName, szProfName);
				strcpy(szLocalAccountInfo.szCDkey, szCDkey);
				strcpy(szLocalAccountInfo.szExpCDkey, szExpCDkey);
				strcpy(szLocalAccountInfo.szEmailAddy, szEmailAddy);
				szLocalAccountInfo.Spawn = Spawn;
				szLocalAccountInfo.iGameIndex = GameNameToIndex(szGameName);
				GameIndexToName(szLocalAccountInfo.szGameAbbr, szLocalAccountInfo.iGameIndex, true);
				szLocalAccountInfo.lVerByte = GetVersionByte();
				SetLastProfile(ProfileExists(szProfName));
				EndDialog(hDlg, 0);
				break;
				}
			case IDC_CONFIG_SAVEPROFILE:
				{
				char szTempChar[32];
				char szUsername[32], szPassword[32], szHome[32], szServer[32], szProfName[32];
				char szGameName[32], szCDkey[32], szExpCDkey[32], szEmailAddy[128];
				bool Spawn = FALSE;
				GetDlgItemText(hDlg, IDC_CONFIG_PROFILE_NAME, szProfName, sizeof(szProfName));
				if(strstr(szProfName, "[Unsaved]"))
				{
					sscanf(szProfName, "[Unsaved] %s", szTempChar);
					strcpy(szProfName, szTempChar);
				}
				GetDlgItemText(hDlg, IDC_CONFIG_NAME, szUsername, sizeof(szUsername));
				GetDlgItemText(hDlg, IDC_CONFIG_PASS, szPassword, sizeof(szPassword));
				GetDlgItemText(hDlg, IDC_CONFIG_HOME, szHome, sizeof(szHome));
				GetDlgItemText(hDlg, IDC_CONFIG_SERVER, szServer, sizeof(szServer));
				GetDlgItemText(hDlg, IDC_CONFIG_GAME, szGameName, sizeof(szGameName));
				GetDlgItemText(hDlg, IDC_CONFIG_CDKEY, szCDkey, sizeof(szCDkey));
				GetDlgItemText(hDlg, IDC_CONFIG_EMAIL, szEmailAddy, sizeof(szEmailAddy));
				strcpy(szExpCDkey, "none");
				if(GameNameToIndex(szGameName) == 1 || 6) {
					GetDlgItemText(hDlg, IDC_CONFIG_EXPCDKEY, szExpCDkey, sizeof(szExpCDkey));
				}
				switch(IsDlgButtonChecked(hDlg, IDC_CONFIG_SPAWN))
				{
				case BST_CHECKED:
					Spawn = TRUE;
					break;
				case BST_UNCHECKED:
					Spawn = FALSE;
					break;
				default:
					Spawn = FALSE;
					break;
				}
				SaveProfile(szUsername, szPassword, szHome, szServer, szProfName, GameNameToIndex(szGameName), szCDkey, szExpCDkey, szEmailAddy, Spawn);
				break;
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CALLBACK MainDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_INITDIALOG:
		{
			SendMessage(GetDlgItem(hDlg, IDC_BNCHAT), EM_AUTOURLDETECT, TRUE, 0);

			WSADATA wsaData;
			WSAStartup(0x101, &wsaData);
			szLocalAccountInfo.Connected = false;
			hMainDlg = hDlg;
			hBNChat = GetDlgItem(hDlg, IDC_BNCHAT);
			SendMessage(hBNChat, EM_SETBKGNDCOLOR, 0, (LPARAM)BLACK);

			hListView = GetDlgItem(hDlg, IDC_CHANLIST);
			InitImageList(IDB_ICONS);

			SetFonts();

			wpOrigRichTextProc = (WNDPROC)SetWindowLong(GetDlgItem(hDlg, IDC_BNCHAT), GWL_WNDPROC, (LONG)RichTextSubclass); 
			wpOrigListViewProc = (WNDPROC)SetWindowLong(GetDlgItem(hDlg, IDC_CHANLIST), GWL_WNDPROC, (LONG)ListViewSubclass); 
			wpOrigEditBoxProc = (WNDPROC)SetWindowLong(GetDlgItem(hDlg, IDC_BNSEND), GWL_WNDPROC, (LONG)EditBoxSubclass);

			srand(time(0));
			return TRUE;
			break;
		}
	case WM_SIZE:
		{
			if(wParam == SIZE_MINIMIZED)
				TrayHandler_AddAndMinimize();
			break;
		}
	case WM_SHELLNOTIFY:
		{
			if((lParam == WM_RBUTTONDOWN) || (lParam == WM_LBUTTONDBLCLK))
				TrayHandler_Restore();
			break;
		}
	case WM_CTLCOLOREDIT:
		{
			HBRUSH hBrush;
			hBrush = CreateSolidBrush(RGB(0, 0, 0));
			SetBkColor((HDC)wParam, RGB(0, 0, 0));
            SetTextColor((HDC)wParam, RGB(255, 255, 255));
			return (DWORD)hBrush;
			break;
		}
	case DATA_PENDING_TCP:
		{
			if(LOWORD(lParam) == FD_CLOSE){
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Connection closed\n");

				if(sckBNCS != INVALID_SOCKET)
					CloseTCPSocket(sckBNCS);

				sckBNCS = CreateTCPSocket();
				AppendText(hBNChat, WHITE, "Attempting to reconnect to server %s:6112\n", szLocalAccountInfo.szServer);
				if(ConnectTCPSocket(sckBNCS, szLocalAccountInfo.szServer, 6112)){
					szLocalAccountInfo.Connected = true;
					AppendText(hBNChat, WHITE, "Connected\n");
					WSAAsyncSelect(sckBNCS, hMainDlg, DATA_PENDING_TCP, FD_READ | FD_CLOSE);
					Queue.Clear();
					send(sckBNCS, "\x1", 1, 0);
					SendAuthInfo(sckBNCS);
				} else {
					szLocalAccountInfo.Connected = false;
					AppendText(hBNChat, RED, "Connection closed (Error %d)\n", WSAGetLastError());
				}
				return TRUE;
			}
			char szBuffer[20000];
			int iRecvLen = recv(sckBNCS, szBuffer, sizeof(szBuffer) - 1, 0);
			ParseBNCS(sckBNCS, szBuffer, iRecvLen);
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case ID_CONFIGURATION_FONTSETUP_CHANTEXT:
				{
					CHOOSEFONT cf;
					cf.lStructSize = sizeof(CHOOSEFONT); 
					cf.hwndOwner = hMainDlg; 
					cf.hDC = (HDC)NULL; 
					cf.lpLogFont = &lfChatFont; 
					cf.iPointSize = -200; 
					cf.Flags = CF_SCREENFONTS; 
					cf.rgbColors = RGB(0,0,0); 
					cf.lCustData = 0L; 
					cf.lpfnHook = (LPCFHOOKPROC)NULL; 
					cf.lpTemplateName = (LPSTR)NULL; 
					cf.hInstance = (HINSTANCE) NULL; 
					cf.lpszStyle = (LPSTR)NULL; 
					cf.nFontType = SCREEN_FONTTYPE; 
					cf.nSizeMin = 0; 
					cf.nSizeMax = 0;
					ChooseFont(&cf);

					hfBotFont = CreateFontIndirect(cf.lpLogFont);
					CRegisterWIN32* Registry = new CRegisterWIN32;
					Registry->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Fonts\\Channel text", FALSE);
						lfChatFont.lfCharSet = cf.lpLogFont->lfCharSet;
						lfChatFont.lfClipPrecision = cf.lpLogFont->lfClipPrecision;
						lfChatFont.lfEscapement = cf.lpLogFont->lfEscapement;
						strcpy(lfChatFont.lfFaceName, cf.lpLogFont->lfFaceName);
						lfChatFont.lfHeight = cf.lpLogFont->lfHeight;
						lfChatFont.lfItalic = cf.lpLogFont->lfItalic;
						lfChatFont.lfOrientation = cf.lpLogFont->lfOrientation;
						lfChatFont.lfOutPrecision = cf.lpLogFont->lfOutPrecision;
						lfChatFont.lfPitchAndFamily = cf.lpLogFont->lfPitchAndFamily;
						lfChatFont.lfQuality = cf.lpLogFont->lfQuality;
						lfChatFont.lfStrikeOut = cf.lpLogFont->lfStrikeOut;
						lfChatFont.lfUnderline = cf.lpLogFont->lfUnderline;
						lfChatFont.lfWeight = cf.lpLogFont->lfWeight;
						lfChatFont.lfWidth = cf.lpLogFont->lfWidth;
						
						Registry->WriteDword("CharSet", cf.lpLogFont->lfCharSet);
						Registry->WriteDword("ClipPrecision", cf.lpLogFont->lfClipPrecision);
						Registry->WriteDword("Escapement", cf.lpLogFont->lfEscapement);
						Registry->Write("FaceName", cf.lpLogFont->lfFaceName);
						Registry->WriteDword("Height", cf.lpLogFont->lfHeight);
						Registry->WriteDword("Italic", cf.lpLogFont->lfItalic);
						Registry->WriteDword("Orientation", cf.lpLogFont->lfOrientation);
						Registry->WriteDword("OutPrecision", cf.lpLogFont->lfOutPrecision);
						Registry->WriteDword("PitchAndFamily", cf.lpLogFont->lfPitchAndFamily);
						Registry->WriteDword("Quality", cf.lpLogFont->lfQuality);
						Registry->WriteDword("StrikeOut", cf.lpLogFont->lfStrikeOut);
						Registry->WriteDword("Underline", cf.lpLogFont->lfUnderline);
						Registry->WriteDword("Weight", cf.lpLogFont->lfWeight);
						Registry->WriteDword("Width", cf.lpLogFont->lfWidth);
					Registry->Close();
					delete Registry;
					
					SendMessage(GetDlgItem(hMainDlg, IDC_BNCHAT), WM_SETFONT, (WPARAM)hfBotFont, TRUE);
					SendMessage(GetDlgItem(hMainDlg, IDC_BNCHAT), WM_SETTEXT, 0, (LPARAM)"");
					break;
				}
			case ID_CONFIGURATION_FONTSETUP_YOURTEXT:
				{
					CHOOSEFONT cf;
					cf.lStructSize = sizeof(CHOOSEFONT); 
					cf.hwndOwner = hMainDlg; 
					cf.hDC = (HDC)NULL; 
					cf.lpLogFont = &lfBotFont; 
					cf.iPointSize = -200; 
					cf.Flags = CF_SCREENFONTS; 
					cf.rgbColors = RGB(0,0,0); 
					cf.lCustData = 0L; 
					cf.lpfnHook = (LPCFHOOKPROC)NULL; 
					cf.lpTemplateName = (LPSTR)NULL; 
					cf.hInstance = (HINSTANCE) NULL; 
					cf.lpszStyle = (LPSTR)NULL; 
					cf.nFontType = SCREEN_FONTTYPE; 
					cf.nSizeMin = 0; 
					cf.nSizeMax = 0;
					ChooseFont(&cf);

					hfBotFont = CreateFontIndirect(cf.lpLogFont);
					CRegisterWIN32* Registry = new CRegisterWIN32;
					Registry->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Fonts\\Your text", FALSE);
						Registry->WriteDword("CharSet", cf.lpLogFont->lfCharSet);
						Registry->WriteDword("ClipPrecision", cf.lpLogFont->lfClipPrecision);
						Registry->WriteDword("Escapement", cf.lpLogFont->lfEscapement);
						Registry->Write("FaceName", cf.lpLogFont->lfFaceName);
						Registry->WriteDword("Height", cf.lpLogFont->lfHeight);
						Registry->WriteDword("Italic", cf.lpLogFont->lfItalic);
						Registry->WriteDword("Orientation", cf.lpLogFont->lfOrientation);
						Registry->WriteDword("OutPrecision", cf.lpLogFont->lfOutPrecision);
						Registry->WriteDword("PitchAndFamily", cf.lpLogFont->lfPitchAndFamily);
						Registry->WriteDword("Quality", cf.lpLogFont->lfQuality);
						Registry->WriteDword("StrikeOut", cf.lpLogFont->lfStrikeOut);
						Registry->WriteDword("Underline", cf.lpLogFont->lfUnderline);
						Registry->WriteDword("Weight", cf.lpLogFont->lfWeight);
						Registry->WriteDword("Width", cf.lpLogFont->lfWidth);
					Registry->Close();
					delete Registry;

					SendMessage(GetDlgItem(hMainDlg, IDC_BNSEND), WM_SETFONT, (WPARAM)hfBotFont, TRUE);
					break;
				}
			case ID_CONFIGURATION_FONTSETUP_CHANNELNAME:
				{
					CHOOSEFONT cf;
					cf.lStructSize = sizeof(CHOOSEFONT); 
					cf.hwndOwner = hMainDlg; 
					cf.hDC = (HDC)NULL; 
					cf.lpLogFont = &lfBotFont; 
					cf.iPointSize = -200; 
					cf.Flags = CF_SCREENFONTS; 
					cf.rgbColors = RGB(0,0,0); 
					cf.lCustData = 0L; 
					cf.lpfnHook = (LPCFHOOKPROC)NULL; 
					cf.lpTemplateName = (LPSTR)NULL; 
					cf.hInstance = (HINSTANCE) NULL; 
					cf.lpszStyle = (LPSTR)NULL; 
					cf.nFontType = SCREEN_FONTTYPE; 
					cf.nSizeMin = 0; 
					cf.nSizeMax = 0;
					ChooseFont(&cf);
					
					hfBotFont = CreateFontIndirect(cf.lpLogFont);
					CRegisterWIN32* Registry = new CRegisterWIN32;
					Registry->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Fonts\\Channel name", FALSE);
						Registry->WriteDword("CharSet", cf.lpLogFont->lfCharSet);
						Registry->WriteDword("ClipPrecision", cf.lpLogFont->lfClipPrecision);
						Registry->WriteDword("Escapement", cf.lpLogFont->lfEscapement);
						Registry->Write("FaceName", cf.lpLogFont->lfFaceName);
						Registry->WriteDword("Height", cf.lpLogFont->lfHeight);
						Registry->WriteDword("Italic", cf.lpLogFont->lfItalic);
						Registry->WriteDword("Orientation", cf.lpLogFont->lfOrientation);
						Registry->WriteDword("OutPrecision", cf.lpLogFont->lfOutPrecision);
						Registry->WriteDword("PitchAndFamily", cf.lpLogFont->lfPitchAndFamily);
						Registry->WriteDword("Quality", cf.lpLogFont->lfQuality);
						Registry->WriteDword("StrikeOut", cf.lpLogFont->lfStrikeOut);
						Registry->WriteDword("Underline", cf.lpLogFont->lfUnderline);
						Registry->WriteDword("Weight", cf.lpLogFont->lfWeight);
						Registry->WriteDword("Width", cf.lpLogFont->lfWidth);
					Registry->Close();
					delete Registry;

					SendMessage(GetDlgItem(hMainDlg, IDC_CHANNAME), WM_SETFONT, (WPARAM)hfBotFont, TRUE);
					break;
				}
			case ID_CONFIGURATION_FONTSETUP_CHANNELLIST:
				{
					CHOOSEFONT cf;
					cf.lStructSize = sizeof(CHOOSEFONT); 
					cf.hwndOwner = hMainDlg; 
					cf.hDC = (HDC)NULL; 
					cf.lpLogFont = &lfBotFont; 
					cf.iPointSize = -200; 
					cf.Flags = CF_SCREENFONTS; 
					cf.rgbColors = RGB(0,0,0); 
					cf.lCustData = 0L; 
					cf.lpfnHook = (LPCFHOOKPROC)NULL; 
					cf.lpTemplateName = (LPSTR)NULL; 
					cf.hInstance = (HINSTANCE) NULL; 
					cf.lpszStyle = (LPSTR)NULL; 
					cf.nFontType = SCREEN_FONTTYPE; 
					cf.nSizeMin = 0; 
					cf.nSizeMax = 0;
					ChooseFont(&cf);
					
					hfBotFont = CreateFontIndirect(cf.lpLogFont);
					CRegisterWIN32* Registry = new CRegisterWIN32;
					Registry->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Fonts\\Channel list", FALSE);
						Registry->WriteDword("CharSet", cf.lpLogFont->lfCharSet);
						Registry->WriteDword("ClipPrecision", cf.lpLogFont->lfClipPrecision);
						Registry->WriteDword("Escapement", cf.lpLogFont->lfEscapement);
						Registry->Write("FaceName", cf.lpLogFont->lfFaceName);
						Registry->WriteDword("Height", cf.lpLogFont->lfHeight);
						Registry->WriteDword("Italic", cf.lpLogFont->lfItalic);
						Registry->WriteDword("Orientation", cf.lpLogFont->lfOrientation);
						Registry->WriteDword("OutPrecision", cf.lpLogFont->lfOutPrecision);
						Registry->WriteDword("PitchAndFamily", cf.lpLogFont->lfPitchAndFamily);
						Registry->WriteDword("Quality", cf.lpLogFont->lfQuality);
						Registry->WriteDword("StrikeOut", cf.lpLogFont->lfStrikeOut);
						Registry->WriteDword("Underline", cf.lpLogFont->lfUnderline);
						Registry->WriteDword("Weight", cf.lpLogFont->lfWeight);
						Registry->WriteDword("Width", cf.lpLogFont->lfWidth);
					Registry->Close();
					delete Registry;

					SendMessage(GetDlgItem(hMainDlg, IDC_CHANLIST), WM_SETFONT, (WPARAM)hfBotFont, TRUE);
					break;
				}
			case ID_CONFIGURATION_CHARACTERSETUP:
				CreateDialog(hDlgInst, MAKEINTRESOURCE(IDD_CONFIGURE), hMainDlg, ConfigureDialogProc);	
				break;
			case ID_CONFIGURATION_MISCELLANEOUS:
				CreateDialog(hDlgInst, MAKEINTRESOURCE(IDD_MISC), hMainDlg, MiscDialogProc);
				break;
			case ID_CONFIGURATION_CDKEYS:
				CreateDialog(hDlgInst, MAKEINTRESOURCE(IDD_CDKEYS), hMainDlg, CdkeyDialogProc);
				break;
			case ID_CONFIGURATION_FILEPATHS:
				CreateDialog(hDlgInst, MAKEINTRESOURCE(IDD_FILE_PATHS), hMainDlg, FilePathDialogProc);
				break;
			case ID_CONNECTION_DISCONNECT:
				{
					szLocalAccountInfo.Connected = false;
					CloseTCPSocket(sckBNCS);
					AppendText(hBNChat, RED, "Connection closed\n");
					break;
				}
			case ID_CONNECTION_CONNECT:
				{
					CloseTCPSocket(sckBNCS);
					sckBNCS = CreateTCPSocket();
					szLocalAccountInfo.Connected = false;
					AppendText(hBNChat, WHITE, "Attempting to connect to server %s:6112\n", szLocalAccountInfo.szServer);
					if(ConnectTCPSocket(sckBNCS, szLocalAccountInfo.szServer, 6112)){
						szLocalAccountInfo.Connected = true;
						szLocalAccountInfo.StartedUp = true;
						AppendText(hBNChat, WHITE, "Connected\n");
						WSAAsyncSelect(sckBNCS, hMainDlg, DATA_PENDING_TCP, FD_READ | FD_CLOSE);
						send(sckBNCS, "\x1", 1, 0);
						SendAuthInfo(sckBNCS);
						Queue.Clear();
					} else {
						szLocalAccountInfo.Connected = false;
						AppendText(hBNChat, RED, "Connection closed (Error %d)\n", WSAGetLastError());
						break;
					}
				}
			}
			return TRUE;
		}
	case WM_CLOSE:
		{
			if(MessageBox(hDlg, "Do you really want to quit?", "Confirm", MB_ICONQUESTION | MB_YESNO) == IDNO) { break; }
			EndDialog(hDlg, 0);
			_exit(0);
			return TRUE;
		}
	case WM_DESTROY:
		{
			TrayHandler_Remove();
			SetWindowLong(GetDlgItem(hDlg, IDC_BNCHAT), GWL_WNDPROC, (LONG)wpOrigRichTextProc);
			SetWindowLong(GetDlgItem(hDlg, IDC_CHANLIST), GWL_WNDPROC, (LONG)wpOrigListViewProc);
			PostQuitMessage(0);
			break;
		}
	}
	return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	hDlgInst = hInstance;
	char szRegPath[128];
	InitCommonControls();
	HINSTANCE RichEditDLL = LoadLibrary("RichEd32.dll");
	CRegisterWIN32* LoadConfig = new CRegisterWIN32;
		strcpy(szRegPath, "Software\\Beta Productions\\SoupBot2");
		LoadConfig->Open(HKEY_CURRENT_USER, szRegPath, TRUE);
			sprintf(szRegPath, "Software\\Beta Productions\\SoupBot2\\Profiles\\Profile%i", LoadConfig->ReadInt("Last Profile"));
		LoadConfig->Close();
		LoadConfig->Open(HKEY_CURRENT_USER, szRegPath, TRUE);
			strcpy(szLocalAccountInfo.szUsername, LoadConfig->Read("Username"));
			strcpy(szLocalAccountInfo.szPassword, LoadConfig->Read("Password"));
			strcpy(szLocalAccountInfo.szHomeChannel, LoadConfig->Read("Home Channel"));
			strcpy(szLocalAccountInfo.szServer, LoadConfig->Read("Server"));
			strcpy(szLocalAccountInfo.szProfileName, LoadConfig->Read("Profile Name"));
			strcpy(szLocalAccountInfo.szCDkey, LoadConfig->Read("CD Key"));
			strcpy(szLocalAccountInfo.szExpCDkey, LoadConfig->Read("ExpCD Key"));
			strcpy(szLocalAccountInfo.szEmailAddy, LoadConfig->Read("Email"));
			szLocalAccountInfo.Spawn = LoadConfig->ReadInt("CD Spawn");
			szLocalAccountInfo.iGameIndex = LoadConfig->ReadInt("Game Type");
			GameIndexToName(szLocalAccountInfo.szGameAbbr, szLocalAccountInfo.iGameIndex, true);
			szLocalAccountInfo.lVerByte = GetVersionByte();
		LoadConfig->Close();
		LoadConfig->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Miscellaneous\\Idle", TRUE);
			strcpy(szLocalAccountInfo.IdleMessage, LoadConfig->Read("Message"));
			szLocalAccountInfo.IdleTimer = LoadConfig->ReadDword("Idle Delay");
			szLocalAccountInfo.IdleOn = LoadConfig->ReadDword("Use Idle");
			szLocalAccountInfo.IdleSpec = LoadConfig->ReadDword("Use Message");
			szLocalAccountInfo.IdleQuotes = LoadConfig->ReadDword("Use Quotes");
		LoadConfig->Close();
		LoadConfig->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Miscellaneous", TRUE);
			strcpy(szLocalAccountInfo.StartupMsg, LoadConfig->Read("Startup Message"));
		LoadConfig->Close();
	delete LoadConfig;
	IdleTicker = (szLocalAccountInfo.IdleTimer / 30);
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDLG), NULL, MainDialogProc);
	FreeLibrary(RichEditDLL);
	return 0;
}

void SaveProfile(char Username[32], char Password[256], char Home[128], char Server[256], char ProfileName[128], int GameIndex, char CDkey[32], char ExpCDkey[32], char EmailAddy[128], bool Spawn)
{
	char szTempChar[256] = "blank";
	bool bProfileExists = TRUE;
	int iNumProfiles = 1;

	if(ProfileExists((strstr(ProfileName, ") ") ? (ProfileName+4) : ProfileName)) != 0)
	{
		char szProfToUpdate[128];
		CRegisterWIN32* Update = new CRegisterWIN32;
		sprintf(szProfToUpdate, "Software\\Beta Productions\\SoupBot2\\Profiles\\Profile%i", ProfileExists((strstr(ProfileName, ") ") ? (ProfileName+4) : ProfileName)));
		Update->Open(HKEY_CURRENT_USER, szProfToUpdate, FALSE);
			Update->Write("Username", Username);
			Update->Write("Password", Password);
			Update->Write("Server", Server);
			Update->Write("Home Channel", Home);
			Update->WriteDword("Game Type", GameIndex);
			Update->Write("CD Key", CDkey);
			Update->Write("ExpCD Key", ExpCDkey);
			Update->WriteDword("CD Spawn", Spawn);
			Update->Write("Email", EmailAddy);
		Update->Close();
		delete Update;
		return;
	}

	CRegisterWIN32* Registry = new CRegisterWIN32;
	if(!Registry->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Profiles", TRUE))
	{
		Registry->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Profiles", FALSE);
	}
	Registry->Close();
	for(iNumProfiles = 1; bProfileExists == TRUE; iNumProfiles++)
	{
		if(iNumProfiles > 10) { MessageBox(FindWindow(NULL, "Character setup"), "Reached maximum amount of profiles.\nPlease delete unused profiles.", "Message", MB_OK); return; }
		sprintf(szTempChar, "Software\\Beta Productions\\SoupBot2\\Profiles\\Profile%i", iNumProfiles);
		if(Registry->Open(HKEY_CURRENT_USER, szTempChar, TRUE) == TRUE)
		{ bProfileExists = TRUE; }
		else
		{ bProfileExists = FALSE; break; }
	}
	Registry->Close();
	Registry->Open(HKEY_CURRENT_USER, szTempChar, FALSE);
		if(strstr(ProfileName, ") "))
		{
			strcpy(szTempChar, (ProfileName+4));
			Registry->Write("Profile Name", szTempChar);
		} else { Registry->Write("Profile Name", ProfileName); }
		Registry->Write("Username", Username);
		Registry->Write("Password", Password);
		Registry->Write("Server", Server);
		Registry->Write("Home Channel", Home);
		Registry->WriteDword("Game Type", GameIndex);
		Registry->Write("CD Key", CDkey);
		Registry->Write("ExpCD Key", ExpCDkey);
		Registry->WriteDword("CD Spawn",  Spawn);
		Registry->Write("Email", EmailAddy);
	Registry->Close();
	delete Registry;
	return;
}

void LoadProfiles()
{
	HWND ConfigDialogHandle = FindWindow(NULL, "Character setup");
	CRegisterWIN32* Registry = new CRegisterWIN32;
	int iNumProfiles = 1;
	char szTempChar[256] = "blank";
	char szTempChar2[256] = "blank";
	bool bProfileExists = TRUE;
	
	for(iNumProfiles = 1; iNumProfiles <= 10; iNumProfiles++)
	{
		sprintf(szTempChar, "Software\\Beta Productions\\SoupBot2\\Profiles\\Profile%i", iNumProfiles);
		if(Registry->Open(HKEY_CURRENT_USER, szTempChar, TRUE) == TRUE)
		{
			strcpy(szTempChar, Registry->Read("Profile Name", "Untitled"));
			strcpy(szTempChar2, szTempChar);
			sprintf(szTempChar, "(%i) %s", iNumProfiles, szTempChar2);
			SendDlgItemMessage( ConfigDialogHandle, IDC_CONFIG_PROFILE_NAME, CB_ADDSTRING, 0, (LPARAM)szTempChar );
		}
	}
	delete Registry;
	return;
}

void SetLastProfile(int LastProfile)
{
	CRegisterWIN32* Registry = new CRegisterWIN32;
		Registry->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2", FALSE);
			Registry->WriteDword("Last Profile", LastProfile);
		Registry->Close();
	delete Registry;
	return;
}

int ProfileExists(char ProfileName[128])
{
	HWND ConfigDialogHandle = FindWindow(NULL, "Character setup");
	CRegisterWIN32* Registry = new CRegisterWIN32;
	int iNumProfiles = 1;
	char szTempChar[256];

	for(iNumProfiles = 1; iNumProfiles <= 10; iNumProfiles++)
	{
		sprintf(szTempChar, "Software\\Beta Productions\\SoupBot2\\Profiles\\Profile%i", iNumProfiles);
		if(Registry->Open(HKEY_CURRENT_USER, szTempChar, TRUE) == TRUE)
		{
			if(!stricmp(Registry->Read("Profile Name", "None"), ProfileName))
			{  Registry->Close();
			   delete Registry;
			   return iNumProfiles; }
			Registry->Close();
		}
	}
	delete Registry;
 return 0;
}

void DeleteProfile(char Profile[32])
{
	char szTempChar[128];
	int iProfNumb;
	strcpy(szTempChar, (strstr(Profile, ") ") ? (Profile+4) : Profile));

	iProfNumb = ProfileExists(szTempChar);
	
	if(iProfNumb == 0)
		return;

	sprintf(szTempChar, "Software\\Beta Productions\\SoupBot2\\Profiles\\Profile%i", iProfNumb);
	CRegisterWIN32* DeleteProf = new CRegisterWIN32;
		DeleteProf->DeleteKey(HKEY_CURRENT_USER, szTempChar);
	delete DeleteProf;
	return;
}

void RenameProfile(char Profile[32], char OldProfile[32])
{
	char szTempChar[128];
	int iProfNumb;
	strcpy(szTempChar, (strstr(Profile, ") ") ? (Profile+4) : Profile));

	if(ProfileExists(szTempChar) != 0)
	{
		MessageBox(FindWindow(NULL, "Character setup"), "That profile already exists.\nPlease choose a different name.", "Notice", MB_OK);
		return;
	}

	iProfNumb = ProfileExists((strstr(OldProfile, ") ") ? (OldProfile+4) : OldProfile));
	sprintf(szTempChar, "Software\\Beta Productions\\SoupBot2\\Profiles\\Profile%i", iProfNumb);
	CRegisterWIN32* RenameProf = new CRegisterWIN32;
		RenameProf->Open(HKEY_CURRENT_USER, szTempChar, FALSE);
			RenameProf->DeleteValue("Profile Name");
			RenameProf->Write("Profile Name", Profile);
		RenameProf->Close();
	delete RenameProf;
	return;
}

int GameNameToIndex(char GameName[32])
{
	if(strcmp(GameName, "Diablo II") == 0)
	{
		return 0;
	}
	if(strcmp(GameName, "Diablo II LoD") == 0)
	{
		return 1;
	}
	if(strcmp(GameName, "Starcraft Broodwar") == 0)
	{
		return 2;
	}
	if(strcmp(GameName, "Starcraft Retail") == 0)
	{
		return 3;
	}
	if(strcmp(GameName, "Warcraft II BNE") == 0)
	{
		return 4;
	}
	if(strcmp(GameName, "Warcraft III") == 0)
	{
		return 5;
	}
	if(strcmp(GameName, "Warcraft III TFT") == 0)
	{
		return 6;
	}
return 1;
}

void GameIndexToName(char* gamename, int Index, bool FourLetterAbbr)
{
	if(FourLetterAbbr) {
		strcpy(gamename, "RATS");
		switch (Index)
		{
		case 0:
			strcpy(gamename, "VD2D");
			break;
		case 1:
			strcpy(gamename, "PX2D");
			break;
		case 2:
			strcpy(gamename, "PXES");
			break;
		case 3:
			strcpy(gamename, "RATS");
			break;
		case 4:
			strcpy(gamename, "NB2W");
			break;
		case 5:
			strcpy(gamename, "3RAW");
			break;
		case 6:
			strcpy(gamename, "PX3W");
			break;
		default:
			strcpy(gamename, "RATS");
			break;
		}
	} else {
		strcpy(gamename, "Starcraft Retail");
		switch (Index)
		{
		case 0:
			strcpy(gamename, "Diablo II");
			break;
		case 1:
			strcpy(gamename, "Diablo II LoD");
			break;
		case 2:
			strcpy(gamename, "Starcraft Broodwar");
			break;
		case 3:
			strcpy(gamename, "Starcraft Retail");
			break;
		case 4:
			strcpy(gamename, "Warcraft II BNE");
			break;
		case 5:
			strcpy(gamename, "Warcraft III");
			break;
		case 6:
			strcpy(gamename, "Warcraft III TFT");
			break;
		default:
			strcpy(gamename, "Starcraft Retail");
			break;
		}
	}
}

int FindCheckedRadioButton()
{
	if(IsDlgButtonChecked(FindWindow(NULL, "CD Key setup"), IDC_CDKEYS_SC) == BST_CHECKED)
	{	
		return 1;
	}
	if(IsDlgButtonChecked(FindWindow(NULL, "CD Key setup"), IDC_CDKEYS_W2) == BST_CHECKED)
	{
		return 2;
	}
	if(IsDlgButtonChecked(FindWindow(NULL, "CD Key setup"), IDC_CDKEYS_D2) == BST_CHECKED)
	{
		return 3;
	}
	if(IsDlgButtonChecked(FindWindow(NULL, "CD Key setup"), IDC_CDKEYS_LOD) == BST_CHECKED)
	{	
		return 4;
	}
	if(IsDlgButtonChecked(FindWindow(NULL, "CD Key setup"), IDC_CDKEYS_WAR3) == BST_CHECKED)
	{	
		return 5;
	}
	if(IsDlgButtonChecked(FindWindow(NULL, "CD Key setup"), IDC_CDKEYS_TFT) == BST_CHECKED)
	{	
		return 6;
	}
	return 0;
}

int CDkeyExists(char CDkey[32], char Product[6])
{
	CRegisterWIN32* Registry = new CRegisterWIN32;
	int iNumCDkeys = 1;
	char szTempChar[64];

	sprintf(szTempChar, "Software\\Beta Productions\\SoupBot2\\CDKeys\\%s", Product);
	if(Registry->Open(HKEY_CURRENT_USER, szTempChar, TRUE) == TRUE)
	{
		for(iNumCDkeys = 1; iNumCDkeys <= 10; iNumCDkeys++)
		{
			sprintf(szTempChar, "CDKey#%i", iNumCDkeys);
			if(!stricmp(Registry->Read(szTempChar, "None"), CDkey))
			{ Registry->Close();
			  delete Registry;
			  return iNumCDkeys; }
		}
	}
	Registry->Close();
	delete Registry;
 return 0;
}

void DeleteCDkey(int CDkeyNumber, char Product[6])
{
	char szTempChar[64];
	
	sprintf(szTempChar, "Software\\Beta Productions\\SoupBot2\\CDKeys\\%s", Product);
	CRegisterWIN32* Delete = new CRegisterWIN32;
		Delete->Open(HKEY_CURRENT_USER, szTempChar, FALSE);
			sprintf(szTempChar, "CDKey#%i", CDkeyNumber);
			Delete->DeleteValue(szTempChar);
		Delete->Close();
	delete Delete;
	return;
}

void WriteCDkeyList(HWND hWnd, int DlgItemID)
{
	CRegisterWIN32* Write = new CRegisterWIN32;
	char szCDkeyNum[32];
	char szTempChar[32];
	int x, y;
	SendDlgItemMessage(hWnd, DlgItemID, CB_RESETCONTENT, 0, 0);

	switch(DlgItemID) {
	case IDC_CDKEYS_CDKEY:
		{
			for(x = 1; x <= 6; x++)
			{
				switch(x)
				{
				case CDKEY_STAR:
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"SC/BW Keys:");
					Write->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\STAR", TRUE);
					for(y = 1; y <= 10; y++)
					{	
						sprintf(szCDkeyNum, "CDKey#%i", y);
						sprintf(szTempChar, "%s", Write->Read(szCDkeyNum));
						if(strlen(szTempChar) != 0)
						{ SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)szTempChar); }
					}
					Write->Close();
					break;
				case CDKEY_W2BN:
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"  ");
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"Warcraft II keys:");
					Write->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\W2BN", TRUE);
					for(y = 1; y <= 10; y++)
					{	
						sprintf(szCDkeyNum, "CDKey#%i", y);
						sprintf(szTempChar, "%s", Write->Read(szCDkeyNum));
						if(strlen(szTempChar) != 0)
						{ SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)szTempChar); }
					}
					Write->Close();
					break;
				case CDKEY_D2DV:
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"  ");
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"Diablo II keys:");
					Write->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\D2DV", TRUE);
					for(y = 1; y <= 10; y++)
					{	
						sprintf(szCDkeyNum, "CDKey#%i", y);
						sprintf(szTempChar, "%s", Write->Read(szCDkeyNum));
						if(strlen(szTempChar) != 0)
						{ SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)szTempChar); }
					}
					Write->Close();
					break;
				case CDKEY_D2XP:
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"  ");
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"Lord of Destruction keys:");
					Write->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\D2XP", TRUE);
					for(y = 1; y <= 10; y++)
					{	
						sprintf(szCDkeyNum, "CDKey#%i", y);
						sprintf(szTempChar, "%s", Write->Read(szCDkeyNum));
						if(strlen(szTempChar) != 0)
						{ SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)szTempChar); }
					}
					Write->Close();
					break;
				case CDKEY_WAR3:
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"  ");
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"Warcraft III keys:");
					Write->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\WAR3", TRUE);
					for(y = 1; y <= 10; y++)
					{	
						sprintf(szCDkeyNum, "CDKey#%i", y);
						sprintf(szTempChar, "%s", Write->Read(szCDkeyNum));
						if(strlen(szTempChar) != 0)
						{ SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)szTempChar); }
					}
					Write->Close();
					break;
				case CDKEY_W3XP:
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"  ");
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"The Frozen Throne keys:");
					Write->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\W3XP", TRUE);
					for(y = 1; y <= 10; y++)
					{	
						sprintf(szCDkeyNum, "CDKey#%i", y);
						sprintf(szTempChar, "%s", Write->Read(szCDkeyNum));
						if(strlen(szTempChar) != 0)
						{ SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)szTempChar); }
					}
					Write->Close();
					break;
				}
			}
			break;
		}
	case IDC_CONFIG_CDKEY:
		{
			for(x = 1; x <= 6; x++)
			{
				switch(x)
				{
				case CDKEY_STAR:
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"SC/BW Keys:");
					Write->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\STAR", TRUE);
					for(y = 1; y <= 10; y++)
					{	
						sprintf(szCDkeyNum, "CDKey#%i", y);
						sprintf(szTempChar, "%s", Write->Read(szCDkeyNum));
						if(strlen(szTempChar) != 0)
						{ SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)szTempChar); }
					}
					Write->Close();
					break;
				case CDKEY_W2BN:
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"  ");
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"Warcraft II keys:");
					Write->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\W2BN", TRUE);
					for(y = 1; y <= 10; y++)
					{	
						sprintf(szCDkeyNum, "CDKey#%i", y);
						sprintf(szTempChar, "%s", Write->Read(szCDkeyNum));
						if(strlen(szTempChar) != 0)
						{ SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)szTempChar); }
					}
					Write->Close();
					break;
				case CDKEY_D2DV:
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"  ");
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"Diablo II keys:");
					Write->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\D2DV", TRUE);
					for(y = 1; y <= 10; y++)
					{	
						sprintf(szCDkeyNum, "CDKey#%i", y);
						sprintf(szTempChar, "%s", Write->Read(szCDkeyNum));
						if(strlen(szTempChar) != 0)
						{ SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)szTempChar); }
					}
					Write->Close();
					break;
				case CDKEY_WAR3:
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"  ");
					SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"Warcraft III keys:");
					Write->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\WAR3", TRUE);
					for(y = 1; y <= 10; y++)
					{	
						sprintf(szCDkeyNum, "CDKey#%i", y);
						sprintf(szTempChar, "%s", Write->Read(szCDkeyNum));
						if(strlen(szTempChar) != 0)
						{ SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)szTempChar); }
					}
					Write->Close();
					break;
				}
			}
			break;
		}
	}
	delete Write;
	return;
}

void WriteExpCDkeyList(HWND hWnd, int DlgItemID)
{
	CRegisterWIN32* Write = new CRegisterWIN32;
	char szCDkeyNum[32];
	char szTempChar[32];
	int x, y;
	SendDlgItemMessage(hWnd, DlgItemID, CB_RESETCONTENT, 0, 0);
	for(x = 1; x <= 2; x++)
	{
		switch(x)
		{
		case 1:
			SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"Lord of Destruction keys:");
			Write->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\D2XP", TRUE);
			for(y = 1; y <= 10; y++)
			{	
				sprintf(szCDkeyNum, "CDKey#%i", y);
				sprintf(szTempChar, "%s", Write->Read(szCDkeyNum));
				if(strlen(szTempChar) != 0)
				{ SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)szTempChar); }
			}
			Write->Close();
			break;
		case 2:
			SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"  ");
			SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)"The Frozen Throne keys:");
			Write->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\CDKeys\\W3XP", TRUE);
			for(y = 1; y <= 10; y++)
			{	
				sprintf(szCDkeyNum, "CDKey#%i", y);
				sprintf(szTempChar, "%s", Write->Read(szCDkeyNum));
				if(strlen(szTempChar) != 0)
				{ SendDlgItemMessage(hWnd, DlgItemID, CB_ADDSTRING, 0, (LPARAM)szTempChar); }
			}
			Write->Close();
			break;
		}
	}
	delete Write;
	return;
}

void GameNameToAbbrv(char gamename[32], char* abbrv)
{
	strcpy(abbrv, "STAR");
	if(!strcmp(gamename, "Starcraft/Brood War"))
		strcpy(abbrv, "STAR");
	if(!strcmp(gamename, "Warcraft II"))
		strcpy(abbrv, "W2BN");
	if(!strcmp(gamename, "Diablo II"))
		strcpy(abbrv, "D2DV");
	if(!strcmp(gamename, "Lord of Destruction"))
		strcpy(abbrv, "D2XP");
	if(!strcmp(gamename, "Warcraft III"))
		strcpy(abbrv, "WAR3");
	if(!strcmp(gamename, "The Frozen Throne"))
		strcpy(abbrv, "W3XP");
}

void GetFilePath(char *filepath, char game[12], char filetype[12])
{
	OPENFILENAME ofnFilePath;
	char szChar[256];
	
	ZeroMemory(&ofnFilePath,sizeof(ofnFilePath));
	ZeroMemory(szChar, 256);

	ofnFilePath.lStructSize = sizeof(ofnFilePath);
	ofnFilePath.hwndOwner = FindWindow(NULL, "File path setup");

	if(((!strcmp(game, "Diablo II")) && (!strcmp(filetype, "EXE")))) { ofnFilePath.lpstrFilter = "Diablo II EXE\0*.exe\0\0"; }
	if(((!strcmp(game, "Diablo II")) && (!strcmp(filetype, "DLL")))) { ofnFilePath.lpstrFilter = "Diablo II BNClient\0Bnclient.dll\0\0"; }
	if(((!strcmp(game, "Diablo II")) && (!strcmp(filetype, "SNP")))) { ofnFilePath.lpstrFilter = "Diablo II D2Client\0D2Client.dll\0\0"; }

	if(((!strcmp(game, "Starcraft/Brood War")) && (!strcmp(filetype, "EXE")))) { ofnFilePath.lpstrFilter = "Starcraft/Brood War EXE\0*.exe\0\0"; }
	if(((!strcmp(game, "Starcraft/Brood War")) && (!strcmp(filetype, "DLL")))) { ofnFilePath.lpstrFilter = "Starcraft/Brood War STORM\0storm.dll\0\0"; }
	if(((!strcmp(game, "Starcraft/Brood War")) && (!strcmp(filetype, "SNP")))) { ofnFilePath.lpstrFilter = "Starcraft/Brood War BATTLE\0battle.snp\0\0"; }

	if(((!strcmp(game, "Warcraft II")) && (!strcmp(filetype, "EXE")))) { ofnFilePath.lpstrFilter = "Warcraft II EXE\0*.exe\0\0"; }
	if(((!strcmp(game, "Warcraft II")) && (!strcmp(filetype, "DLL")))) { ofnFilePath.lpstrFilter = "Warcraft II STORM\0storm.dll\0\0"; }
	if(((!strcmp(game, "Warcraft II")) && (!strcmp(filetype, "SNP")))) { ofnFilePath.lpstrFilter = "Warcraft II BATTLE\0battle.snp\0\0"; }

	if(((!strcmp(game, "Lord of Destruction")) && (!strcmp(filetype, "EXE")))) { ofnFilePath.lpstrFilter = "Lord of Destruction EXE\0*.exe\0\0"; }
	if(((!strcmp(game, "Lord of Destruction")) && (!strcmp(filetype, "DLL")))) { ofnFilePath.lpstrFilter = "Lord of Destruction BNClient\0Bnclient.dll\0\0"; }
	if(((!strcmp(game, "Lord of Destruction")) && (!strcmp(filetype, "SNP")))) { ofnFilePath.lpstrFilter = "Lord of Destruction D2Client\0D2Client.dll\0\0"; }

	if(((!strcmp(game, "Warcraft III")) && (!strcmp(filetype, "EXE")))) { ofnFilePath.lpstrFilter = "War III executable\0*.exe\0\0"; }
	if(((!strcmp(game, "Warcraft III")) && (!strcmp(filetype, "DLL")))) { ofnFilePath.lpstrFilter = "War III Bnclient.dll\0*.dll\0\0"; }
	if(((!strcmp(game, "Warcraft III")) && (!strcmp(filetype, "SNP")))) { ofnFilePath.lpstrFilter = "War III Game.dll\0game.dll\0\0"; }

	if(((!strcmp(game, "The Frozen Throne")) && (!strcmp(filetype, "EXE")))) { ofnFilePath.lpstrFilter = "TFT executable\0*.exe\0\0"; }
	if(((!strcmp(game, "The Frozen Throne")) && (!strcmp(filetype, "DLL")))) { ofnFilePath.lpstrFilter = "TFT Bnclient.dll\0*.dll\0\0"; }
	if(((!strcmp(game, "The Frozen Throne")) && (!strcmp(filetype, "SNP")))) { ofnFilePath.lpstrFilter = "TFT Game.dll\0game.dll\0\0"; }


	ofnFilePath.lpstrFile = szChar;
	ofnFilePath.nMaxFile = 256;
	ofnFilePath.lpstrTitle = "Open\0";
	ofnFilePath.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES;
	
	if(GetOpenFileName(&ofnFilePath) == FALSE) { strcpy(filepath, "Game file"); return;}
	strcpy(filepath, szChar);
	return;
}

int MessageBoxFmt(HWND hWnd, int iType, const char *lpszCaption, const char *lpszText, ...)
{
	char szTxt[2048] = "";
	va_list vaArg;
	va_start(vaArg, lpszText);
	vsprintf(szTxt, lpszText, vaArg);
	va_end(vaArg);
	return MessageBox(hWnd, szTxt, lpszCaption, iType);
}

char *NewString(int iSize)
{
	char *pPtr = new char[iSize];
	return pPtr;
}

void AddQuote(char *quote) {
	FILE *File;

	File = fopen("Quotes.txt", "a");

	fputs("\n", File);
	fputs(quote, File);
	
	fclose(File);
}

void CharFormat(COLORREF Color, CHARFORMAT &cfFormat)
{
	cfFormat.cbSize = sizeof(CHARFORMAT);
	cfFormat.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE | CFM_CHARSET;
	cfFormat.dwEffects = CFE_PROTECTED;
	cfFormat.yHeight = lfChatFont.lfHeight;
	cfFormat.yOffset = 0;
	cfFormat.crTextColor = Color;
	cfFormat.bCharSet = lfChatFont.lfCharSet;
	cfFormat.bPitchAndFamily = lfChatFont.lfPitchAndFamily;
	strcpy(cfFormat.szFaceName, lfChatFont.lfFaceName);
}

void __cdecl AppendText(HWND hRichEdit, COLORREF Color, char *szFmt, ...)
{
	char buffer[328];
	va_list args;

	va_start(args, szFmt);
	_vsnprintf(buffer, sizeof(buffer) - 1, szFmt, args);
	buffer[sizeof(buffer)-1] = '\0';
	va_end(args);

	CHARFORMAT cfFormat;
	CHARRANGE CharRange = {-1, -1};
	SendMessage(hRichEdit, EM_EXSETSEL, 0, (LPARAM)&CharRange);
	CharFormat(Color, cfFormat);
	SendMessage(hRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfFormat);
	SendMessage(hRichEdit, EM_REPLACESEL, FALSE, (LPARAM)buffer);
	SendMessage(hRichEdit, EM_SCROLL, SB_LINEDOWN, 0);
}

void __cdecl AppendTextTS(HWND hRichEdit, COLORREF Color, char *szFmt, ...)
{
	char buffer[328];
	va_list args;
	SYSTEMTIME st;

	va_start(args, szFmt);
	_vsnprintf(buffer, sizeof(buffer) - 1, szFmt, args);
	buffer[sizeof(buffer)-1] = '\0';
	va_end(args);

	GetLocalTime(&st);
	AppendText(hBNChat, Color, "[%02i:%02i:%02i] ", st.wHour, st.wMinute, st.wSecond, buffer);

	CHARFORMAT cfFormat;
	CHARRANGE CharRange = {-1, -1};
	SendMessage(hRichEdit, EM_EXSETSEL, 0, (LPARAM)&CharRange);
	CharFormat(Color, cfFormat);
	SendMessage(hRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfFormat);
	SendMessage(hRichEdit, EM_REPLACESEL, FALSE, (LPARAM)buffer);
	SendMessage(hRichEdit, EM_SCROLL, SB_LINEDOWN, 0);
}

void TrayHandler_Remove(void){
	extern NOTIFYICONDATA note;

	Shell_NotifyIcon(NIM_DELETE, &note);
}

void TrayHandler_AddAndMinimize(void){
	extern NOTIFYICONDATA note;
	char ToolTipText[255];

	note.cbSize = sizeof(NOTIFYICONDATA);
	note.hIcon = LoadIcon(hDlgInst, MAKEINTRESOURCE(IDI_SMALL));
	note.hWnd = hMainDlg;
	note.uCallbackMessage = WM_SHELLNOTIFY;

	sprintf(ToolTipText, "%s - %s", szLocalAccountInfo.szRealUsername, szLocalAccountInfo.szServer);
	lstrcpy(note.szTip, ToolTipText);

	note.uID = IDI_TRAY;
	note.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	ShowWindow(hMainDlg, SW_HIDE);
	Shell_NotifyIcon(NIM_ADD, &note);
}

void TrayHandler_Restore(void) {
	extern NOTIFYICONDATA note;

	ShowWindow(hMainDlg, SW_SHOW);
	ShowWindow(hMainDlg, SW_RESTORE);
	Shell_NotifyIcon(NIM_DELETE, &note);
}

long GetVersionByte(void) {
	long version_byte;
	CRegisterWIN32* Regist = new CRegisterWIN32;

	switch(szLocalAccountInfo.iGameIndex) {
	case GAME_VD2D:
		version_byte = VER_VD2D;
		szLocalAccountInfo.Expansion = false;
		Regist->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\D2DV", TRUE);
			strcpy(szLocalAccountInfo.EXE, Regist->Read("Game EXE", "Game EXE"));
			strcpy(szLocalAccountInfo.DLL1, Regist->Read("Game DLL", "Game DLL"));
			strcpy(szLocalAccountInfo.DLL2, Regist->Read("Game SNP", "Game SNP"));
		Regist->Close();
		break;
	case GAME_PX2D:
		version_byte = VER_PX2D;
		szLocalAccountInfo.Expansion = true;
		Regist->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\D2XP", TRUE);
			strcpy(szLocalAccountInfo.EXE, Regist->Read("Game EXE", "Game EXE"));
			strcpy(szLocalAccountInfo.DLL1, Regist->Read("Game DLL", "Game DLL"));
			strcpy(szLocalAccountInfo.DLL2, Regist->Read("Game SNP", "Game SNP"));
		Regist->Close();
		break;
	case GAME_NB2W:
		version_byte = VER_NB2W;
		szLocalAccountInfo.Expansion = false;
		Regist->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\W2BN", TRUE);
			strcpy(szLocalAccountInfo.EXE, Regist->Read("Game EXE", "Game EXE"));
			strcpy(szLocalAccountInfo.DLL1, Regist->Read("Game DLL", "Game DLL"));
			strcpy(szLocalAccountInfo.DLL2, Regist->Read("Game SNP", "Game SNP"));
		Regist->Close();
		break;
	case GAME_PXES:
		version_byte = VER_PXES;
		szLocalAccountInfo.Expansion = false;
		Regist->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\STAR", TRUE);
			strcpy(szLocalAccountInfo.EXE, Regist->Read("Game EXE", "Game EXE"));
			strcpy(szLocalAccountInfo.DLL1, Regist->Read("Game DLL", "Game DLL"));
			strcpy(szLocalAccountInfo.DLL2, Regist->Read("Game SNP", "Game SNP"));
		Regist->Close();
		break;
	case GAME_RATS:
		version_byte = VER_RATS;
		szLocalAccountInfo.Expansion = false;
		Regist->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\STAR", TRUE);
			strcpy(szLocalAccountInfo.EXE, Regist->Read("Game EXE", "Game EXE"));
			strcpy(szLocalAccountInfo.DLL1, Regist->Read("Game DLL", "Game DLL"));
			strcpy(szLocalAccountInfo.DLL2, Regist->Read("Game SNP", "Game SNP"));
		Regist->Close();
		break;
	case GAME_3RAW:
		version_byte = VER_3RAW;
		szLocalAccountInfo.Expansion = false;
		Regist->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\WAR3", TRUE);
			strcpy(szLocalAccountInfo.EXE, Regist->Read("Game EXE", "Game EXE"));
			strcpy(szLocalAccountInfo.DLL1, Regist->Read("Game DLL", "Game DLL"));
			strcpy(szLocalAccountInfo.DLL2, Regist->Read("Game SNP", "Game SNP"));
		Regist->Close();
		break;
	case GAME_PX3W:
		version_byte = VER_PX3W;
		szLocalAccountInfo.Expansion = true;
		Regist->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\W3XP", TRUE);
			strcpy(szLocalAccountInfo.EXE, Regist->Read("Game EXE", "Game EXE"));
			strcpy(szLocalAccountInfo.DLL1, Regist->Read("Game DLL", "Game DLL"));
			strcpy(szLocalAccountInfo.DLL2, Regist->Read("Game SNP", "Game SNP"));
		Regist->Close();
		break;
	default:
		version_byte = VER_RATS;
		szLocalAccountInfo.Expansion = false;
		Regist->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\File Paths\\STAR", TRUE);
			strcpy(szLocalAccountInfo.EXE, Regist->Read("Game EXE", "Game EXE"));
			strcpy(szLocalAccountInfo.DLL1, Regist->Read("Game DLL", "Game DLL"));
			strcpy(szLocalAccountInfo.DLL2, Regist->Read("Game SNP", "Game SNP"));
		Regist->Close();
		break;
	}

	delete Regist;
	return version_byte;
}

void InitImageList(int iImgList)
{
	//Create new image list
	HIMAGELIST hiList = ImageList_Create(28, 14, ILC_COLOR16, 8, 1);

	//Load the icons into HBITMAP
	HBITMAP hbIcons = LoadBitmap(hDlgInst, MAKEINTRESOURCE(iImgList));

	//Insert them into the image list
	ImageList_Add(hiList, hbIcons, NULL);

	//Attach image list to the listview
	SendMessage(hListView, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)hiList);

	//Set a few colors
	SendMessage(hListView, LVM_SETBKCOLOR, 0, (LPARAM)0x00000000);
	SendMessage(hListView, LVM_SETTEXTBKCOLOR, 0, (LPARAM)0x00000000);
	SendMessage(hListView, LVM_SETTEXTCOLOR, 0, (LPARAM)GREEN);
}

int GetItemIndex(int ListView, char *item)
{
	int ItemCount = SendMessage(hListView, LVM_GETITEMCOUNT, 0, 0);
	for(int i = 0; i < ItemCount; i++){
		char buffer[32] = "";
		ListView_GetItemText(hListView, i, 0, buffer, sizeof(buffer));
		if(!stricmp(buffer, item))
			return i;
	}
	return -1;
}

void InsertItem(char *szItem, int iIcon)
{
	SendMessage(hListView, LVM_SETTEXTCOLOR, 0, (LPARAM)GREEN);

	LVITEM *lviItem = new LVITEM;
	lviItem->mask =  LVIF_IMAGE | LVIF_TEXT; //Image + Text
	lviItem->iItem = (int)SendMessage(hListView, LVM_GETITEMCOUNT, 0, 0);
	lviItem->iSubItem = 0;
	lviItem->pszText = szItem;
	lviItem->iImage = iIcon;
	SendMessage(hListView, LVM_INSERTITEM, 0, (LPARAM)lviItem);
	SendMessage(hListView, LVM_ARRANGE, LVA_ALIGNLEFT, 0); //So there aren't holes in the list when something is removed
}

void ModifyItem(int ListView, char *item, int icon)
{
	int itemcount = SendMessage(hListView, LVM_GETITEMCOUNT, 0, 0);
	int pos = GetItemIndex(IDC_CHANLIST, item);
	LVITEM lvi;
	lvi.mask =  LVIF_IMAGE | LVIF_TEXT;
	lvi.iItem = pos;
	lvi.iSubItem = 0;
	lvi.pszText = item;
	lvi.iImage = icon;
	SendMessage(hListView, LVM_DELETEITEM, pos, 0);
	SendMessage(hListView, LVM_INSERTITEM, 0, (LPARAM)&lvi);
}

// BATTLE.NET FUNCTIONS

void SendBNCSPacket(SOCKET sckBNCS, char bId)
{
	unsigned short uPacketLen = (unsigned short)dBuf.length() + 4;
	char *lpszPacketBuffer = new char[uPacketLen];
	lpszPacketBuffer[0] = (unsigned char)0xff;
	lpszPacketBuffer[1] = bId;
	*(unsigned short *)(lpszPacketBuffer + 2) = uPacketLen;
	dBuf.get(lpszPacketBuffer + 4, dBuf.length());
	send(sckBNCS, lpszPacketBuffer, uPacketLen, 0);
	lpszPacketBuffer = 0;
	if(lpszPacketBuffer)
		delete [] lpszPacketBuffer;
	dBuf.clear();
}

void ParseBNCS(SOCKET sckBNCS, char *lpszBuffer, int iLen)
{
	int iBufLen = iLen;
	while((int)iBufLen >= 4 && (unsigned char)lpszBuffer[0] == 0xff){
		char bPacketId = (unsigned char)lpszBuffer[1];
		unsigned short uPacketLen = *(unsigned short *)(lpszBuffer + 2);
		switch(bPacketId){
		case 0x0:
			{
				SendBNCSPacket(sckBNCS, 0x0);
				break;
			}
		case SID_PING:
			{
				unsigned long dwPingSeed = *(unsigned long *)(lpszBuffer + PACKET_HEAD);
					dBuf.add(dwPingSeed);
					SendBNCSPacket(sckBNCS, SID_PING);

				if(szLocalAccountInfo.IdleOn) {
					if(IdleTicker > 0)
						IdleTicker--;

					if(IdleTicker <= 0) {
						if(szLocalAccountInfo.IdleSpec)
							Send(sckBNCS, "%s", szLocalAccountInfo.IdleMessage);
						if(szLocalAccountInfo.IdleQuotes)
							SendRandomQuote();
						IdleTicker = (szLocalAccountInfo.IdleTimer / 30);
					}
				}
				break;
			}
		case SID_AUTH_INFO:
			{
				unsigned long LogonType = *(unsigned long *)(lpszBuffer + PACKET_HEAD);
				ServerToken = *(unsigned long *)(lpszBuffer + PACKET_HEAD + 4);

				char mpqName[32];
				strcpy(mpqName, lpszBuffer + (PACKET_HEAD + 20));

				char ChecksumFormula[256];
				strcpy(ChecksumFormula, lpszBuffer + (PACKET_HEAD + 33));
				
				char ServerSignature[128];
				memcpy(ServerSignature, lpszBuffer + (PACKET_HEAD + 34 + strlen(ChecksumFormula)), 128);
				HandleAuthInfo(sckBNCS, ChecksumFormula, mpqName, ServerSignature, LogonType);
				break;
			}
		case SID_MESSAGEBOX:
			{
				char Text[128], Caption[128];
				unsigned long Style = *(unsigned long *)(lpszBuffer + PACKET_HEAD);
				strcpy(Text, lpszBuffer + (PACKET_HEAD + 4));
				strcpy(Caption, lpszBuffer + (PACKET_HEAD + 4 + strlen(Text)));

				MessageBox(hMainDlg, Text, "SID_MESSAGEBOX", MB_OK);
				break;
			}
		case SID_LOGONRESPONSE2:
			{
				unsigned long LogonResult = *(unsigned long*)(lpszBuffer + PACKET_HEAD);
				HandleLogonResponse(LogonResult);
				break;
			}
		case SID_ENTERCHAT:
			{
				strcpy(szLocalAccountInfo.szRealUsername, ((char *)lpszBuffer + PACKET_HEAD));
				AppendText(hBNChat, WHITE, "Your name: %s\n", (char *)lpszBuffer + PACKET_HEAD);
				AppendText(hBNChat, WHITE, "Base name: %s\n", szLocalAccountInfo.szUsername);
				break;
			}
		case SID_GETCHANNELLIST:
			{
				break;
			}
		case SID_FLOODDETECTED:
			{
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				break;
			}
		case SID_UDPPINGRESPONSE:
			{
				break;
			}
		case SID_SETEMAIL:
			{
				SendSetEmail();
				break;
			}
		case SID_FRIENDSUPDATE:
			{
				break;
			}
		case SID_FRIENDSADD:
			{
				break;
			}
		case SID_FRIENDSREMOVE:
			{
				break;
			}
		case SID_FRIENDSPOSITION:
			{
				break;
			}
		case SID_GETICONDATA:
			{
				break;
			}
		case SID_AUTH_ACCOUNTLOGONPROOF:
			{
				HandleAuthLogonProof((char *)lpszBuffer);
				break;
			}
		case SID_AUTH_ACCOUNTLOGON:
			{
				HandleAuthLogon((char *)lpszBuffer);
				break;
			}
		case SID_AUTH_ACCOUNTCREATE:
			{
				HandleAuthCreate((char *)lpszBuffer);
				break;
			}
		case SID_CREATEACCOUNT2:
			{
				unsigned long ResultCode = *(unsigned long*)(lpszBuffer + PACKET_HEAD);
				HandleCreateAccount(ResultCode);
				break;
			}
		case SID_READUSERDATA:
			{
				unsigned long RequestID = *(unsigned long*)(lpszBuffer + PACKET_HEAD + 4 + 4);
				switch(RequestID) {
				case PROFILE_PROFILE:
					HandleReadUserData((char *)lpszBuffer);
					break;
				case PROFILE_RECORDDATA:
					HandleRecordData((char *)lpszBuffer);
					break;
				}
				break;
			}
		case SID_CHATEVENT:
			{
				unsigned long dwEvent = *(unsigned long *)(lpszBuffer + 4);
				unsigned long dwFlags = *(unsigned long *)(lpszBuffer + 8);
				unsigned long dwPing = *(unsigned long *)(lpszBuffer + 12);
				int iPos = 7 * sizeof(unsigned long);
				char *pszSpeaker = lpszBuffer + iPos;
				iPos += (strlen(pszSpeaker) + 1);
				char *pszSaid = lpszBuffer + iPos;
				
				switch(dwEvent) {
					case 1:
						{
							char String[400];
							char *pPtr = NewString(strlen(pszSpeaker) + 1);
							strcpy(pPtr, pszSpeaker);
							
							unsigned long dwProduct = *(unsigned long *)pszSaid;
							switch(dwProduct){
							case 'SSHR':
								sprintf(String, "In channel: %s:%02x:%d Starcraft shareware\n", pszSpeaker, dwFlags, dwPing);
								break;
							case 'DSHR':
								sprintf(String, "In channel: %s:%02x:%d Diablo shareware\n", pszSpeaker, dwFlags, dwPing);
								break;
							case 'DRTL':
								sprintf(String, "In channel: %s:%02x:%d Diablo\n", pszSpeaker, dwFlags, dwPing);
								break;
							case 'STAR':
								sprintf(String, "In channel: %s:%02x:%d Starcraft\n", pszSpeaker, dwFlags, dwPing);
								break;
							case 'SEXP':
								sprintf(String, "In channel: %s:%02x:%d Brood War\n", pszSpeaker, dwFlags, dwPing);
								break;
							case 'JSTR':
								sprintf(String, "In channel: %s:%02x:%d Starcraft japanese\n", pszSpeaker, dwFlags, dwPing);
								break;
							case 'W2BN':
								sprintf(String, "In channel: %s:%02x:%d Warcraft II\n", pszSpeaker, dwFlags, dwPing);
								break;
							case 'D2DV':
								sprintf(String, "In channel: %s:%02x:%d Diablo II\n", pszSpeaker, dwFlags, dwPing);
								break;
							case 'D2XP':
								sprintf(String, "In channel: %s:%02x:%d Lord of Destruction\n", pszSpeaker, dwFlags, dwPing);
								break;
							case 'WAR3':
								sprintf(String, "In channel: %s:%02x:%d Warcraft III\n", pszSpeaker, dwFlags, dwPing);
								break;
							case 'W3XP':
								sprintf(String, "In channel: %s:%02x:%d The Frozen Throne\n", pszSpeaker, dwFlags, dwPing);
								break;
							case 'CHAT':
								sprintf(String, "In channel: %s:%02x:%d Chat\n", pszSpeaker, dwFlags, dwPing);
								break;
							}
							AppendText(hBNChat, YELLOW, "%s", String);
							InsertItem(pszSpeaker, GetIconCode(dwProduct, dwFlags));

							char ChanNameText[128];
							sprintf(ChanNameText, "%s (%d)", szLocalAccountInfo.szCurrentChan, ListView_GetItemCount(GetDlgItem(hMainDlg, IDC_CHANLIST)));
							SetWindowText(GetDlgItem(hMainDlg, IDC_CHANNAME), ChanNameText);
							break;
						}
					case 2:
						{
							char String[400];
							char *pPtr = NewString(strlen(pszSpeaker) + 1);
							strcpy(pPtr, pszSpeaker);
							unsigned long dwProduct = *(unsigned long *)pszSaid;
							switch(dwProduct){
							case 'SSHR':
								sprintf(String, "%s:%02x:%d Starcraft shareware joined the channel", pszSpeaker, dwFlags, dwPing);
								break;
							case 'DSHR':
								sprintf(String, "%s:%02x:%d Diablo shareware joined the channel", pszSpeaker, dwFlags, dwPing);
								break;
							case 'DRTL':
								sprintf(String, "%s:%02x:%d Diablo joined the channel", pszSpeaker, dwFlags, dwPing);
								break;
							case 'STAR':
								sprintf(String, "%s:%02x:%d Starcraft joined the channel", pszSpeaker, dwFlags, dwPing);
								break;
							case 'SEXP':
								sprintf(String, "%s:%02x:%d Brood War joined the channel", pszSpeaker, dwFlags, dwPing);
								break;
							case 'JSTR':
								sprintf(String, "%s:%02x:%d Starcraft japanese joined the channel", pszSpeaker, dwFlags, dwPing);
								break;
							case 'W2BN':
								sprintf(String, "%s:%02x:%d Warcraft II joined the channel", pszSpeaker, dwFlags, dwPing);
								break;
							case 'D2DV':
								sprintf(String, "%s:%02x:%d Diablo II joined the channel", pszSpeaker, dwFlags, dwPing);
								break;
							case 'D2XP':
								sprintf(String, "%s:%02x:%d Lord of Destruction joined the channel", pszSpeaker, dwFlags, dwPing);
								break;
							case 'WAR3':
								sprintf(String, "%s:%02x:%d Warcraft III joined the channel", pszSpeaker, dwFlags, dwPing);
								break;
							case 'W3XP':
								sprintf(String, "%s:%02x:%d The Frozen Throne joined the channel", pszSpeaker, dwFlags, dwPing);
								break;
							case 'CHAT':
								sprintf(String, "%s:%02x:%d Chat joined the channel", pszSpeaker, dwFlags, dwPing);
								break;
							}
							AppendTextTS(hBNChat, GREEN, "%s\n", String);
							InsertItem(pszSpeaker, GetIconCode(dwProduct, dwFlags));

							char ChanNameText[128];
							sprintf(ChanNameText, "%s (%d)", szLocalAccountInfo.szCurrentChan, ListView_GetItemCount(GetDlgItem(hMainDlg, IDC_CHANLIST)));
							SetWindowText(GetDlgItem(hMainDlg, IDC_CHANNAME), ChanNameText);
							break;
						}
					case 3:
						{
							AppendTextTS(hBNChat, GREEN, "%s has left the channel\n", pszSpeaker);
							HWND DlgItem = GetDlgItem(hMainDlg, IDC_CHANLIST);

							LVFINDINFO *lviItem = new LVFINDINFO;
							lviItem->flags = LVFI_STRING;
							lviItem->psz = pszSpeaker;
							ListView_DeleteItem(DlgItem, ListView_FindItem(DlgItem, -1, lviItem));
							delete lviItem;

							SendMessage(DlgItem, LVM_ARRANGE, LVA_ALIGNLEFT, 0);
							char ChanNameText[128];
							sprintf(ChanNameText, "%s (%d)", szLocalAccountInfo.szCurrentChan, ListView_GetItemCount(GetDlgItem(hMainDlg, IDC_CHANLIST)));
							SetWindowText(GetDlgItem(hMainDlg, IDC_CHANNAME), ChanNameText);
							break;
						}
					case 4:
						{
							AppendTextTS(hBNChat, LIGHTBLUE, "%s:%02x whispers: %s\n", pszSpeaker, dwFlags, pszSaid);
							strcpy(szLocalAccountInfo.szWhisperTo, pszSpeaker);
							break;
						}
					case 5:
						{
							AppendTextTS(hBNChat, WHITE, "<");
							AppendText(hBNChat, LGREEN, "%s", pszSpeaker);
							AppendText(hBNChat, WHITE, "> %s\n", pszSaid);
							break;
						}
					case 6:
						{
							AppendTextTS(hBNChat, YELLOW, "Broadcast: (%s) %s\n", pszSpeaker, pszSaid);
							break;
						}
					case 7:
						{
							char String[400];
							ListView_DeleteAllItems(GetDlgItem(hMainDlg, IDC_CHANLIST));
							strcpy(szLocalAccountInfo.szCurrentChan, pszSaid);
							switch(dwFlags) {
							case 1:
								sprintf(String, "Joined public channel: %s", pszSaid);
								break;
							case 0:
								sprintf(String, "Joined moderated channel: %s", pszSaid);
								break;
							case 4:
								sprintf(String, "Joined restricted channel: %s", pszSaid);
								break;
							case 8:
								sprintf(String, "Joined silent channel: %s", pszSaid);
								break;
							case 16:
								sprintf(String, "Joined system channel: %s", pszSaid);
								break;
							case 33:
								sprintf(String, "Joined product-specific channel: %s", pszSaid);
								break;
							case 4096:
								sprintf(String, "Joined globally accessible channel: %s", pszSaid);
								break;
							default:
								sprintf(String, "Joined 0x%02x flagged channel: %s", dwFlags, pszSaid);
								break;
							}
							AppendTextTS(hBNChat, RED, "%s\n", String);

							if(szLocalAccountInfo.StartedUp) {
								if(szLocalAccountInfo.UseStartmsg)
									Send(sckBNCS, "%s", szLocalAccountInfo.StartupMsg);
								szLocalAccountInfo.StartedUp = false;
							}

							break;
						}
					case 9:
						{
							unsigned long dwProduct = *(unsigned long *)pszSaid;

							ModifyItem(IDC_CHANLIST, pszSpeaker, GetIconCode(dwProduct, dwFlags));
							if(dwFlags & 0x02){
								if(!stricmp(szLocalAccountInfo.szRealUsername, pszSpeaker))
									AppendTextTS(hBNChat, GRAY, "You have acquired operator status.\n");
								else
									AppendTextTS(hBNChat, GRAY, "%s has acquired operator status.\n", pszSpeaker);
							}
						break;
						}
					case 10:
						{
							AppendTextTS(hBNChat, LIGHTBLUE, "You whisper %s:%02x: %s\n", pszSpeaker, dwFlags, pszSaid);
							strcpy(szLocalAccountInfo.szWhisperTo, pszSpeaker);
							break;
						}
					case 13:
						{
							break;
						}
					case 14:
						{
							break;
						}
					case 15:
						{
							break;
						}
					case 18:
						{
							AppendTextTS(hBNChat, YELLOW, "Info: %s\n", pszSaid);
							break;
						}
					case 19:
						{
							AppendTextTS(hBNChat, RED, "Error: %s\n", pszSaid);
							break;
						}
					case 23:
						{
							AppendTextTS(hBNChat, YELLOW, "* %s %s *\n", pszSpeaker, pszSaid);
							break;
						}
					default:
						{
							AppendTextTS(hBNChat, RED, "Unknown EID_%02x\n", dwEvent);
							break;
						}
				}

				break;
			}
		case SID_AUTH_CHECK:
			{
				unsigned long dwResult = *(unsigned long *)(lpszBuffer + PACKET_HEAD);
				HandleAuthCheck(dwResult);
				break;
			}
		default:
			{
				AppendText(hBNChat, WHITE, "Received unknown packet: FF 0x%02x\n", bPacketId);
				break;
			}
		}
		memmove(lpszBuffer, lpszBuffer + uPacketLen, iBufLen - uPacketLen);
		iBufLen -= uPacketLen;
	}
}

void SendAuthInfo(SOCKET sckBNCS)
{
	if(szLocalAccountInfo.Connected) {
		dBuf.add((int)0);														
		dBuf.add((int)'IX86');													
		dBuf.add(szLocalAccountInfo.iGameAbbr());							
		dBuf.add(szLocalAccountInfo.lVerByte);
		dBuf.add((int)0);
		dBuf.add((int)0);
		dBuf.add((int)480);
		dBuf.add((int)1033);
		dBuf.add((int)1033);
		dBuf.add("USA");
		dBuf.add("United States");
		SendBNCSPacket(sckBNCS, SID_AUTH_INFO);
	} else {
		AppendText(hBNChat, RED, "Error while connecting\n");
	}
}

void HandleAuthInfo(SOCKET sckBNCS, const char *ChecksumFormula, char *lpszMPQName, char *ServerSignature, unsigned long LogonType)
{
	if(szLocalAccountInfo.Connected) {
		unsigned int mpqNumber;

		switch(LogonType) {
		case 0:
			{
				// Old Logon System
				szLocalAccountInfo.UseNLS = false;
				break;
			}
		case 1:
			{
				// WC3 Beta NLS
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "The NLS revision that the server has requested is not supported!\n");
				break;
			}
		case 2:
			{
				// New Logon System
				szLocalAccountInfo.UseNLS = true;
				break;
			}
		default:
			{
				// Who knows?
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Unsupported logon system\n");
				break;
			}
		}
		kd_init();
		AppendText(hBNChat, WHITE, "Checking program version\n");

		mpqNumber = extractMPQNumber(lpszMPQName);
		if(mpqNumber < 0) {
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			AppendText(hBNChat, RED, "Unknown MPQ number\n");
		}

		if(szLocalAccountInfo.UseNLS) {
			if(!nls_check_signature(sName.sin_addr.s_addr, ServerSignature)) {
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected= false;
				AppendText(hBNChat, RED, "Server signature check failed\nConnection closed\n");
			}
		}

		const char* files[] = { szLocalAccountInfo.EXE, szLocalAccountInfo.DLL1, szLocalAccountInfo.DLL2 };
		unsigned long Checksum;
		if(!checkRevision(ChecksumFormula, files, 3, mpqNumber, &Checksum)) {
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			AppendText(hBNChat, RED, "Version check failed\n");
		}

		unsigned long EXEVersion;
		unsigned int Version;
		char EXEInfo[300];
		EXEVersion = getExeInfo(szLocalAccountInfo.EXE, EXEInfo, 256, &Version, 0x1);
		if(!EXEVersion) {
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			AppendText(hBNChat, RED, "Check file location at %s\n", szLocalAccountInfo.EXE);
		}

		if(EXEVersion >= sizeof(EXEInfo)) {
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			AppendText(hBNChat, RED, "Info variable not large enough. Resize to %x\n", EXEVersion);
		}
		
		ClientToken = time(0) + GetTickCount();
		dBuf.add(ClientToken);
		dBuf.add(EXEVersion);
		dBuf.add(Checksum);

		switch(szLocalAccountInfo.Expansion) {
		case true:
			{
				dBuf.add((int)0x02);
				break;
			}
		default:
			{
				dBuf.add((int)0x01);
				break;
			}
		}
		
		switch(szLocalAccountInfo.Spawn) {
		case true:
			{
				dBuf.add((int)1);
				break;
			}
		default:
			{
				dBuf.add((int)0);
				break;
			}
		}

		CDKeyDecoder decoder(szLocalAccountInfo.szCDkey, strlen(szLocalAccountInfo.szCDkey));
		if(!decoder.isKeyValid()) {
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			AppendText(hBNChat, RED, "Invalid CD key\n");
		}

		unsigned long getProd, getVal;
		getProd = decoder.getProduct();
		getVal = decoder.getVal1();
		dBuf.add((int)strlen(szLocalAccountInfo.szCDkey));
		dBuf.add(getProd);
		dBuf.add(getVal);
		dBuf.add((int)0x00);

		int hashLength = decoder.calculateHash(ClientToken, ServerToken);
		if(!hashLength) {
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			AppendText(hBNChat, RED, "Hash was rejected\n");
		}

		char KeyHash[64];
		if(!decoder.getHash(KeyHash)) {
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			AppendText(hBNChat, RED, "CD key hash was rejected\n");
		}

		dBuf.add(KeyHash, 5 * 4);

		if(szLocalAccountInfo.Expansion) 
		{
			kd_init();
			CDKeyDecoder expdecoder(szLocalAccountInfo.szExpCDkey, strlen(szLocalAccountInfo.szExpCDkey));
			if(!expdecoder.isKeyValid()) {
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Invalid expansion CD key\n");
			}

			unsigned long expgetProd, expgetVal;
			expgetProd = expdecoder.getProduct();
			expgetVal = expdecoder.getVal1();
			dBuf.add((int)strlen(szLocalAccountInfo.szExpCDkey));
			dBuf.add(expgetProd);
			dBuf.add(expgetVal);
			dBuf.add((int)0x00);

			int exphashLength = expdecoder.calculateHash(ClientToken, ServerToken);
			if(!exphashLength) {
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Expansion hash was rejected\n");
			}

			char expKeyHash[64];
			if(!expdecoder.getHash(expKeyHash)) {
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Expansion CD key hash was rejected\n");
			}

			dBuf.add(expKeyHash, 5 * 4);
		}

		dBuf.add(EXEInfo);
		dBuf.add(szLocalAccountInfo.szUsername);

		if(szLocalAccountInfo.Connected)
			SendBNCSPacket(sckBNCS, SID_AUTH_CHECK);

	} else {
		AppendText(hBNChat, RED, "Error while connecting\n");
	}
}

void HandleAuthCheck(unsigned long Result) {
	switch(Result) {
		case 0x00:
			{	
				// OK.
				AppendText(hBNChat, WHITE, "Version information accepted\n");
				if(szLocalAccountInfo.UseNLS)
				{
					SendAuthLogon();
				} else {
					SendUDPPingResponse();
					SendGetIconData();
					SendLogonResponse();
				}
				break;
			}
		case 0x100:
			{
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Game version is out of date\n");
				break;
			}
		case 0x101:
			{
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Invalid game version\n");
				break;
			}
		case 0x200:
			{
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Invalid CD key for this product ID\n");
				break;
			}
		case 0x201:
			{
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "CD key is use\nConnection closed\n");
				break;
			}
		case 0x202:
			{
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "CD-key is banned by Battle.net\n");
				break;
			}
		case 0x203:
			{
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Invalid CD key for this product ID\n");
				break;
			}
		case 0x210:
			{
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Invalid expansion CD key for this product ID\n");
				break;
			}
		case 0x211:
			{
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Expansion CD key is use by someone\n");
				break;
			}
		case 0x212:
			{
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Expansion CD-key is banned by Battle.net\n");
				break;
			}
		case 0x213:
			{
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Invalid expansion CD key for this product ID\n");
				break;
			}
		default:
			{
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "SID_AUTH_CHECK failed for an unknown reason\n");
				break;
			}
		}

}

void SendLogonResponse(void) {
	if(szLocalAccountInfo.Connected) {
		char outBuffer[20];

		doubleHashPassword(szLocalAccountInfo.szPassword, ClientToken, ServerToken, outBuffer);
		AppendText(hBNChat, WHITE, "Attempting to log on\n", outBuffer);

		dBuf.add(ClientToken);
		dBuf.add(ServerToken);
		dBuf.add(outBuffer, 5 * sizeof(unsigned long));
		dBuf.add(szLocalAccountInfo.szUsername);
		SendBNCSPacket(sckBNCS, SID_LOGONRESPONSE2);
	}
}

void HandleLogonResponse(unsigned long Result) {
	if(szLocalAccountInfo.Connected) {
		switch(Result) {
		case 0:
			{
				SendEnterChat();
				break;
			}
		case 1:
			{
				AppendText(hBNChat, RED, "Failed logon, trying to make account\n");
				CreateDialog(hDlgInst, MAKEINTRESOURCE(IDD_CREATE_ACCT), hMainDlg, AcctCreateDialogProc);
				break;
			}
		case 2:
			{
				AppendText(hBNChat, RED, "Failed logon\n");
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
				AppendText(hBNChat, RED, "Connection closed\n");
				break;
			}
		}
	}
}

void SendUDPPingResponse(void) {
	if(szLocalAccountInfo.Connected) {
		dBuf.add((int)'bnet');
		SendBNCSPacket(sckBNCS, SID_UDPPINGRESPONSE);
	}
}

void SendGetIconData(void) {
	if(szLocalAccountInfo.Connected) {
		SendBNCSPacket(sckBNCS, SID_GETICONDATA);
	}
}

void SendEnterChat(void) {
	if(szLocalAccountInfo.Connected) {
		dBuf.add(szLocalAccountInfo.szUsername);
		dBuf.add(szLocalAccountInfo.szGameAbbr);
		SendBNCSPacket(sckBNCS, SID_ENTERCHAT);

		dBuf.add(szLocalAccountInfo.szGameAbbr, 4);
		SendBNCSPacket(sckBNCS, SID_GETCHANNELLIST);

		dBuf.add((int)2);
		dBuf.add(szLocalAccountInfo.szHomeChannel);
		SendBNCSPacket(sckBNCS, SID_JOINCHANNEL);
		szLocalAccountInfo.Connected = true;
	}
}

void SendCreateAccount(void) {
	char PassHash[20];

	hashPassword(szLocalAccountInfo.szPassword, PassHash);
	dBuf.add(PassHash, 5 * sizeof(unsigned long));
	dBuf.add(szLocalAccountInfo.szUsername);
	SendBNCSPacket(sckBNCS, SID_CREATEACCOUNT2);
}

void HandleCreateAccount(unsigned long Result) {

	switch(Result) {
	case 0:
		// OK
		break;
	case 2:
		{
			AppendText(hBNChat, RED, "Username contains invalid characters\n");
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			break;
		}
	case 3:
		{
			AppendText(hBNChat, RED, "Username contains banned words\n");
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			break;
		}
	case 4:
		{
			AppendText(hBNChat, RED, "%s already exists\n", szLocalAccountInfo.szUsername);
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			break;
		}
	case 6:
		{
			AppendText(hBNChat, RED, "Username is too short\n");
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			break;
		}
	default:
		{
			AppendText(hBNChat, RED, "Unknown account creation result\n");
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			break;
		}
	}

	if(szLocalAccountInfo.Connected) {
		AppendText(hBNChat, WHITE, "Created account, logging on\n");
		SendLogonResponse();
	} else {
		AppendText(hBNChat, RED, "Error while connecting\n");
	}
}

void SendSetEmail(void) {
	AppendText(hBNChat, WHITE, "The specified email address has been registered to your account\n");
	dBuf.add(szLocalAccountInfo.szEmailAddy);
	SendBNCSPacket(sckBNCS, SID_SETEMAIL);
}

int LocalAccountInfo::iGameAbbr(void) {
	int iAbbr = 'STAR';

	if(!strcmp(szLocalAccountInfo.szGameAbbr, "RATS"))
		iAbbr = 'STAR';
	if(!strcmp(szLocalAccountInfo.szGameAbbr, "PXES"))
		iAbbr = 'SEXP';
	if(!strcmp(szLocalAccountInfo.szGameAbbr, "NB2W"))
		iAbbr = 'W2BN';
	if(!strcmp(szLocalAccountInfo.szGameAbbr, "VD2D"))
		iAbbr = 'D2DV';
	if(!strcmp(szLocalAccountInfo.szGameAbbr, "PX2D"))
		iAbbr = 'D2XP';
	if(!strcmp(szLocalAccountInfo.szGameAbbr, "3RAW"))
		iAbbr = 'WAR3';
	if(!strcmp(szLocalAccountInfo.szGameAbbr, "PX3W"))
		iAbbr = 'W3XP';

	return iAbbr;
}

void Send(SOCKET sckBNCS, char *lpszFmt, ...)
{
	char szTxt[2048] = "";
	va_list vaArg;
	va_start(vaArg, lpszFmt);
	vsprintf(szTxt, lpszFmt, vaArg);
	va_end(vaArg);
	char *pPtr = NewString(strlen(szTxt) + 1);
	strcpy(pPtr, szTxt);

	if(szLocalAccountInfo.Connected) {	
		dBuf.add(pPtr);
		SendBNCSPacket(sckBNCS, SID_CHATCOMMAND);
	}
}

void DoSendText(void) {
	if(szLocalAccountInfo.Connected) {
		char SendText[200];
		GetWindowText(GetDlgItem(hMainDlg, IDC_BNSEND), SendText, sizeof(SendText));
		if(strlen(SendText) > 0) {
			SetWindowText(GetDlgItem(hMainDlg, IDC_BNSEND), "");
			if(SendText[0] == '/') {
				HandleCommand((char *)SendText, false);
			} else {
				AppendTextTS(hBNChat, WHITE, "<");
				AppendText(hBNChat, TEAL, "%s", szLocalAccountInfo.szRealUsername);
				AppendText(hBNChat, WHITE, "> %s\n", SendText);
				Send(sckBNCS, "%s", SendText);
			}
		}
	} else {
		AppendText(hBNChat, RED, "Not connected\n");
	}
}

void SendAuthLogon(void) {
	char Var_A[32];

	if(!NLS) {
		NLS = nls_init(szLocalAccountInfo.szUsername, szLocalAccountInfo.szPassword);
		if(!NLS) {
			AppendText(hBNChat, RED, "Failed to initialize NLS\n");
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			AppendText(hBNChat, RED, "Connection closed\n");
		}
	}

	nls_get_A(NLS, Var_A);

	dBuf.add(Var_A, 32);
	dBuf.add(szLocalAccountInfo.szUsername);
	SendBNCSPacket(sckBNCS, SID_AUTH_ACCOUNTLOGON);
}

void HandleAuthLogon(char *data) {
	char M1[20];
	char Salt[32];
	char Var_B[32];

	unsigned long ResultCode = *(unsigned long*)(data + PACKET_HEAD);

	switch(ResultCode) {
	case 0x00:
		{
			memcpy(Salt, data + (PACKET_HEAD + 4), 32);
			memcpy(Var_B, data + (PACKET_HEAD + 4 + 32), 32);
			
			nls_get_M1(NLS, M1, Var_B, Salt);

			dBuf.add(M1, 20);
			SendBNCSPacket(sckBNCS, SID_AUTH_ACCOUNTLOGONPROOF);
			break;
		}
	case 0x01:
		{
			SendAuthCreate();
			break;
		}
	case 0x05:
		{
			AppendText(hBNChat, RED, "Account requires upgrade\n");
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			break;
		}
	default:
		{
			AppendText(hBNChat, RED, "Unknown logon result\n");
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			break;
		}
	}
}

void HandleAuthLogonProof(char *data) {
	char M2[20];

	unsigned long ResultCode = *(unsigned long*)(data + PACKET_HEAD);

	switch(ResultCode) {
	case 0:
		{
			memcpy(M2, data + (PACKET_HEAD + 4), 20);
			if(!nls_check_M2(NLS, M2, NULL, NULL)) {
				AppendText(hBNChat, RED, "Server password proof check failed\n");
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
			}

			FreeNLS();
			SendEnterChat();
			break;
		}
	case 2:
		{
			AppendText(hBNChat, RED, "Incorrect password\n");
		    CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			break;
		}
	case 0xE:
		{
			SendSetEmail();
			memcpy(M2, data + (PACKET_HEAD + 4), 20);
			if(!nls_check_M2(NLS, M2, NULL, NULL)) {
				AppendText(hBNChat, RED, "Server password proof check failed\n");
				CloseTCPSocket(sckBNCS);
				szLocalAccountInfo.Connected = false;
			}

			FreeNLS();
			SendEnterChat();
			break;
		}
	default:
		{
			AppendText(hBNChat, RED, "Unknown logon proof error\n");
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			break;
		}
	}
}

void FreeNLS(void) {
	if(NLS) {
		nls_free(NLS);
		NLS = 0;
	}
}

void SendAuthCreate(void) {
	long BufLen = (65 + strlen(szLocalAccountInfo.szUsername));
	char *AuthBuf = new char[BufLen];

	memset(AuthBuf, 0, BufLen);
	if(!nls_account_create(NLS, AuthBuf, BufLen)) {
		AppendText(hBNChat, RED, "Failed to generate account create packet\n");
		CloseTCPSocket(sckBNCS);
		szLocalAccountInfo.Connected = false;
	}

	if(szLocalAccountInfo.Connected) {
		AppendText(hBNChat, WHITE, "Attempting to create %s...\n", szLocalAccountInfo.szUsername);

		dBuf.add(AuthBuf, BufLen);
		SendBNCSPacket(sckBNCS, SID_AUTH_ACCOUNTCREATE);
	}

	delete AuthBuf;
}

void HandleAuthCreate(char *data) {
	unsigned long ResultCode = *(unsigned long*)(data + PACKET_HEAD);

	switch(ResultCode) {
	case 0:
		// OK
		break;
	case 7: case 8: case 9: case 0xA: case 0xB: case 0xC: {
		AppendText(hBNChat, RED, "The username you're trying to create is invalid\n");
		CloseTCPSocket(sckBNCS);
		szLocalAccountInfo.Connected = false;
		break;
		}
	default:
		{
			AppendText(hBNChat, RED, "%s already exists\n", szLocalAccountInfo.szUsername);
			CloseTCPSocket(sckBNCS);
			szLocalAccountInfo.Connected = false;
			AppendText(hBNChat, RED, "Connection closed\n");
			break;
		}
	}

	if(szLocalAccountInfo.Connected) {
		SendAuthLogon();
	}
}

void SendRandomQuote(void) {
	char *test[200];
    char buf[255];
	char otherbuf[255];
    int i = 0, y = 0;

    FILE *fp = fopen("Quotes.txt", "r");
    if (fp) {
           while (fgets(buf, sizeof(buf), fp)) {
                   test[i] = (char *)malloc(strlen(buf)+1);
                   strcpy(test[i], buf);
                   i++;
           }
    }

	y = (GetTickCount() % (i - 1));

	strcpy(otherbuf, test[y]);
		strtok(otherbuf, "\r\n");
		if(strlen(otherbuf) > 0)
			Send(sckBNCS, "/me %s - SoupBot v2", otherbuf);
}

void SendRandomQuote2(void) {
	char *test[200];
    char buf[255];
	char otherbuf[255];
    int i = 0, y = 0;

    FILE *fp = fopen("Quotes.txt", "r");
    if (fp) {
           while (fgets(buf, sizeof(buf), fp)) {
                   test[i] = (char *)malloc(strlen(buf)+1);
                   strcpy(test[i], buf);
                   i++;
           }
    }

	y = (GetTickCount() % (i - 1));

	strcpy(otherbuf, test[y]);
		strtok(otherbuf, "\r\n");
		if(strlen(otherbuf) > 0)
		{
			Send(sckBNCS, "%s", otherbuf);
			AppendTextTS(hBNChat, WHITE, "<");
			AppendText(hBNChat, TEAL, "%s", szLocalAccountInfo.szRealUsername);
			AppendText(hBNChat, WHITE, "> %s\n", otherbuf);
		}
}

void SendLeaveChat(void) {
	SendBNCSPacket(sckBNCS, SID_LEAVECHAT);
}

int GetIconCode(unsigned long product, int flags)
{
	int iconcode;
	switch(product) {
		case 'SSHR':
			iconcode = ICO_SSHR;
			break;
		case 'DSHR':
			iconcode = ICO_DSHR;
			break;
		case 'DRTL':
			iconcode = ICO_DRTL;
			break;
		case 'STAR':
			iconcode = ICO_SC;
			break;
		case 'SEXP':
			iconcode = ICO_BW;
			break;
		case 'JSTR':
			iconcode = ICO_SCJ;
			break;
		case 'W2BN':
			iconcode = ICO_WAR2;
			break;
		case 'D2DV':
			iconcode = ICO_D2;
			break;
		case 'D2XP':
			iconcode = ICO_D2X;
			break;
		case 'WAR3':
			iconcode = ICO_WAR3;
			break;
		case 'W3XP':
			iconcode = ICO_W3XP;
			break;
		case 'CHAT':
			iconcode = ICO_CHAT;
			break;
		default:
			iconcode = ICO_BRX;
			break;
	}

	if(flags & 0x01) iconcode = ICO_BLIZZ;
	if(flags & 0x02) iconcode = ICO_OP;
	if(flags & 0x04) iconcode = ICO_BLIZZ;
	if(flags & 0x08) iconcode = ICO_BLIZZ;
	if(flags & 0x20) iconcode = ICO_BRX;
	if(flags & 0x40) iconcode = ICO_BLIZZ;
	return iconcode;
}

void GetProfile(char *Account) {
	char name[35];
	strcpy(name, Account);

	if(!strncmp(Account, "*", 1)) {
		char seps[] = "*";
		char *token;
		token = strtok(Account, seps);
		strcpy(name, token);
	}

	strcpy(szProfileData.szAccount, name);
	dBuf.add(1);
	dBuf.add(3);
	dBuf.add((unsigned long)PROFILE_PROFILE);
	dBuf.add(name);
	dBuf.add("Profile\\Sex");
	dBuf.add("Profile\\Location");
	dBuf.add("Profile\\Description");
	SendBNCSPacket(sckBNCS, SID_READUSERDATA);
}

void GetRecordData(char *Account) {
	char name[35];
	strcpy(name, Account);

	if(!strncmp(Account, "*", 1)) {
		char seps[] = "*";
		char *token;
		token = strtok(Account, seps);
		strcpy(name, token);
	}

	if(!strcmp(szLocalAccountInfo.szRealUsername, name)) {
		strcpy(szProfileData.szAccount, name);
		dBuf.add(1);
		dBuf.add(4);
		dBuf.add((unsigned long)PROFILE_RECORDDATA);
		dBuf.add(name);
		dBuf.add("System\\Account Created");
		dBuf.add("System\\Last Logon");
		dBuf.add("System\\Last Logoff");
		dBuf.add("System\\Time Logged");
		SendBNCSPacket(sckBNCS, SID_READUSERDATA);
	}
}

void HandleReadUserData(char *data) {
	unsigned long Accounts = *(unsigned long*)(data + PACKET_HEAD);
	unsigned long Keys = *(unsigned long*)(data + PACKET_HEAD + 4);
	
	strcpy(szProfileData.szSex, data + PACKET_HEAD + 12);
	strcpy(szProfileData.szLocation, data + PACKET_HEAD + 13 + strlen(szProfileData.szSex));
	strcpy(szProfileData.szDescription, data + PACKET_HEAD + 14 + strlen(szProfileData.szSex) + strlen(szProfileData.szLocation));

	CreateDialog(hDlgInst, MAKEINTRESOURCE(IDD_PROFILE), hMainDlg, ProfileDialogProc);	
	return;
}

void HandleRecordData(char *data) {
	int x, length;
	FILETIME Time;
	SYSTEMTIME SysTime;
	char AcctCreatedHigh[128], AcctCreatedLow[128];
	char LastLogonHigh[128], LastLogonLow[128];
	char LastLogoffHigh[128], LastLogoffLow[128];
	char TimeLogged[128], Month[32], Day[32];

	strcpy(AcctCreatedHigh, data + (PACKET_HEAD + 12));
	for(x = 0; x < strlen(AcctCreatedHigh); x++) {
		if(AcctCreatedHigh[x] == 0x20) {
			AcctCreatedHigh[x] = '\0';
		}
	}
	strcpy(AcctCreatedLow, data + (PACKET_HEAD + 12 + strlen(AcctCreatedHigh) + 1));
	Time.dwHighDateTime = atol(AcctCreatedHigh);
	Time.dwLowDateTime = atol(AcctCreatedLow);
	FileTimeToSystemTime(&Time, &SysTime);
	GetMonth(SysTime.wMonth, Month);
	GetDay(SysTime.wDayOfWeek, Day);
	AppendText(hBNChat, GRAY, "%s\\Recorddata\\Account Created: %s, %s %i, %i %i:%i:%i %s\n", szLocalAccountInfo.szRealUsername, Day, Month, SysTime.wDay, SysTime.wYear, SysTime.wHour > 12 ? SysTime.wHour - 12 : SysTime.wHour, SysTime.wMinute, SysTime.wSecond, SysTime.wHour > 12 ? "AM" : "PM");

	length = PACKET_HEAD + 12 + strlen(AcctCreatedHigh) + 1 + strlen(AcctCreatedLow) + 1;
	strcpy(LastLogonHigh, data + length);
	for(x = 0; x < strlen(LastLogonHigh); x++) {
		if(LastLogonHigh[x] == 0x20) {
			LastLogonHigh[x] = '\0';
		}
	}
	length += (strlen(LastLogonHigh) + 1);
	strcpy(LastLogonLow, data + length);
	Time.dwHighDateTime = atol(LastLogonHigh);
	Time.dwLowDateTime = atol(LastLogonLow);
	FileTimeToSystemTime(&Time, &SysTime);
	GetMonth(SysTime.wMonth, Month);
	GetDay(SysTime.wDayOfWeek, Day);
	AppendText(hBNChat, GRAY, "%s\\Recorddata\\Last Logon: %s, %s %i, %i %i:%i:%i %s\n", szLocalAccountInfo.szRealUsername, Day, Month, SysTime.wDay, SysTime.wYear, SysTime.wHour > 12 ? SysTime.wHour - 12 : SysTime.wHour, SysTime.wMinute, SysTime.wSecond, SysTime.wHour > 12 ? "AM" : "PM");


	length += (strlen(LastLogonLow) + 1);
	strcpy(LastLogoffHigh, data + length);
	for(x = 0; x < strlen(LastLogoffHigh); x++) {
		if(LastLogoffHigh[x] == 0x20) {
			LastLogoffHigh[x] = '\0';
		}
	}
	length += (strlen(LastLogoffHigh) + 1);
	strcpy(LastLogoffLow, data + length);
	Time.dwHighDateTime = atol(LastLogoffHigh);
	Time.dwLowDateTime = atol(LastLogoffLow);
	FileTimeToSystemTime(&Time, &SysTime);
	GetMonth(SysTime.wMonth, Month);
	GetDay(SysTime.wDayOfWeek, Day);
	AppendText(hBNChat, GRAY, "%s\\Recorddata\\Last Logoff: %s, %s %i, %i %i:%i:%i %s\n", szLocalAccountInfo.szRealUsername, Day, Month, SysTime.wDay, SysTime.wYear, SysTime.wHour > 12 ? SysTime.wHour - 12 : SysTime.wHour, SysTime.wMinute, SysTime.wSecond, SysTime.wHour > 12 ? "AM" : "PM");

	length += (strlen(LastLogoffLow) + 1);
	strcpy(TimeLogged, data + length);
	int iTimeLogged = atol(TimeLogged);
	int days = iTimeLogged / 86400;
	iTimeLogged -= days * 86400;

	int hours = iTimeLogged / 3600;
	iTimeLogged -= hours * 3600;

	int minutes = iTimeLogged / 60;
	iTimeLogged -= minutes * 60;

	AppendText(hBNChat, GRAY, "%s\\Recorddata\\Time Logged: %i days, %i hours, %i minutes, %i seconds\n", szLocalAccountInfo.szRealUsername, days, hours, minutes, iTimeLogged);

	return;
}

void SetProfile(char *sex, char *location, char *description) {
	dBuf.add(1);
	dBuf.add(3);
	dBuf.add(szLocalAccountInfo.szUsername);
	dBuf.add("Profile\\Sex");
	dBuf.add("Profile\\Location");
	dBuf.add("Profile\\Description");
	dBuf.add(sex);
	dBuf.add(location);
	dBuf.add(description);
	SendBNCSPacket(sckBNCS, SID_WRITEUSERDATA);
	return;
}

void SetFonts(void) {
	LOGFONT font;
					
	CRegisterWIN32* Registry = new CRegisterWIN32;
	Registry->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Fonts\\Channel list", TRUE);
		font.lfCharSet = Registry->ReadDword("CharSet", 0);
		font.lfClipPrecision = Registry->ReadDword("ClipPrecision", 2);
		font.lfEscapement = Registry->ReadDword("Escapement", 0);
		strcpy(font.lfFaceName, Registry->Read("FaceName", "Verdana"));
		font.lfHeight = Registry->ReadDword("Height", 0xfffffff0);
		font.lfItalic = Registry->ReadDword("Italic", 0);
		font.lfOrientation = Registry->ReadDword("Orientation", 0);
		font.lfOutPrecision = Registry->ReadDword("OutPrecision", 3);
		font.lfPitchAndFamily = Registry->ReadDword("PitchAndFamily", 34);
		font.lfQuality = Registry->ReadDword("Quality", 1);
		font.lfStrikeOut = Registry->ReadDword("StrikeOut", 0);
		font.lfUnderline = Registry->ReadDword("Underline", 0);
		font.lfWeight = Registry->ReadDword("Weight", 400);
		font.lfWidth = Registry->ReadDword("Width", 0);
	Registry->Close();
	hfBotFont = CreateFontIndirect(&font);
	SendMessage(GetDlgItem(hMainDlg, IDC_CHANLIST), WM_SETFONT, (WPARAM)hfBotFont, TRUE);

	Registry->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Fonts\\Channel name", TRUE);
		font.lfCharSet = Registry->ReadDword("CharSet", 0);
		font.lfClipPrecision = Registry->ReadDword("ClipPrecision", 2);
		font.lfEscapement = Registry->ReadDword("Escapement", 0);
		strcpy(font.lfFaceName, Registry->Read("FaceName", "Verdana"));
		font.lfHeight = Registry->ReadDword("Height", 0xfffffff0);
		font.lfItalic = Registry->ReadDword("Italic", 0);
		font.lfOrientation = Registry->ReadDword("Orientation", 0);
		font.lfOutPrecision = Registry->ReadDword("OutPrecision", 3);
		font.lfPitchAndFamily = Registry->ReadDword("PitchAndFamily", 34);
		font.lfQuality = Registry->ReadDword("Quality", 1);
		font.lfStrikeOut = Registry->ReadDword("StrikeOut", 0);
		font.lfUnderline = Registry->ReadDword("Underline", 0);
		font.lfWeight = Registry->ReadDword("Weight", 400);
		font.lfWidth = Registry->ReadDword("Width", 0);
	Registry->Close();
	hfBotFont = CreateFontIndirect(&font);
	SendMessage(GetDlgItem(hMainDlg, IDC_CHANNAME), WM_SETFONT, (WPARAM)hfBotFont, TRUE);

	Registry->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Fonts\\Your text", TRUE);
		font.lfCharSet = Registry->ReadDword("CharSet", 0);
		font.lfClipPrecision = Registry->ReadDword("ClipPrecision", 2);
		font.lfEscapement = Registry->ReadDword("Escapement", 0);
		strcpy(font.lfFaceName, Registry->Read("FaceName", "Verdana"));
		font.lfHeight = Registry->ReadDword("Height", 0xfffffff0);
		font.lfItalic = Registry->ReadDword("Italic", 0);
		font.lfOrientation = Registry->ReadDword("Orientation", 0);
		font.lfOutPrecision = Registry->ReadDword("OutPrecision", 3);
		font.lfPitchAndFamily = Registry->ReadDword("PitchAndFamily", 34);
		font.lfQuality = Registry->ReadDword("Quality", 1);
		font.lfStrikeOut = Registry->ReadDword("StrikeOut", 0);
		font.lfUnderline = Registry->ReadDword("Underline", 0);
		font.lfWeight = Registry->ReadDword("Weight", 400);
		font.lfWidth = Registry->ReadDword("Width", 0);
	Registry->Close();
	hfBotFont = CreateFontIndirect(&font);
	SendMessage(GetDlgItem(hMainDlg, IDC_BNSEND), WM_SETFONT, (WPARAM)hfBotFont, TRUE);

	Registry->Open(HKEY_CURRENT_USER, "Software\\Beta Productions\\SoupBot2\\Fonts\\Channel text", TRUE);
		lfChatFont.lfCharSet = Registry->ReadDword("CharSet", 0);
		lfChatFont.lfClipPrecision = Registry->ReadDword("ClipPrecision", 2);
		lfChatFont.lfEscapement = Registry->ReadDword("Escapement", 0);
		strcpy(lfChatFont.lfFaceName, Registry->Read("FaceName", "Verdana"));
		lfChatFont.lfHeight = Registry->ReadDword("Height", 0xfffffff0);
		lfChatFont.lfItalic = Registry->ReadDword("Italic", 0);
		lfChatFont.lfOrientation = Registry->ReadDword("Orientation", 0);
		lfChatFont.lfOutPrecision = Registry->ReadDword("OutPrecision", 3);
		lfChatFont.lfPitchAndFamily = Registry->ReadDword("PitchAndFamily", 34);
		lfChatFont.lfQuality = Registry->ReadDword("Quality", 1);
		lfChatFont.lfStrikeOut = Registry->ReadDword("StrikeOut", 0);
		lfChatFont.lfUnderline = Registry->ReadDword("Underline", 0);
		lfChatFont.lfWeight = Registry->ReadDword("Weight", 400);
		lfChatFont.lfWidth = Registry->ReadDword("Width", 0);
	Registry->Close();
	hfBotFont = CreateFontIndirect(&lfChatFont);
	SendMessage(GetDlgItem(hMainDlg, IDC_BNCHAT), WM_SETFONT, (WPARAM)hfBotFont, TRUE);
	delete Registry;
}

void HandleCommand(char *command, bool remote) {
	if(remote) {
		Send(sckBNCS, "%s", command);
	}

	if(!remote) {
		if(!strncmp(command, "/rejoin", 7))
		{
			if(szLocalAccountInfo.Connected) {
				SendBNCSPacket(sckBNCS, SID_LEAVECHAT);
				dBuf.add((int)2);
				dBuf.add(szLocalAccountInfo.szCurrentChan);
				SendBNCSPacket(sckBNCS, SID_JOINCHANNEL);
			}
			return;
		}
		if(!strncmp(command, "/sendquote", 10))
		{
			SendRandomQuote2();
			return;
		}
		if(!strncmp(command, "/reply ", 7))
		{
			if(strlen(szLocalAccountInfo.szWhisperTo) > 0) {
				Send(sckBNCS, "/w %s %s", szLocalAccountInfo.szWhisperTo, (command + 7));
				return;
			} else {
				AppendTextTS(hBNChat, RED, "Error: No quick reply-to name set\n");
				return;
			}
			
		}
		if(!strncmp(command, "/profile ", 9))
		{
			if(strlen(command) > 9) {
				GetProfile(command + 9);
				return;
			} else {
				AppendTextTS(hBNChat, RED, "Error: Whose profile do you want to check?\n");
				return;
			}

		}
		Send(sckBNCS, "%s", command);
	}
}

void GetDay(int day, char *out) {
	switch (day) {
	case 0:
		strcpy(out, "Sunday");
		break;
	case 1:
		strcpy(out, "Monday");
		break;
	case 2:
		strcpy(out, "Tuesday");
		break;
	case 3:
		strcpy(out, "Wednesday");
		break;
	case 4:
		strcpy(out, "Thursday");
		break;
	case 5:
		strcpy(out, "Friday");
		break;
	case 6:
		strcpy(out, "Saturday");
		break;
	default:
		strcpy(out, "Unknown day");
		break;
	}
}

void GetMonth(int month, char *out) {
	switch (month) {
	case 1:
		strcpy(out, "January");
		break;
	case 2:
		strcpy(out, "February");
		break;
	case 3:
		strcpy(out, "March");
		break;
	case 4:
		strcpy(out, "April");
		break;
	case 5:
		strcpy(out, "May");
		break;
	case 6:
		strcpy(out, "June");
		break;
	case 7:
		strcpy(out, "July");
		break;
	case 8:
		strcpy(out, "August");
		break;
	case 9:
		strcpy(out, "September");
		break;
	case 10:
		strcpy(out, "October");
		break;
	case 11:
		strcpy(out, "November");
		break;
	case 12:
		strcpy(out, "December");
		break;
	default:
		strcpy(out, "Unknown month");
		break;
	}
}