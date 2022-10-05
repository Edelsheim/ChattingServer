#pragma once
#include <WinSock2.h>

#define MAX_BUF 2048

struct SOCKETINFO : public WSAOVERLAPPED {
	WSABUF buf;
	CHAR message[MAX_BUF];
};