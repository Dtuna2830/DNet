#include "DNet/TcpConnection.h"
#include <MSWSock.h>

namespace DNet
{

TcpConnection::TcpConnection(SOCKET socket, Endpoint &remoteEndpoint, EventLoop &loop)
	: sock(socket), rEndpoint(remoteEndpoint), eventLoop(loop)
{
}

Error TcpConnection::send(const char *data, size_t length)
{
	if (state != State::Connected) return Error(ErrorCode::NotConnected);

	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Write);
	event->buffer.assign(data, length);

	WSABUF wsabuf;
	wsabuf.len = static_cast<ULONG>(event->buffer.size());
	wsabuf.buf = event->buffer.data();

	DWORD flags = 0;
	DWORD bytesSent = 0;
	int result = WSASend(sock, &wsabuf, 1, &bytesSent, flags, event, nullptr);
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			eventLoop.getEventPool().deallocateEvent(event);
			return Error(err);
		}
	}
	return Error();
}

Error TcpConnection::startReceiving()
{
	if (state != State::Connected) return Error(ErrorCode::NotConnected);

	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Read);
	event->buffer.reserve(Buffer::STACK_SIZE);
	WSABUF wsabuf;
	wsabuf.len = static_cast<ULONG>(event->buffer.capacity());
	wsabuf.buf = event->buffer.data();

	DWORD flags = 0;
	DWORD bytesReceived = 0;
	int result = WSARecv(sock, &wsabuf, 1, &bytesReceived, &flags, event, nullptr);
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			eventLoop.getEventPool().deallocateEvent(event);
			return Error(err);
		}
	}
	return Error();
}

Error TcpConnection::close()
{
	if (sock == INVALID_SOCKET) return Error();
	Error err;
	int closeResult = closesocket(sock);
	if (closeResult == SOCKET_ERROR)
	{
		int wsaErr = WSAGetLastError();
		if (wsaErr != WSAENOTSOCK)
		{
			err = Error(wsaErr);
		}
	}
	sock = INVALID_SOCKET;
	state = State::Disconnected;
	return err;
}

Error TcpConnection::shutdown()
{
	if (sock == INVALID_SOCKET) return Error();
	if (shutdownSent) return Error();
	Error err;

	int shutdownResult = ::shutdown(sock, SD_SEND);
	if (shutdownResult == SOCKET_ERROR)
	{
		int wsaErr = WSAGetLastError();
		if (wsaErr != WSAENOTSOCK)
		{
			err = Error(wsaErr);
		}
	}

	shutdownSent = true;
	if (shutdownReceived)
	{
		Error closeErr = close();
		if (err.ok() && !closeErr.ok()) err = closeErr;
		if (closeCallback) closeCallback();
		if (internalCloseCallback) internalCloseCallback();
	}

	return err;
}

void TcpConnection::handleEvent(DWORD bytesTransferred, Event *event)
{
	EventPool &pool = eventLoop.getEventPool();
	if (event->error != ERROR_SUCCESS)
	{
		if (errorCallback) errorCallback(Error(event->error));
		close();
		if (closeCallback) closeCallback();
		if (internalCloseCallback) internalCloseCallback();
		pool.deallocateEvent(event);
		return;
	}

	if (event->type == EventType::Read)
	{
		if (bytesTransferred == 0)
		{
			pool.deallocateEvent(event);
			shutdownReceived = true;
			if (shutdownCallback) shutdownCallback();
			if (shutdownSent && sock != INVALID_SOCKET)
			{
				close();
				if (closeCallback) closeCallback();
				if (internalCloseCallback) internalCloseCallback();
			}
			return;
		}
		if (dataCallback) dataCallback(bytesTransferred, event->buffer.data());
		pool.deallocateEvent(event);
		startReceiving();
		return;
	}
	else if (event->type == EventType::Connect)
	{
		pool.deallocateEvent(event);
		int optResult = setsockopt(sock, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, nullptr, 0);
		if (optResult == SOCKET_ERROR)
		{
			if (errorCallback) errorCallback(Error(WSAGetLastError()));
			close();
			if (closeCallback) closeCallback();
			if (internalCloseCallback) internalCloseCallback();
			return;
		}

		state = State::Connected;
		if (connectCallback) connectCallback();
		startReceiving();
		return;
	}
	pool.deallocateEvent(event);
}

} // namespace DNet
