#pragma once

#include <iostream>
#include <thread>
#include <atomic>

#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "common.h"

class Client {
public:
	Client();
	~Client();

	const bool connect(const char* ip, const unsigned short port);
	const int send(const char* message, const size_t messageSize);
	const int send(std::string message);
private:
	SOCKET connecter;
	std::atomic_bool receiveRun;
	std::thread receiveThread;

	void receive();
};