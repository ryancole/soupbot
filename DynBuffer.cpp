#include "DynBuffer.h"
#include <string.h>

void DynBuffer::add(const char *lpszData)
{
	int iNewLen = dwLen + strlen(lpszData) + 1;
	char *lpszTemporaryBuffer = new char[dwLen];
	memcpy(lpszTemporaryBuffer, lpszBuffer, dwLen);
	lpszBuffer = new char[iNewLen];
	memcpy(lpszBuffer, lpszTemporaryBuffer, dwLen);
	strcpy(lpszBuffer + dwLen, lpszData);
	dwLen += strlen(lpszData) + 1;
	lpszTemporaryBuffer = 0;
	delete [] lpszTemporaryBuffer;
}

void DynBuffer::add(const void *lpData, unsigned long dwLength)
{
	int iNewLen = dwLen + dwLength;
	char *lpszTemporaryBuffer = new char[dwLen];
	memcpy(lpszTemporaryBuffer, lpszBuffer, dwLen);
	lpszBuffer = new char[iNewLen];
	memcpy(lpszBuffer, lpszTemporaryBuffer, dwLen);
	memcpy(lpszBuffer + dwLen, lpData, dwLength);
	dwLen += dwLength;
	lpszTemporaryBuffer = 0;
	delete [] lpszTemporaryBuffer;
}

void DynBuffer::add(unsigned long dwData)
{
	int iNewLen = dwLen + 4;
	char *lpszTemporaryBuffer = new char[dwLen];
	memcpy(lpszTemporaryBuffer, lpszBuffer, dwLen);
	lpszBuffer = new char[iNewLen];
	memcpy(lpszBuffer, lpszTemporaryBuffer, dwLen);
	memcpy(lpszBuffer + dwLen, &dwData, 4);
	dwLen += 4;
	lpszTemporaryBuffer = 0;
	delete [] lpszTemporaryBuffer;
}

void DynBuffer::add(unsigned short uData)
{
	int iNewLen = dwLen + 2;
	char *lpszTemporaryBuffer = new char[dwLen];
	memcpy(lpszTemporaryBuffer, lpszBuffer, dwLen);
	lpszBuffer = new char[iNewLen];
	memcpy(lpszBuffer, lpszTemporaryBuffer, dwLen);
	memcpy(lpszBuffer + dwLen, &uData, 2);
	dwLen += 2;
	lpszTemporaryBuffer = 0;
	delete [] lpszTemporaryBuffer;
}

void DynBuffer::add(char bData)
{
	int iNewLen = dwLen + 1;
	char *lpszTemporaryBuffer = new char[dwLen];
	memcpy(lpszTemporaryBuffer, lpszBuffer, dwLen);
	lpszBuffer = new char[iNewLen];
	memcpy(lpszBuffer, lpszTemporaryBuffer, dwLen);
	lpszBuffer[iNewLen - 1] = bData;
	dwLen += 1;
	lpszTemporaryBuffer = 0;
	delete [] lpszTemporaryBuffer;
}

void DynBuffer::get(void *lpDest, unsigned long dwLength)
{
	memcpy(lpDest, lpszBuffer + dwPos, dwLength);
}

void DynBuffer::clear()
{
	lpszBuffer = 0;
	dwLen = 0;
	dwPos = 0;
}