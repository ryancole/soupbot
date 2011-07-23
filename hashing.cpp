#include "hashing.h"

#define ROL(nr, shift)	((nr << shift) | (nr >> (32 - shift)))

/* Code values used in Diablo 2 CD-Key decoding */

BYTE codevalues[256] = {
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0x00, 0xFF, 0x01, 0xFF, 0x02, 0x03, 0x04, 0x05, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0xFF, 0x0D, 0x0E, 0xFF, 0x0F, 0x10, 0xFF,
0x11, 0xFF, 0x12, 0xFF, 0x13, 0xFF, 0x14, 0x15, 0x16, 0xFF, 0x17, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0xFF, 0x0D, 0x0E, 0xFF, 0x0F, 0x10, 0xFF,
0x11, 0xFF, 0x12, 0xFF, 0x13, 0xFF, 0x14, 0x15, 0x16, 0xFF, 0x17, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

DWORD dwMpqChecksumKeys[] = { 0xE7F4CB62lu, 0xF6A14FFClu, 0xAA5504AFlu, 0x871FCDC2lu, 0x11BF6A18lu, 0xC57292E6lu, 0x7927D27Elu, 0x2FEC8733lu };

bool CheckRevision2(LPCTSTR lpszFileName1, LPCTSTR lpszFileName2, LPCTSTR lpszFileName3, LPCTSTR lpszValueString, DWORD * lpdwVersion, DWORD * lpdwChecksum, LPSTR lpExeInfoString, LPCTSTR lpszMpqFileName) {
   HANDLE hFile, hFileMapping;
   char * s, lpszFileName[256], cOperations[16];
   int nHashFile, nVariable1[16], nVariable2[16], nVariable3[16], nVariable, i, k, nHashOperations;
   DWORD dwTotalSize, dwSize, j, dwBytesRead, dwVariables[4], dwMpqKey, * lpdwBuffer;
   LPSTR lpszFileNames[3];
   FILETIME ft;
   SYSTEMTIME st;
   LPBYTE lpbBuffer;
   VS_FIXEDFILEINFO * ffi;

   s = strchr((char *) lpszMpqFileName, '.');
   if (s == NULL)
      return FALSE;
   nHashFile = (int) (*(s - 1) - '0');
   if (nHashFile > 7 || nHashFile < 0)
      return FALSE;
   dwMpqKey = dwMpqChecksumKeys[nHashFile];
   lpszFileNames[0] = (LPSTR) lpszFileName1;
   lpszFileNames[1] = (LPSTR) lpszFileName2;
   lpszFileNames[2] = (LPSTR) lpszFileName3;
   s = (char *) lpszValueString;
   while (*s != '\0') {
      if (isalpha(*s))
         nVariable = (int) (toupper(*s) - 'A');
      else {
         nHashOperations = (int) (*s - '0');
         s = strchr(s, ' ');
         if (s == NULL)
            return FALSE;
         s++;
         break;
      }
      if (*(++s) == '=')
         s++;
      dwVariables[nVariable] = atol(s);
      s = strchr(s, ' ');
      if (s == NULL)
         return FALSE;
      s++;
   }
   for (i = 0; i < nHashOperations; i++) {
      if (!isalpha(*s))
         return FALSE;
      nVariable1[i] = (int) (toupper(*s) - 'A');
      if (*(++s) == '=')
         s++;
      if (toupper(*s) == 'S')
         nVariable2[i] = 3;
      else
         nVariable2[i] = (int) (toupper(*s) - 'A');
      cOperations[i] = *(++s);
      s++;
      if (toupper(*s) == 'S')
         nVariable3[i] = 3;
      else
         nVariable3[i] = (int) (toupper(*s) - 'A');
      s = strchr(s, ' ');
      if (s == NULL)
         break;
      s++;
   }
   dwVariables[0] ^= dwMpqKey;
   for (i = 0; i < 3; i++) {
      if (lpszFileNames[i][0] == '\0')
         continue;
      hFile = CreateFile(lpszFileNames[i], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (hFile == (HANDLE) INVALID_HANDLE_VALUE)
         return FALSE;
      hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
      if (hFileMapping == NULL) {
         CloseHandle(hFile);
         return FALSE;
      }
      lpdwBuffer = (LPDWORD) MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
      if (lpdwBuffer == NULL) {
         CloseHandle(hFileMapping);
         CloseHandle(hFile);
         return FALSE;
      }
      if (i == 0) {
         GetFileTime(hFile, &ft, NULL, NULL);
         FileTimeToSystemTime(&ft, &st);
         dwTotalSize = GetFileSize(hFile, NULL);
      }
      dwSize = (GetFileSize(hFile, NULL) / 1024lu) * 1024lu;
      for (j = 0; j < (dwSize / 4lu); j++) {
         dwVariables[3] = lpdwBuffer[j];
         for (k = 0; k < nHashOperations; k++) {
            switch (cOperations[k]) {
               case '+':
                  dwVariables[nVariable1[k]] = dwVariables[nVariable2[k]] + dwVariables[nVariable3[k]];
                  break;
               case '-':
                  dwVariables[nVariable1[k]] = dwVariables[nVariable2[k]] - dwVariables[nVariable3[k]];
                  break;
               case '^':
                  dwVariables[nVariable1[k]] = dwVariables[nVariable2[k]] ^ dwVariables[nVariable3[k]];
                  break;
               default:
                  return FALSE;
            }
         }
      }
      UnmapViewOfFile(lpdwBuffer);
      CloseHandle(hFileMapping);
      CloseHandle(hFile);
   }
   strcpy(lpszFileName, lpszFileName1);
   dwSize = GetFileVersionInfoSize(lpszFileName, &dwBytesRead);
   lpbBuffer = (LPBYTE) VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
   if (lpbBuffer == NULL)
      return FALSE;
   if (GetFileVersionInfo(lpszFileName, NULL, dwSize, lpbBuffer) == FALSE)
      return FALSE;
   if (VerQueryValue(lpbBuffer, "\\", (LPVOID *) &ffi, (PUINT) &dwSize) == FALSE)
      return FALSE;
   *lpdwVersion = ((HIWORD(ffi->dwProductVersionMS) & 0xFF) << 24) | ((LOWORD(ffi->dwProductVersionMS) & 0xFF) << 16) | ((HIWORD(ffi->dwProductVersionLS) & 0xFF) << 8) | (LOWORD(ffi->dwProductVersionLS) & 0xFF);
   VirtualFree(lpbBuffer, 0lu, MEM_RELEASE);
   s = (char *) &lpszFileName[strlen(lpszFileName)-1];
   while (*s != '\\' && s > (char *) lpszFileName)
      s--;
   s++;
   sprintf(lpExeInfoString, "%s %02u/%02u/%02u %02u:%02u:%02u %lu", s, st.wMonth, st.wDay, st.wYear % 100, st.wHour, st.wMinute, st.wSecond, dwTotalSize);
   *lpdwChecksum = dwVariables[2];
   return TRUE;
}

bool RealmHash(char *OutBuf, DWORD encryptvalue, DWORD*val1, DWORD*val2, DWORD*val3, DWORD*val4, DWORD*val5, int subtract) {
	DWORD tohash[7];
	DWORD myhash[5];
	tohash[0] = 1;
	tohash[1] = encryptvalue;
	HashData("password", 8, tohash+2); 
	HashData(tohash, sizeof(tohash), myhash); 
	*val1=myhash[0];
	*val2=myhash[1];
	*val3=myhash[2];
	*val4=myhash[3];
	*val5=myhash[4];
	return TRUE;
}

bool HashPass(char *password, int length, DWORD*val1, DWORD*val2, DWORD*val3, DWORD*val4, DWORD*val5, int nothing) {
	DWORD passwordhash[7];
	passwordhash[0] = *val1;
	passwordhash[1] = *val2;
	HashData(password, length, passwordhash+2);
	HashData(passwordhash, 7*4, passwordhash+2);
	*val1=passwordhash[2];
	*val2=passwordhash[3];
	*val3=passwordhash[4];
	*val4=passwordhash[5];
	*val5=passwordhash[6];
	return TRUE;
}

bool CreateAccount(char *OutBuf, char *password, int nothing1, int nothing2, int nothing3, int nothing4, int nothing5, int nothing6) {
	DWORD dwHashBuffer[5];
	HashData(password, strlen(password), dwHashBuffer);
	memcpy(OutBuf, dwHashBuffer, 5*4);
	return TRUE;
}

bool HashCDKey2(char *OutBuf, DWORD sessionkey, DWORD prodid, DWORD val1, DWORD val2, DWORD seed, int nothing1, int nothing2) {
	DWORD dwHashBuff[6];
	DWORD dwHashResult[5];
		dwHashBuff[0] = seed;
		dwHashBuff[1] = sessionkey-1234;
		dwHashBuff[2] = prodid-1234;
		dwHashBuff[3] = val1;
		dwHashBuff[4] = 0;
		dwHashBuff[5] = val2;
		HashData(dwHashBuff, 24, dwHashResult); 
		memcpy(OutBuf, dwHashResult, 5*4);
		return TRUE;
}

bool HashCDKey(DWORD*key, DWORD*seed, DWORD*prodid, DWORD*val1, DWORD*val2, int nothing1, int nothing2, int nothing3) {
	DWORD dwHashBuff[5];
		dwHashBuff[0] = *key;
		dwHashBuff[1] = *seed;
		dwHashBuff[2] = *prodid;
		dwHashBuff[3] = *val1;
		dwHashBuff[4] = *val2;
		HashData(dwHashBuff, 20, dwHashBuff); 

	*key = dwHashBuff[0];
	*seed = dwHashBuff[1];
	*prodid = dwHashBuff[2];
	*val1 = dwHashBuff[3];
	*val2 = dwHashBuff[4];
		return TRUE;
}

void getvalues(const char *databuf, DWORD *ping, DWORD *flags, char *name, char *txt)
{
	DWORD a, b, c, d, e, f, recvbufpos = 4;
	f = *(unsigned long *)(databuf + recvbufpos);
	recvbufpos+=4;
	a = *(unsigned long *)(databuf + recvbufpos);
	recvbufpos+=4;
	b = *(unsigned long *)(databuf + recvbufpos);
	recvbufpos+=4;
	c = *(unsigned long *)(databuf + recvbufpos);
	recvbufpos+=4;
	d = *(unsigned long *)(databuf + recvbufpos);
	recvbufpos+=4;
	e = *(unsigned long *)(databuf + recvbufpos);
	recvbufpos+=4;
	*flags = a;
	*ping = b;
	strcpy(name, databuf + 28);
	strcpy(txt, databuf + (strlen(name) + 29));
}

bool DecodeCDKey(LPCTSTR lpszCDKey, unsigned long * lpdwProductId, unsigned long * lpdwValue1, unsigned long * lpdwValue2) {
   char key[1024], value[1024];
   int i, length, keylength;
   bool bValid;

   length = strlen(lpszCDKey);
   keylength = 0;
   for (i = 0; i < length; i++) {
      if (isalnum(lpszCDKey[i])) {
         key[keylength] = lpszCDKey[i];
         keylength++;
      }
   }
   if (keylength == 13)
      bValid = DecodeStarcraftKey(key);
   else if (keylength == 16)
      bValid = DecodeD2Key(key);
   else
      return false;
   strncpy(value, key, 2);
   value[2] = '\0';
   sscanf(value, "%X", lpdwProductId);
   if (keylength == 16) {
      strncpy(value, &key[2], 6);
      value[6] = '\0';
      sscanf(value, "%X", lpdwValue1);
      strcpy(value, &key[8]);
      value[8] = '\0';
      sscanf(value, "%X", lpdwValue2);
   }
   else if (keylength == 13) {
      strncpy(value, &key[2], 7);
      value[7] = '\0';
      sscanf(value, "%ld", lpdwValue1);
      strncpy(value, &key[9], 3);
      value[3] = '\0';
      sscanf(value, "%ld", lpdwValue2);
   }    
   return bValid;
}

bool DecodeStarcraftKey(LPSTR key) {
   DWORD n, n2, v, v2; //r keyvalue
   BYTE c2, c; //c1
   bool bValid;
   int i;

   v = 3;
   for (i = 0; i < 12; i++) {
      c = key[i];
      n = Get_Num_Value(c);
      n2 = v * 2;
      n ^= n2;
      v += n;
   }
   v %= 10;
   if (Get_Hex_Value(v) == key[12])
      bValid = true;
   else
      bValid = false;
   v = 194;
   for (i = 11; v >= 7, i >= 0; i--) {
      c = key[i];
      n = v / 12;
      n2 = v % 12;
      v -= 17;
      c2 = key[n2];
      key[i] = c2;
      key[n2] = c;
   }
   v2 = 0x13AC9741;
   for (i = 11; i >= 0; i--) {
      c = toupper(key[i]);
      key[i] = c;
      if (c <= '7') {
         v = v2;
         c2 = (unsigned char)v & 0xFF;
         c2 &= 7;
         c2 ^= c;
         v >>= 3;
         key[i] = c2;
         v2 = v;
      }
      else if (c < 'A') {
         c2 = (BYTE) i;
         c2 &= 1;
         c2 ^= c;
         key[i] = c2;
      }
   }
   return bValid;
}

bool DecodeD2Key(LPSTR key) {
   DWORD r, n, n2, v, v2, keyvalue;
   BYTE c1, c2, c;
   bool bValid;
   int i;   

   r = 1;
   keyvalue = 0;
   for (i = 0; i < 16; i += 2) {
      c1 = codevalues[key[i]];
      n = c1 * 3;
      c2 = codevalues[key[i+1]];
      n = c2 + n * 8;
      if (n >= 0x100) {
         n -= 0x100;
         keyvalue |= r;
      }
      n2 = n;
      n2 >>= 4;
      key[i] = Get_Hex_Value(n2);
      key[i+1] = Get_Hex_Value(n);
      r <<= 1;
   }
   v = 3;
   for (i = 0; i < 16; i++) {
      c = key[i];
      n = Get_Num_Value(c);
      n2 = v * 2;
      n ^= n2;
      v += n;
   }
   v &= 0xFF;
   if (v == keyvalue)
      bValid = TRUE;
   else
      bValid = FALSE;
   for (i = 15; i >= 0; i--) {
      c = key[i];
      if (i > 8)
         n = i - 9;
      else
         n = 0xF - (8 - i);
      n &= 0xF;
      c2 = key[n];
      key[i] = c2;
      key[n] = c;
   }
   v2 = 0x13AC9741;
   for (i = 15; i >= 0; i--) {
      c = toupper(key[i]);
      key[i] = c;
      if (c <= '7') {
         v = v2;
         c2 = (char)v & 0xFF;
         c2 &= 7;
         c2 ^= c;
         v >>= 3;
         key[i] = c2;
         v2 = v;
      }
      else if (c < 'A') {
         c2 = (BYTE) i;
         c2 &= 1;
         c2 ^= c;
         key[i] = c2;
      }
   }
   return bValid;
}

char Get_Hex_Value(unsigned long v)
{
   v &= 0xF;
   if (v < 10)
      return (v + 0x30);
   else
      return (v + 0x37);
}

int Get_Num_Value(char c)
{
   c = toupper(c);
   if (isdigit(c))
      return (c - 0x30);
   else
      return (c - 0x37);
}

void HashData(void* lpSource, int nLength, void* lpResult)
{
   BYTE bBuffer[1024];
   int i;
   DWORD a, b, c, d, e, g, * lpdwBuffer;

   ZeroMemory(bBuffer, 1024);
   CopyMemory(bBuffer, lpSource, nLength);
   lpdwBuffer = (LPDWORD) bBuffer;

   for (i=0; i<64; i++)
      lpdwBuffer[i+16] = ROL(1, (lpdwBuffer[i] ^ lpdwBuffer[i+8] ^
                             lpdwBuffer[i+2] ^ lpdwBuffer[i+13]) % 32);
   a = 0x67452301lu;
   b = 0xefcdab89lu;
   c = 0x98badcfelu;
   d = 0x10325476lu;
   e = 0xc3d2e1f0lu;
   for (i = 0; i < (20 * 1); i++)
   {
      g = lpdwBuffer[i] + ROL(a,5) + e + ((b & c) | (~b & d)) + 0x5a827999lu;
      e = d;
      d = c;
      c = ROL(b,30);
      b = a;
      a = g;
   }
   for (; i < (20 * 2); i++)
   {
      g = (d ^ c ^ b) + e + ROL(g,5) + lpdwBuffer[i] + 0x6ed9eba1lu;
      e = d;
      d = c;
      c = ROL(b,30);
      b = a;
      a = g;
   }
   for (; i < (20 * 3); i++)
   {
      g = lpdwBuffer[i] + ROL(g,5) + e + ((c & b) | (d & c) | (d & b)) -
          0x70e44324lu;
      e = d;
      d = c;
      c = ROL(b,30);
      b = a;
      a = g;
   }
   for (; i < (20 * 4); i++)
   {
      g = (d ^ c ^ b) + e + ROL(g,5) + lpdwBuffer[i] - 0x359d3e2alu;
      e = d;
      d = c;
      c = ROL(b,30);
      b = a;
      a = g;
   }

   lpdwBuffer = (LPDWORD) lpResult;
   lpdwBuffer[0] = 0x67452301lu + g;
   lpdwBuffer[1] = 0xefcdab89lu + b;
   lpdwBuffer[2] = 0x98badcfelu + c;
   lpdwBuffer[3] = 0x10325476lu + d;
   lpdwBuffer[4] = 0xc3d2e1f0lu + e;
   return;
}

void ParseStatstring(char *statstring, char *outbuf)
{
	DWORD a, b, c, d, e, f, g, h, i, j;
	strcpy(outbuf, statstring);
	//Starcraft Shareware
	if(!strnicmp(statstring, "RHSS", 4))
	{
		if(sscanf(statstring + 5, "%d %d %d %d %d %d %d %d %d", &a, &b, &c, &d, &e, &f, &g, &h, &i, &j) != 9)
		{
			strcpy(outbuf, "StarCraft shareware bot");
			return;
		}
		if(a > 0)
			sprintf(outbuf, "StarCraft shareware (Wins: %d, Ladder Rating: %d)", c, a);
		else
			sprintf(outbuf, "StarCraft shareware (Wins: %d)", c);
	}

	//Starcraft Japanese
	if(!strnicmp(statstring, "RTSJ", 4))
	{
		if(sscanf(statstring + 5, "%d %d %d %d %d %d %d %d %d", &a, &b, &c, &d, &e, &f, &g, &h, &i, &j) != 9)
		{
			sprintf(outbuf, "StarCraft japanese %sbot.", (d == 1) ? "Spawn " : "");
			return;
		}
		if(a > 0)
			sprintf(outbuf, "StarCraft japanese %s(Wins: %d, Ladder Rating: %d)", (d == 1) ? "Spawn " : "", c, a);
		else
			sprintf(outbuf, "StarCraft japanese %s(Wins: %d)", (d == 1) ? "Spawn " : "", c);
	}

	//Starcraft
	if(!strnicmp(statstring, "RATS", 4))
	{
		if(sscanf(statstring + 5, "%d %d %d %d %d %d %d %d %d", &a, &b, &c, &d, &e, &f, &g, &h, &i, &j) != 9)
		{
			sprintf(outbuf, "StarCraft %sbot.", (d == 1) ? "Spawn " : "");
			return;
		}
		if(a > 0)
			sprintf(outbuf, "StarCraft %s(Wins: %d, Ladder Rating: %d)", (d == 1) ? "Spawn " : "", c, a);
		else
			sprintf(outbuf, "StarCraft %s(Wins: %d)", (d == 1) ? "Spawn " : "", c);
	}

	//Brood War
	if(!strnicmp(statstring, "PXES", 4))
	{
		if(sscanf(statstring + 5, "%d %d %d %d %d %d %d %d %d", &a, &b, &c, &d, &e, &f, &g, &h, &i, &j) != 9)
		{
			sprintf(outbuf, "StarCraft: Brood War %sbot.", (d == 1) ? "Spawn " : "");
			return;
		}
		if(a > 0)
			sprintf(outbuf, "StarCraft: Brood War %s(Wins: %d, Ladder Rating: %d)", (d == 1) ? "Spawn " : "", c, a);
		else
			sprintf(outbuf, "StarCraft: Brood War %s(Wins: %d)", (d == 1) ? "Spawn " : "", c);
	}

	//Warcraft II
	if(!strnicmp(statstring, "NB2W", 4))
	{
		if(sscanf(statstring + 5, "%d %d %d %d %d %d %d %d %d", &a, &b, &c, &d, &e, &f, &g, &h, &i, &j) != 9)
		{
			sprintf(outbuf, "WarCraft II %sbot.", (d == 1) ? "Spawn " : "");
			return;
		}
		if(a > 0)
			sprintf(outbuf, "WarCraft II %s(Wins: %d, Ladder Rating: %d)", (d == 1) ? "Spawn " : "", c, a);
		else
			sprintf(outbuf, "WarCraft II %s(Wins: %d)", (d == 1) ? "Spawn " : "", c);
	}

	//Diablo Shareware
	if(!strnicmp(statstring, "RHSD", 4))
	{
		if(sscanf(statstring + 5, "%d %d %d %d %d %d %d %d %d", &a, &b, &c, &d, &e, &f, &g, &h, &i, &j) != 9)
		{
			strcpy(outbuf, "Diablo shareware bot");
			return;
		}
		char type[32] = "";
		if(c == 0) strcpy(type, "Warrior");
		if(c == 1) strcpy(type, "Sorceror");
		if(c == 2) strcpy(type, "Rogue");
		sprintf(outbuf, "Diablo shareware (Level %d %s, Dots: %d, Strength: %d, Magic: %d, Dexterity: %d, Vitality: %d, Gold: %d)", a, type, b, d, e, f, g, h);
	}

	//Diablo
	if(!strnicmp(statstring, "LTRD", 4))
	{
		if(sscanf(statstring + 5, "%d %d %d %d %d %d %d %d %d", &a, &b, &c, &d, &e, &f, &g, &h, &i, &j) != 9)
		{
			strcpy(outbuf, "Diablo bot");
			return;
		}
		char type[32] = "";
		if(c == 0) strcpy(type, "Warrior");
		if(c == 1) strcpy(type, "Sorceror");
		if(c == 2) strcpy(type, "Rogue");
		sprintf(outbuf, "Diablo (Level %d %s, Dots: %d, Strength: %d, Magic: %d, Dexterity: %d, Vitality: %d, Gold: %d)", a, type, b, d, e, f, g, h);
	}

	//Diablo II
	if(!strnicmp(statstring + 2, "2D", 2))
		strcpy(outbuf, ParseD2Stats(statstring));

	//Chat Client
	if(!strnicmp(statstring, "TAHC", 4))
		strcpy(outbuf, "Chat client");
	
	//Warcraft III
	if(!strnicmp(statstring, "3RAW", 4))
		strcpy(outbuf, ParseW3Stats(statstring));
	
	if(!strnicmp(statstring, "PX3W", 4))
		strcpy(outbuf, ParseW3Stats(statstring));
}

char *ParseD2Stats(char *stats)
{
	char *d2classes[] = 
	{
		"Amazon",
		"Sorceress",
		"Necromancer",
		"Paladin",
		"Barbarian",
		"Druid",
		"Assassin",
		"Unknown Class Type"
	};
	char *statbuf = new char[512];
	memset(statbuf, 0, 512);
	char *p = 0;
	char server[32];
	char name[32];
	if(strlen(stats) > 4) {
		int len = StringCopy(server, stats + 4, ',');
		len += StringCopy(name, stats + len + 5, ',');
		p = stats + len + 6;
	}
	if(!strnicmp(stats, "VD2D", 4))
		strcat(statbuf, "Diablo II (");
	else
		strcat(statbuf, "Diablo II: Lord of Destruction (");
	if(!p || strlen(p) != 33) {
		strcat(statbuf, "Open character)");
	} else {
		char version = p[0] - 0x80;
		char charclass = p[13] - 1;
		if(charclass < 0 || charclass > 6)
			charclass = 7;
		bool female = false;
		if(charclass == 0 || charclass == 1 || charclass == 6)
			female = true;
		int charlevel = p[25];
		char hardcore = p[26] & 4;
		bool expansion = false;
		if(!strnicmp(stats, "PX2D", 4))
		{
			if(p[26] & 0x20)
			{
				switch((p[27] & 0x18) >> 3)	
				{
				case 1:
					if(hardcore)
						mystrcat(statbuf, "Destroyer ");
					else
						mystrcat(statbuf, "Slayer ");
					break;
				case 2:
					if(hardcore)
						mystrcat(statbuf, "Conquerer ");
					else
						mystrcat(statbuf, "Champion ");
					break;
				case 3:
					if(hardcore)
						mystrcat(statbuf, "Guardian ");
					else
					{
						if(!female)
							mystrcat(statbuf, "Patriarch ");
						else
							mystrcat(statbuf, "Matriarch ");
					}
					break;
				}
				expansion = true;
			}
		}
		if(!expansion){
		switch((p[27] & 0x18) >> 3) {
			case 1:
				if(female == false) {
					if(hardcore)
						mystrcat(statbuf, "Count ");
					else
						mystrcat(statbuf, "Sir ");
				} else {
					if(hardcore)
						mystrcat(statbuf, "Countess ");
					else
						mystrcat(statbuf, "Dame ");
				}
				break;
			case 2:
				if(female == false) {
					if(hardcore)
						mystrcat(statbuf, "Duke ");
					else
						mystrcat(statbuf, "Lord ");
				} else {
					if(hardcore)
						mystrcat(statbuf, "Duchess ");
					else
						mystrcat(statbuf, "Lady ");
				}
				break;
			case 3:
				if(female == false) {
					if(hardcore)
						mystrcat(statbuf, "King ");
					else
						mystrcat(statbuf, "Baron ");
				} else {
					if(hardcore)
						mystrcat(statbuf, "Queen ");
					else
						mystrcat(statbuf, "Baroness ");
				}
				break;
		}}
		mystrcat(statbuf, "%s@%s", (strlen(name)==0 ? "<Invalid Character>" : name), server);
		if(hardcore) {
			if(p[26] & 0x08)
				strcat(statbuf, ", a dead");
			else
				strcat(statbuf, ", a");
			mystrcat(statbuf, " hardcore level %d", charlevel);
		} else {
			mystrcat(statbuf, ", level %d", charlevel);
		}
		mystrcat(statbuf, " %s)", d2classes[charclass]);
	}
	return statbuf;
}

char *ParseW3Stats(char *stats)
{
	char IconRace, IconTier;
	char LevelOne = '0';
	char LevelTwo = '0';
	char Clan[6];
	char *statbuf = new char[512];
	long StatsLen = strlen(stats);
	memset(statbuf, 0, 512);
	
	if(!strnicmp(stats, "3RAW", 4))
		strcat(statbuf, "WarCraft III: Reign of Chaos (");
	else
		strcat(statbuf, "WarCraft III: The Frozen Throne (");

	IconTier = stats[5];
	IconRace = stats[6];

	mystrcat(statbuf, "tier-%c ", IconTier);

	switch(IconRace) {
	case 'R':
		{
			strcat(statbuf, "random icon, ");
			break;
		}
	case 'N':
		{
			strcat(statbuf, "night elf icon, ");
			break;
		}
	case 'O':
		{
			strcat(statbuf, "orc icon, ");
			break;
		}
	case 'U':
		{
			strcat(statbuf, "undead icon, ");
			break;
		}
	case 'H':
		{
			strcat(statbuf, "human icon, ");
			break;
		}
	case 'D':
		{
			strcat(statbuf, "tournament icon, ");
			break;
		}
	default:
		{
			strcat(statbuf, "unknown icon, ");
		}
	}

	switch(StatsLen) {
	case 11:
		{
			LevelOne = stats[10];
			break;
		}
	case 12:
		{
			LevelOne = stats[10];
			LevelTwo = stats[11];
			break;
		}
	}

	mystrcat(statbuf, "level %c%c", LevelOne, LevelTwo);	

	if(strlen(stats) > 12) {
		strcpy(Clan, stats+12);
		mystrcat(statbuf, " in clan %s", Clan);
	}

	strcat(statbuf, ")");
	return statbuf;
}

int StringCopy(char *dest, char *source, int stopchar)
{
	int lencopied = 0;
	while(*source && *source != stopchar) {
		*dest = *source;
		dest++;
		source++;
		lencopied++;
	}
	if(lencopied)
		*dest = 0;
	return lencopied;
}

char *mystrcat(char *dest, char *src, ...)
{
	va_list argptr;
	va_start(argptr, src);
	vsprintf(dest + strlen(dest), src, argptr);
	va_end(argptr);
	return dest;
}

int GetIconCode(char *prog, int flags)
{
	int iconcode;
	if(!strncmp(prog, "TAHC", 4)) iconcode = CHAT;
	if(!strncmp(prog, "LTRD", 4)) iconcode = DRTL;
	if(!strncmp(prog, "RATS", 4)) iconcode = SC;
	if(!strncmp(prog, "RTSJ", 4)) iconcode = SCJ;
	if(!strncmp(prog, "RHSD", 4)) iconcode = DSHR;
	if(!strncmp(prog, "RHSS", 4)) iconcode = SSHR;
	if(!strncmp(prog, "PXES", 4)) iconcode = BW;
	if(!strncmp(prog, "VD2D", 4)) iconcode = D2;
	if(!strncmp(prog, "PX2D", 4)) iconcode = D2X;
	if(!strncmp(prog, "NB2W", 4)) iconcode = WAR2;
	if(!strncmp(prog, "3RAW", 4)) iconcode = WAR3;
	if(flags & 0x01) iconcode = BLIZZ;
	if(flags & 0x02) iconcode = OP;
	if(flags & 0x04) iconcode = BLIZZ;
	if(flags & 0x08) iconcode = BLIZZ;
	if(flags & 0x20) iconcode = BRX;
	if(flags & 0x40) iconcode = BLIZZ;
	return iconcode;
}