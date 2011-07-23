#ifndef DYNBUFFER_H_INCLUDED
#define DYNBUFFER_H_INCLUDED
#pragma once

class DynBuffer {
private:
	unsigned long dwLen;
	unsigned long dwPos;
	char *lpszBuffer;
public:
	DynBuffer() { lpszBuffer = 0; dwLen = 0; dwPos = 0; }
	~DynBuffer() { lpszBuffer = 0; if(lpszBuffer) delete [] lpszBuffer; }
	void add(const char *lpszData);
	void add(const void *lpData, unsigned long dwLength);
	void add(unsigned long dwData);
	void add(int iData) { add((unsigned long)iData); }
	void add(unsigned short uData);
	void add(char bData);
	void get(void *lpDest, unsigned long dwLength);
	unsigned long length() { return dwLen; }
	void clear();

	operator char *(void) { return lpszBuffer + dwPos; }
	operator unsigned long(void) { return *(unsigned long *)(lpszBuffer + dwPos); }
	operator unsigned short(void) { return *(unsigned short *)(lpszBuffer + dwPos); }
	operator char(void) { return lpszBuffer[dwPos]; }

	operator ++(int) { dwPos++; if(dwPos > dwLen) dwPos = dwLen; }
	operator --(int) { dwPos--; if(dwPos < 0) dwPos = 0; }
	operator +=(int size) { dwPos += size; if(dwPos > dwLen) dwPos = dwLen; }
	operator -=(int size) { dwPos -= size; if(dwPos < 0) dwPos = 0; }
};

#endif