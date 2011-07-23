#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

#include <winsock2.h>
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <richedit.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "resource.h"
#include "CRegister.h"
#include "DynBuffer.h"
#include "List.h"
#include "Socket.h"

#define DATA_PENDING_TCP	(WM_USER + 1)
#define WM_SHELLNOTIFY		(WM_USER + 5000)
#define PACKET_HEAD			4
#define IDI_TRAY			42

#define BLACK      0x000000
#define WHITE      0xffffff
#define RED        0x4b4bff
#define GREEN      0x32ff32
#define LGREEN	   0x96ff32
#define TEAL	   0xdde64e
#define BLUE       0xff0000
#define LIGHTBLUE  0xff8787
#define GRAY       0xb4b4b4
#define YELLOW     0x4bffff

#define SID_PING					0x25
#define SID_AUTH_INFO				0x50
#define SID_AUTH_CHECK				0x51
#define SID_LOGONRESPONSE2			0x3A
#define SID_UDPPINGRESPONSE			0x14
#define SID_MESSAGEBOX				0x19
#define SID_GETICONDATA				0x2D
#define SID_ENTERCHAT				0x0A
#define SID_OPTIONALWORK			0x4A
#define SID_GETCHANNELLIST			0x0B
#define SID_JOINCHANNEL				0x0C
#define SID_CHATEVENT				0x0F
#define SID_FLOODDETECTED			0x13
#define SID_AUTH_ACCOUNTLOGON		0x53
#define SID_AUTH_ACCOUNTLOGONPROOF	0x54
#define SID_AUTH_ACCOUNTCREATE		0x52
#define SID_CREATEACCOUNT2			0x3D
#define SID_SETEMAIL				0x59
#define SID_CHATCOMMAND				0x0E
#define SID_LEAVECHAT				0x10
#define SID_READUSERDATA			0x26
#define SID_WRITEUSERDATA			0x27
#define SID_LEAVECHAT				0x10
#define SID_FRIENDSUPDATE			0x66
#define SID_FRIENDSADD				0x67
#define SID_FRIENDSREMOVE			0x68
#define SID_FRIENDSPOSITION			0x69

#define PROFILE_PROFILE				1
#define PROFILE_RECORDDATA			2

#define ICO_BLIZZ		0
#define ICO_SPEAKER		1
#define ICO_OP			2
#define ICO_BRX			3
#define ICO_CHAT		4
#define ICO_DRTL		5
#define ICO_DSHR		6
#define ICO_D2			7
#define ICO_SC			8
#define ICO_SSHR		9
#define ICO_BW			10
#define ICO_WAR2		11
#define ICO_D2X			12
#define ICO_SPAWN		13
#define ICO_BNET		14
#define ICO_SCJ			15
#define ICO_JSPAWN		16
#define ICO_WAR3		17
#define ICO_INVIS		18
#define ICO_G1			19
#define ICO_G2			20
#define ICO_Y3			21
#define ICO_Y4			22
#define ICO_R5			23
#define ICO_R6			24
#define ICO_PLUG		25
#define ICO_W3XP		26

#define GAME_VD2D	0
#define GAME_PX2D	1
#define GAME_PXES	2
#define GAME_RATS	3
#define GAME_NB2W	4
#define GAME_3RAW	5
#define GAME_PX3W	6

#define VER_RATS	0xCD
#define VER_PXES	0xCD
#define VER_NB2W	0x4F
#define VER_VD2D	0xB
#define VER_PX2D	0xB
#define VER_3RAW	0x12
#define VER_PX3W	0x12

#define CDKEY_EROR	0
#define CDKEY_STAR	1
#define CDKEY_W2BN	2
#define CDKEY_D2DV	3
#define CDKEY_D2XP	4
#define CDKEY_WAR3	5
#define CDKEY_W3XP	6

List<char *> Queue;

void SaveProfile(char Username[32], char Password[256], char Home[128], char Server[256], char ProfileName[128], int GameIndex, char CDkey[32], char ExpCDkey[32], char EmailAddy[128], bool Spawn);
void LoadProfiles();
void SetLastProfile(int LastProfile);
void DeleteProfile(char Profile[32]);
void RenameProfile(char Profile[32], char OldProfile[32]);
void GameIndexToName(char* gamename, int Index, bool FourLetterAbbr);
int ProfileExists(char ProfileName[128]);
int GameNameToIndex(char GameName[32]);
int FindCheckedRadioButton();
int CDkeyExists(char CDkey[128], char Product[6]);
void DeleteCDkey(int CDkeyNumber, char Product[6]);
void WriteCDkeyList(HWND hWnd, int DlgItemID);
void WriteExpCDkeyList(HWND hWnd, int DlgItemID);
void GameNameToAbbrv(char gamename[32], char* abbrv);
void GetFilePath(char *filepath, char game[12], char filetype[12]);

int MessageBoxFmt(HWND hWnd, int iType, const char *lpszCaption, const char *lpszText, ...);
char *NewString(int iSize);
void CharFormat(COLORREF Color, CHARFORMAT &cfFormat);
void __cdecl AppendText(HWND hRichEdit, COLORREF Color, char *szFmt, ...);

long GetVersionByte(void);
void InitImageList(int iImgList);
int GetItemIndex(int ListView, char *item);
void ModifyItem(int ListView, char *item, int icon);
void InsertItem(char *szItem, int iIcon);

void AddQuote(char *quote);

int IdleTicker;

NOTIFYICONDATA note;
void TrayHandler_Remove(void);
void TrayHandler_AddAndMinimize(void);
void TrayHandler_Restore(void);

struct LocalAccountInfo
{
	char szProfileName[32];
	char szUsername[32];
	char szRealUsername[32];
	char szPassword[256];
	char szHomeChannel[128];
	char szServer[256];
	char szCDkey[32];
	char szExpCDkey[32];
	char szGameAbbr[12];
	char szEmailAddy[128];
	int iGameIndex;
	long lVerByte;
	bool Spawn, Connected, UseNLS, Expansion;
	char EXE[260], DLL1[260], DLL2[260];
	int iGameAbbr(void);
	char szCurrentChan[128];
	bool IdleOn, IdleSpec, IdleQuotes;
	int IdleTimer;
	char IdleMessage[256];
	char StartupMsg[128];
	bool StartedUp, UseStartmsg;
	char szWhisperTo[128];
};

struct ProfileStruct
{
	char szAccount[32];
	char szAge[512];
	char szSex[512];
	char szLocation[512];
	char szDescription[512];
};

HFONT hfBotFont;
LOGFONT lfBotFont, lfChatFont;

unsigned long ServerToken, ClientToken;
HWND hListView;

LocalAccountInfo szLocalAccountInfo;
ProfileStruct szProfileData;

HWND hMainDlg, hBNChat;
HINSTANCE hDlgInst;

// winsock
DynBuffer dBuf;
SOCKET sckBNCS;

// BATTLE.NET FUNCTIONS

void SendBNCSPacket(SOCKET sckBNCS, char bId);
void ParseBNCS(SOCKET sckBNCS, char *lpszBuffer, int iLen);
void SendAuthInfo(SOCKET sckBNCS);
void HandleAuthInfo(SOCKET sckBNCS, const char *ChecksumFormula, char *lpszMPQName, char *ServerSignature, unsigned long LogonType);
void HandleAuthCheck(unsigned long Result);
void SendLogonResponse(void);
void HandleLogonResponse(unsigned long Result);
void SendUDPPingResponse(void);
void SendGetIconData(void);
void SendEnterChat(void);
void HandleCreateAccount(unsigned long Result);
void SendCreateAccount(void);
void SendSetEmail(void);
void SendAuthLogon(void);
void HandleAuthLogon(char *data);
void HandleAuthLogonProof(char *data);
void FreeNLS(void);
void SendAuthCreate(void);
void HandleAuthCreate(char *data);
void Send(SOCKET sckBNCS, char *lpszFmt, ...);
void DoSendText(void);
void SendRandomQuote(void);
void SendRandomQuote2(void);
int GetIconCode(unsigned long product, int flags);
void GetProfile(char *Account);
void GetRecordData(char *Account);
void HandleReadUserData(char *data);
void HandleRecordData(char *data);
void SetProfile(char *sex, char *location, char *description);
void SendLeaveChat(void);

void SetFonts(void);
void HandleCommand(char *command, bool remote);
void GetMonth(int month, char *out);
void GetDay(int day, char *out);

#endif