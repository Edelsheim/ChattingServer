#include "session.h"

Session::Session(const size_t& id, const SOCKET& acceptSocket) :
	socket(acceptSocket),
	id(id)
{
}

Session::~Session()
{
	printf("(%lld) close socket\n", this->id);

	if (this->socket != INVALID_SOCKET)
		closesocket(this->socket);
	this->socket = INVALID_SOCKET;
}

void Session::receive(SOCKETINFO* clientData)
{
	DWORD bytes_transferred = 0;
	DWORD flag = 0;

	std::string data = std::string(clientData->buf.buf);
	printf("(%lld) data : %s\n", this->id, data.c_str());

	::WSASend(this->socket, &clientData->buf, 1, &bytes_transferred, flag, NULL, NULL);

	ZeroMemory(clientData, sizeof(SOCKETINFO));
	ZeroMemory(clientData->message, 2048);
	clientData->buf.len = 2048;
	clientData->buf.buf = clientData->message;
	bytes_transferred = 0;
	flag = 0;

	::WSARecv(this->socket, &clientData->buf, 1, &bytes_transferred, &flag, clientData, NULL);
}

void Session::disconnect()
{
	if (this->socket != INVALID_SOCKET) {
		WSABUF disconnect_buffer{};
		disconnect_buffer.len = 0;
		disconnect_buffer.buf = 0;
		::WSASend(this->socket, &disconnect_buffer, 1, 0, 0, NULL, NULL);
	}
}

const SOCKET Session::getSocket() const
{
	return this->socket;
}

const size_t Session::getID() const
{
	return this->id;
}

const size_t Session::makeID()
{
	return (size_t)this->socket;
}
