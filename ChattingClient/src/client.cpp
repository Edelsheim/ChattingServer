#include "client.h"
#include "../../Common/src/message_buffer.h"

Client::Client()
{
	this->connecter = INVALID_SOCKET;
	this->receiveRun = false;
}

Client::~Client()
{
	this->receiveRun = false;

	if (this->receiveThread.joinable())
		this->receiveThread.join();

	if (this->connecter != INVALID_SOCKET) {
		::closesocket(this->connecter);
	}

	::WSACleanup();
}

const bool Client::connect(const char* ip, const unsigned short port)
{
	WSADATA wsa_data{};
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		printf("Client WSAStartup fail.\n");
		::WSACleanup();
		return false;
	}

	SOCKADDR_IN server_addr{};
	if (inet_pton(AF_INET, ip, &server_addr.sin_addr.S_un.S_addr) <= 0) {
		printf("Client set server addr fail.\n");
		::WSACleanup();
		return false;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	this->connecter = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

	if (::WSAConnect(this->connecter, (const sockaddr*)&server_addr, (int)sizeof(server_addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
		printf("Client connect to server(%s:%d) fail.\n", ip, port);
		return false;
	}

	this->receiveRun = true;
	this->receiveThread = std::thread(&Client::receive, this);

	return true;
}

const int Client::send(const char* message, const size_t messageSize)
{
	DWORD byte_send = 0;
	DWORD flag = 0;

	WSABUF buff{};
	char buf[MAX_BUF] = { 0, };
	size_t buf_size = 0;
	MessageStruct ms{};
	ms.type = MessageType::send;
	memcpy(ms.message, message, messageSize);
	MessageBuffer::Serialize(ms, buf_size, buf);

	buff.buf = (CHAR*)buf;
	buff.len = (ULONG)buf_size;
	return ::WSASend(this->connecter, &buff, 1, &byte_send, flag, NULL, NULL);
}

const int Client::send(std::string message)
{
	return this->send(message.c_str(), message.length());
}

void Client::receive()
{
	while (true) {
		if (this->receiveRun == false) break;

		SOCKETINFO socket_info{};
		ZeroMemory(&socket_info, sizeof(SOCKETINFO));
		socket_info.buf.len = MAX_BUF;
		socket_info.buf.buf = socket_info.message;

		DWORD bytes_trans = 0;
		DWORD flag = 0;
		int result = ::WSARecv(this->connecter, &socket_info.buf, 1, &bytes_trans, &flag, NULL, NULL);
		
		MessageStruct ms = MessageBuffer::Deserialize(socket_info.buf.buf);
		if (bytes_trans > 0) {
			printf("receive : %d - %s\n", ms.type, ms.message);
		}
	}
}
