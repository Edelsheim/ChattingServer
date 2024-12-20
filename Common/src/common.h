#pragma once
#include <WinSock2.h>

#define MAX_BUF 2048

enum class MessageType : int {
	login = 0,
	logout,
	alert,
	send,
	receive
};

struct MessageStruct {
	MessageType type;
	size_t session_id;
	CHAR message[MAX_BUF];
};

struct SOCKETINFO : public WSAOVERLAPPED {
	WSABUF buf;
	CHAR message[MAX_BUF];
};