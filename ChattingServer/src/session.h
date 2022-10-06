#pragma once
#include <string>
#include <chrono>
#include <thread>

#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include "common.h"

class Session {
public:
	Session(const size_t& id, const SOCKET& acceptSocket);
	Session(const Session&) = delete;
	~Session();

	void receive(SOCKETINFO* clientData);
	void send(const WSABUF sendData);
	void disconnect();

	const SOCKET getSocket() const;
	const size_t getID() const;
private:
	SOCKET socket;
	size_t id;

	const size_t makeID();
};