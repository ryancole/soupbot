#ifndef SOCKETS_INCLUDED
#define SOCKETS_INCLUDED

#include <winsock2.h>

SOCKET CreateTCPSocket();
bool ConnectTCPSocket(SOCKET sckTCP, const char *szServer, unsigned short uPort);
void CloseTCPSocket(SOCKET sckTCP);

#endif