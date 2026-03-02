#include "DNet/UdpSocket.h"

namespace DNet
{

Error UdpSocket::bind()
{
	if (sock != INVALID_SOCKET) return Error(ErrorCode::AlreadyConnected);
	int af = lEndpoint.address().adressFamily();
	if (af == AF_UNSPEC)
	{
		return Error(ErrorCode::InvalidArgument);
	}

	sock = WSASocket(af, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (sock == INVALID_SOCKET)
	{
		return Error(WSAGetLastError());
	}

	const sockaddr *addr = lEndpoint.get();
	socklen_t addrLen = lEndpoint.length();

	int bindResult = ::bind(sock, addr, addrLen);
	if (bindResult == SOCKET_ERROR)
	{
		closesocket(sock);
		sock = INVALID_SOCKET;
		return Error(WSAGetLastError());
	}

	eventLoop.registerHandle(reinterpret_cast<HANDLE>(sock), reinterpret_cast<ULONG_PTR>(this));
	return Error();
}

Error UdpSocket::startReceiving()
{
	if (sock == INVALID_SOCKET) return Error(ErrorCode::NotConnected);

	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Read);
	event->buffer.reserve(recvSize);
	event->addrLen = sizeof(SOCKADDR_STORAGE);

	WSABUF wsabuf;
	wsabuf.len = static_cast<ULONG>(event->buffer.capacity());
	wsabuf.buf = event->buffer.data();

	DWORD flags = 0;
	DWORD bytesReceived = 0;
	int result = WSARecvFrom(sock, &wsabuf, 1, &bytesReceived, &flags, reinterpret_cast<sockaddr *>(&event->addr),
							 &event->addrLen, event, nullptr);
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

Error UdpSocket::send(const char *data, size_t length, const Endpoint &endpoint)
{
	if (sock == INVALID_SOCKET) return Error(ErrorCode::NotConnected);

	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Write);
	event->buffer.assign(data, length);

	WSABUF wsabuf;
	wsabuf.len = static_cast<ULONG>(event->buffer.size());
	wsabuf.buf = event->buffer.data();

	DWORD flags = 0;
	DWORD bytesSent = 0;
	int result = WSASendTo(sock, &wsabuf, 1, &bytesSent, flags, endpoint.get(), endpoint.length(), event, nullptr);
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

Error UdpSocket::close()
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
	return err;
}

void UdpSocket::handleEvent(DWORD bytesTransferred, Event *event)
{
	EventPool &pool = eventLoop.getEventPool();
	if (event->error != ERROR_SUCCESS)
	{
		if (errorCallback) errorCallback(Error(event->error));
		pool.deallocateEvent(event);
		return;
	}

	if (event->type == EventType::Read)
	{
		if (dataCallback)
		{
			Endpoint remote(reinterpret_cast<const sockaddr *>(&event->addr));
			dataCallback(bytesTransferred, event->buffer.data(), remote);
		}
		pool.deallocateEvent(event);
		startReceiving();
		return;
	}
	pool.deallocateEvent(event);
}

} // namespace DNet
