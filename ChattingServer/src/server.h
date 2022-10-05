#pragma once

#include <iostream>
#include <vector>
#include <concurrent_unordered_map.h>
#include <thread>
#include <memory>

#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include "session.h"

class Server {
public:
	Server(const unsigned short port);
	~Server();

	bool run();

private:
	unsigned short serverPort;
	HANDLE iocp;
	SOCKET listenSocketServer;
	SOCKADDR_IN serverAddr;

	std::vector<std::thread> completionThreads;

	concurrency::concurrent_unordered_map<size_t, std::shared_ptr<Session>> sessions;

	HANDLE createIOCP();

	void makeWorkingThreads();

	bool makeSocket(SOCKET& socket);

	bool bindSocket(SOCKET& socket, const SOCKADDR_IN& addr);

	bool listenSocket(SOCKET& socket, const int& backlog);

	bool watchSocket(SOCKET& socket, HANDLE& cp, const ULONG_PTR& watchKey);

	bool acceptSocket(SOCKET& socket, HANDLE& cp);

	// thread call
	DWORD __stdcall completionThread();
};