#include "Socket.h"
#include <string.h>

sockaddr_in sName;

SOCKET CreateTCPSocket()
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	return s;
}

bool ConnectTCPSocket(SOCKET sckTCP, const char *szServer, unsigned short uPort)
{
	struct hostent *hstEnt;
	memset(&sName, '\0', sizeof(sName));
	sName.sin_family = AF_INET;
	sName.sin_port = htons(uPort);
	char *p = (char *)szServer;
	while (*p && (isdigit(*p) || (*p == '.')))
		p++;
	if(*p){
		hstEnt = gethostbyname(szServer);
		if(hstEnt == 0)
			return false;
		memcpy(&sName.sin_addr, hstEnt->h_addr, hstEnt->h_length);
	}
	else
		sName.sin_addr.s_addr = inet_addr(szServer);
	if(connect(sckTCP, (struct sockaddr *)&sName, sizeof(sName)))
		return false;
	return true;
}

void CloseTCPSocket(SOCKET sckTCP)
{
	closesocket(sckTCP);
	shutdown(sckTCP, SD_BOTH);
	sckTCP = INVALID_SOCKET;
}