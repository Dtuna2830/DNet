#include "DNet/AsyncTcpSocket.h"
#include "NetInit.h"

namespace DNet
{

Error AsyncTcpSocket::open(AddressType addrType)
{
	int af = Address::AddressFamily(addrType);

	socketHandle = WSASocket(af, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
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

Error AsyncTcpSocket::bind(const Endpoint &endpoint)
{
	int bindResult = ::bind(socketHandle, endpoint.get(), endpoint.length());
	if (bindResult == SOCKET_ERROR) return Error(WSAGetLastError());
	return Error();
}

Error AsyncTcpSocket::listen(int backlog)
{
	int listenResult = ::listen(socketHandle, backlog);
	if (listenResult == SOCKET_ERROR) return Error(WSAGetLastError());
	return Error();
}

Error AsyncTcpSocket::close()
{
	int closeResult = closesocket(socketHandle);
	socketHandle = InvalidSocket;
	if (closeResult == SOCKET_ERROR) return Error(WSAGetLastError());
	return Error();
}

Error AsyncTcpSocket::shutdown(ShutdownType type)
{
	int how;
	if (type == ShutdownType::Recv) how = SD_RECEIVE;
	else if (type == ShutdownType::Send) how = SD_SEND;
	else if (type == ShutdownType::Both) how = SD_BOTH;
	else return Error(ErrorCode::InvalidArgument);

	int shutdownResult = ::shutdown(socketHandle, how);
	if (shutdownResult == SOCKET_ERROR) return Error(WSAGetLastError());
	return Error();
}

Error AsyncTcpSocket::asyncConnect(const Endpoint &endpoint, ConnectCallback callback)
{
	AddressType addrType = endpoint.address().type();
	Endpoint lEndpoint;
	if (addrType == AddressType::IPv4) lEndpoint = Endpoint::AnyV4(0);
	else if (addrType == AddressType::IPv6) lEndpoint = Endpoint::AnyV6(0);
	else return Error(ErrorCode::InvalidArgument);
	int bindResult = ::bind(socketHandle, lEndpoint.get(), lEndpoint.length());
	if (bindResult == SOCKET_ERROR) return Error(WSAGetLastError());

	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Connect);
	// Note: ConnectEx seems to copy sockaddr internally, but this is undocumented and potentially unsafe.
	/*
	memcpy(&event->addr, endpoint.get(), endpoint.length());
	event->addrLen = endpoint.length();
	*/
	event->eventCallback = [this, callback](DWORD, Event *ev)
	{
		Error err(ev->error);
		if (err.ok())
		{
			int optResult = setsockopt(socketHandle, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, nullptr, 0);
			if (optResult == SOCKET_ERROR) err = Error(WSAGetLastError());
		}
		callback(err);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	DWORD bytes;
	// BOOL results = NetInit::ConnectEx(socketHandle, reinterpret_cast<const sockaddr *>(&event->addr), event->addrLen,nullptr, NULL, &bytes, event);
	BOOL result = NetInit::ConnectEx(socketHandle, endpoint.get(), endpoint.length(), nullptr, NULL, &bytes, event);
	if (result == FALSE)
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

Error AsyncTcpSocket::asyncAccept(AsyncTcpSocket &acceptSocket, AcceptCallback callback)
{
	DWORD addrLen = sizeof(SOCKADDR_STORAGE) + 16;
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Accept);
	event->eventCallback = [this, callback, &acceptSocket, addrLen](DWORD, Event *ev)
	{
		Error err(ev->error);
		Endpoint remoteEndpoint;
		if (err.ok())
		{
			int optResult = setsockopt(acceptSocket.socketHandle, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
									   reinterpret_cast<const char *>(&socketHandle), sizeof(socketHandle));
			if (optResult == SOCKET_ERROR)
			{
				err = Error(WSAGetLastError());
			}
			else
			{
				sockaddr *localAddr = nullptr;
				sockaddr *remoteAddr = nullptr;
				int localLen = 0;
				int remoteLen = 0;

				NetInit::GetAcceptExSockaddrs(ev->buffer.data(), 0, addrLen, addrLen, &localAddr, &localLen,
											  &remoteAddr, &remoteLen);
				remoteEndpoint = Endpoint(remoteAddr);
			}
		}
		callback(err, remoteEndpoint);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	DWORD bytes;
	BOOL result = NetInit::AcceptEx(socketHandle, acceptSocket.socketHandle, event->buffer.data(), 0, addrLen, addrLen,
									&bytes, event);
	if (result == FALSE)
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

Error AsyncTcpSocket::asyncRecv(size_t size, RecvCallback callback)
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Read);
	event->buffer.reserve(Buffer::STACK_SIZE);
	event->eventCallback = [this, callback](DWORD bytesReceived, Event *ev)
	{
		Error err(ev->error);
		callback(err, ev->buffer.data(), bytesReceived);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	WSABUF wsabuf;
	wsabuf.len = static_cast<ULONG>(event->buffer.capacity());
	wsabuf.buf = event->buffer.data();
	DWORD flags = 0;
	DWORD bytesReceived = 0;
	int result = WSARecv(socketHandle, &wsabuf, 1, &bytesReceived, &flags, event, nullptr);
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

Error AsyncTcpSocket::asyncSend(const char *data, size_t size, SendCallback callback)
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
	int result = WSASend(socketHandle, &wsabuf, 1, &bytesSent, flags, event, nullptr);
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

Error AsyncTcpSocket::asyncRecvZc(char *data, size_t size, RecvCallback callback)
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Read);
	event->eventCallback = [this, callback, data](DWORD bytesReceived, Event *ev)
	{
		Error err(ev->error);
		callback(err, data, bytesReceived);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	WSABUF wsabuf;
	wsabuf.len = static_cast<ULONG>(size);
	wsabuf.buf = data;
	DWORD flags = 0;
	DWORD bytesReceived = 0;
	int result = WSARecv(socketHandle, &wsabuf, 1, &bytesReceived, &flags, event, nullptr);
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

Error AsyncTcpSocket::asyncSendZc(const char *data, size_t size, SendCallback callback)
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Write);
	event->eventCallback = [this, callback](DWORD bytesSent, Event *ev)
	{
		Error err(ev->error);
		callback(err, bytesSent);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	WSABUF wsabuf;
	wsabuf.len = static_cast<ULONG>(size);
	wsabuf.buf = const_cast<char *>(data);
	DWORD flags = 0;
	DWORD bytesSent = 0;
	int result = WSASend(socketHandle, &wsabuf, 1, &bytesSent, flags, event, nullptr);
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

} // namespace DNet