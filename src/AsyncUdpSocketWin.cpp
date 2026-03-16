#include "DNet/AsyncUdpSocket.h"

namespace DNet
{

Error AsyncUdpSocket::open(AddressType addrType)
{
	int af = Address::AddressFamily(addrType);
	socketHandle = WSASocket(af, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (socketHandle == INVALID_SOCKET) return Error(WSAGetLastError());

	Error err = eventLoop.registerHandle(reinterpret_cast<HANDLE>(socketHandle), 0);
	if (!err.ok())
	{
		closesocket(socketHandle);
		socketHandle = InvalidSocket;
		return err;
	}

	return Error();
}

Error AsyncUdpSocket::bind(const Endpoint &endpoint)
{
	int bindResult = ::bind(socketHandle, endpoint.get(), endpoint.length());
	if (bindResult == SOCKET_ERROR) return Error(WSAGetLastError());
	return Error();
}

Error AsyncUdpSocket::close()
{
	int closeResult = closesocket(socketHandle);
	socketHandle = InvalidSocket;
	if (closeResult == SOCKET_ERROR) return Error(WSAGetLastError());
	return Error();
}

Error AsyncUdpSocket::asyncRecv(size_t size, RecvCallback callback)
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Read);
	event->buffer.reserve(size);
	event->addrLen = sizeof(SOCKADDR_STORAGE);
	event->eventCallback = [this, callback](DWORD bytesReceived, Event *ev)
	{
		Error err(ev->error);
		Endpoint remoteEndpoint;
		if (err.ok())
		{
			remoteEndpoint = Endpoint(reinterpret_cast<const sockaddr *>(&ev->addr));
		}
		callback(err, ev->buffer.data(), bytesReceived, remoteEndpoint);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	WSABUF wsabuf;
	wsabuf.len = static_cast<ULONG>(event->buffer.capacity());
	wsabuf.buf = event->buffer.data();
	DWORD flags = 0;
	DWORD bytesReceived = 0;
	int result = WSARecvFrom(socketHandle, &wsabuf, 1, &bytesReceived, &flags,
							 reinterpret_cast<sockaddr *>(&event->addr), &event->addrLen, event, nullptr);
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

Error AsyncUdpSocket::asyncSend(const char *data, size_t size, const Endpoint &endpoint, SendCallback callback)
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Write);
	event->buffer.assign(data, size);
	event->eventCallback = [this, callback](DWORD bytesSent, Event *ev)
	{
		Error err(ev->error);
		callback(err, bytesSent);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	WSABUF wsabuf;
	wsabuf.len = static_cast<ULONG>(event->buffer.size());
	wsabuf.buf = event->buffer.data();
	DWORD flags = 0;
	DWORD bytesSent = 0;
	int result =
		WSASendTo(socketHandle, &wsabuf, 1, &bytesSent, flags, endpoint.get(), endpoint.length(), event, nullptr);
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

}