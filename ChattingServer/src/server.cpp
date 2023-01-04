#include "server.h"
#include "message_buffer.h"

Server::Server(const unsigned short port)
{
	this->iocp = NULL;
	this->listenSocketServer = INVALID_SOCKET;
	this->serverPort = port;

	ZeroMemory(&this->serverAddr, sizeof(SOCKADDR_IN));
	this->serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	this->serverAddr.sin_family = AF_INET;
	this->serverAddr.sin_port = htons(this->serverPort);

	this->completionThreads.clear();
	this->sessions.clear();

	this->serverRun = false;
	printf("Server port : %d\n", this->serverPort);
}

Server::~Server()
{
	this->serverRun = false;

	for (auto& session : this->sessions) {
		session.second->disconnect();
	}
	this->sessions.clear();

	if (this->listenSocketServer != INVALID_SOCKET) {
		::closesocket(this->listenSocketServer);
	}

	if (this->iocp != NULL) {
		::CloseHandle(this->iocp);
	}

	for (auto& thread : this->completionThreads)
		if (thread.joinable())
			thread.join();

	::WSACleanup();
}

bool Server::run()
{
	WSADATA wsa_data{};
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		printf("WSA Startup fail\n");
		return false;
	}

	this->iocp = this->createIOCP();
	if (this->iocp == NULL) {
		printf("Create IOCP fail.\n");
		return false;
	}

	if (this->makeSocket(this->listenSocketServer) == false) {
		printf("Make listen socket fail.\n");
		return false;
	}

	if (this->bindSocket(this->listenSocketServer, this->serverAddr) == false) {
		printf("Bind listen socket fail.\n");
		::closesocket(this->listenSocketServer);
		return false;
	}

	if (this->listenSocket(this->listenSocketServer, SOMAXCONN) == false) {
		printf("Listen socket fail.\n");
		::closesocket(this->listenSocketServer);
		return false;
	}

	this->serverRun = true;

	this->makeWorkingThreads();

	return this->acceptSocket(this->listenSocketServer, this->iocp);
}

HANDLE Server::createIOCP()
{
	return ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
}

void Server::makeWorkingThreads()
{
	SYSTEM_INFO system_info{};
	GetSystemInfo(&system_info);
	DWORD num = system_info.dwNumberOfProcessors * 2;
	for (DWORD i = 0; i != num; i++)
		this->completionThreads.push_back(std::thread(&Server::completionThread, this));
}

bool Server::makeSocket(SOCKET& socket)
{
	socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (socket == SOCKET_ERROR) return false;
	return true;
}

bool Server::bindSocket(SOCKET& socket, const SOCKADDR_IN& addr)
{
	if (::bind(socket, (const sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		return false;
	else
		return true;
}

bool Server::listenSocket(SOCKET& socket, const int& backlog)
{
	if (::listen(socket, backlog) == SOCKET_ERROR)
		return false;
	else
		return true;
}

bool Server::watchSocket(SOCKET& socket, HANDLE& cp, const ULONG_PTR& watchKey)
{
	HANDLE handle = ::CreateIoCompletionPort((HANDLE)socket, cp, watchKey, 0);
	if (handle == cp) return true;
	else return false;
}

bool Server::acceptSocket(SOCKET& socket, HANDLE& cp)
{
	while (true) {
		if (this->serverRun == false) break;

		printf("Ready accept client...\n");

		SOCKADDR_IN client_addr{};
		INT client_addr_len = sizeof(SOCKADDR_IN) + 16;

		SOCKET client = ::WSAAccept(socket, (sockaddr*)&client_addr, &client_addr_len, 0, 0);
		if (client != INVALID_SOCKET) {
			// make id
			const size_t id = (size_t)client;
			//

			printf("New client %lld\n", id);

			if (this->sessions.find(id) != this->sessions.end()) {
				this->sessions.find(id)->second->disconnect();
				this->sessions.find(id)->second = std::make_shared<Session>(id, client);
			}
			else {
				this->sessions.insert({ id, std::make_shared<Session>(id, client) });
			}

			if (this->watchSocket(client, cp, id) == false) {
				printf("Watch client fail.\n");
				::closesocket(client);
				this->sessions.unsafe_erase(id);
			}
			else {
				DWORD trnas = 0;
				DWORD flag = 0;
				SOCKETINFO socket_info{};
				socket_info.buf.len = MAX_BUF;
				socket_info.buf.buf = socket_info.message;
				int recv_result = ::WSARecv(client, &socket_info.buf, 1, &trnas, &flag, &socket_info, 0);
				if (recv_result == SOCKET_ERROR) {
					int error_code = ::WSAGetLastError();
					if (error_code != WSA_IO_PENDING) {
						printf("%lld accept client recv fail.\n", id);
						::closesocket(client);
						this->sessions.unsafe_erase(id);
					}
				}

				std::string join_message = "New client " + std::to_string(id) + " join!";

				MessageStruct message{};
				message.type = MessageType::alert;
				memcpy(message.message, join_message.c_str(), join_message.length());

				WSABUF new_client_join{};
				char message_buffer[MAX_BUF] = { 0, };
				new_client_join.buf = message_buffer;
				if (MessageBuffer::Serialize(message, (size_t&)new_client_join.len, new_client_join.buf)) {
					for (auto& other_session : this->sessions) {
						if (other_session.first != id)
							other_session.second->send(new_client_join);
					}
				}

				printf("Watch client start.\n");
			}
		}
	}

	for (auto& thread : this->completionThreads)
		if (thread.joinable())
			thread.join();

	return true;
}

DWORD __stdcall Server::completionThread()
{
	while (true) {
		if (this->serverRun == false) break;
		DWORD bytes_trans = 0;
		DWORD flag = 0;
		size_t key = 0;

		SOCKETINFO* socket_info{};
		BOOL result = ::GetQueuedCompletionStatus(this->iocp, &bytes_trans, &key, (LPOVERLAPPED*)&socket_info, INFINITE);
		auto session = this->sessions.find(key);
		if (session != this->sessions.end()) {
			if (result == FALSE && bytes_trans == 0) {
				printf("%lld close socket 1\n", key);
				session->second->disconnect();
				this->sessions.unsafe_erase(key);
			}
			else if (result == FALSE) {
				printf("%lld close socket 2\n", key);
				session->second->disconnect();
				this->sessions.unsafe_erase(key);
			}
			else if (bytes_trans == 0) {
				printf("%lld socket bytes transferred 0\n", key);
			}
			else {
				for (auto& other_session : this->sessions) {
					if (other_session.first != key)
						other_session.second->send(socket_info->buf);
				}

				session->second->receive(socket_info);
			}
		}
	}

	return 0;
}
