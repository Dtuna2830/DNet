#include "DNet/TcpServer.h"
#include "NetInit.h"

namespace DNet
{

Error TcpServer::start()
{
	if (listenSocket != INVALID_SOCKET) return Error(ErrorCode::AlreadyConnected);

	int af = lEndpoint.address().adressFamily();
	if (af == AF_UNSPEC)
	{
		return Error(ErrorCode::InvalidArgument);
	}

	listenSocket = WSASocket(af, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		return Error(WSAGetLastError());
	}

	const sockaddr *addr = lEndpoint.get();
	socklen_t addrLen = lEndpoint.length();

	int bindResult = bind(listenSocket, addr, addrLen);
	if (bindResult == SOCKET_ERROR)
	{
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
		return Error(WSAGetLastError());
	}

	int listenResult = listen(listenSocket, SOMAXCONN);
	if (listenResult == SOCKET_ERROR)
	{
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
		return Error(WSAGetLastError());
	}

	eventLoop.registerHandle(reinterpret_cast<HANDLE>(listenSocket), reinterpret_cast<ULONG_PTR>(this));
	acceptConnection();
	return Error();
}

Error TcpServer::close()
{
	Error err;
	if (listenSocket != INVALID_SOCKET)
	{
		int closeResult = closesocket(listenSocket);
		if (closeResult == SOCKET_ERROR)
		{
			int wsaErr = WSAGetLastError();
			if (wsaErr != WSAENOTSOCK)
			{
				err = Error(wsaErr);
			}
		}
	}
	for (auto &conn : connections)
	{
		Error closeErr = conn->close();
		if (err.ok() && !closeErr.ok()) err = closeErr;
	}
	listenSocket = INVALID_SOCKET;
	connections.clear();
	return err;
}

Error TcpServer::shutdown()
{
	Error err;
	if (listenSocket != INVALID_SOCKET)
	{
		int closeResult = closesocket(listenSocket);
		if (closeResult == SOCKET_ERROR)
		{
			int wsaErr = WSAGetLastError();
			if (wsaErr != WSAENOTSOCK)
			{
				err = Error(wsaErr);
			}
		}
	}
	listenSocket = INVALID_SOCKET;
	return err;
}

void TcpServer::acceptConnection()
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Accept);
	int af = (lEndpoint.address().type() == AddressType::IPv4) ? AF_INET : AF_INET6;
	SOCKET acceptSocket = WSASocket(af, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	{
		if (acceptSocket == INVALID_SOCKET)
		{
			eventLoop.getEventPool().deallocateEvent(event);
			if (errorCallback) errorCallback(Error(WSAGetLastError()));
			return;
		}
	}

	DWORD addrLen = lEndpoint.length() + 16;
	event->handle = reinterpret_cast<HANDLE>(acceptSocket);
	DWORD bytesReceived = 0;
	BOOL result =
		NetInit::AcceptEx(listenSocket, acceptSocket, event->buffer.data(), 0, addrLen, addrLen, &bytesReceived, event);
	if (result == FALSE)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING)
		{
			closesocket(acceptSocket);
			eventLoop.getEventPool().deallocateEvent(event);
			if (errorCallback) errorCallback(Error(err));
		}
	}
}

void TcpServer::handleEvent(DWORD bytesTransferred, Event *event)
{
	EventPool &pool = eventLoop.getEventPool();
	if (event->error != ERROR_SUCCESS)
	{
		if (errorCallback) errorCallback(Error(event->error));
		pool.deallocateEvent(event);
		return;
	}

	if (event->type == EventType::Accept)
	{
		SOCKET acceptSocket = reinterpret_cast<SOCKET>(event->handle);
		DWORD addrLen = lEndpoint.length() + 16;

		sockaddr *localAddr = nullptr;
		sockaddr *remoteAddr = nullptr;
		int localLen = 0;
		int remoteLen = 0;

		NetInit::GetAcceptExSockaddrs(event->buffer.data(), 0, addrLen, addrLen, &localAddr, &localLen, &remoteAddr,
									  &remoteLen);

		int optResult = setsockopt(acceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
								   reinterpret_cast<const char *>(&listenSocket), sizeof(listenSocket));
		if (optResult == SOCKET_ERROR)
		{
			pool.deallocateEvent(event);
			closesocket(acceptSocket);
			if (errorCallback) errorCallback(Error(WSAGetLastError()));
			return;
		}

		Endpoint remoteEndpoint(remoteAddr);
		std::shared_ptr<TcpConnection> connection =
			std::make_shared<TcpConnection>(acceptSocket, remoteEndpoint, eventLoop);
		connection->state = State::Connected;
		connection->onInternalClose([this, connection]() { removeConnection(connection); });
		connections.push_back(connection);
		eventLoop.registerHandle(reinterpret_cast<HANDLE>(acceptSocket), reinterpret_cast<ULONG_PTR>(connection.get()));
		if (connectionCallback) connectionCallback(connection);
		connection->startReceiving();
		pool.deallocateEvent(event);
		acceptConnection();
		return;
	}
	pool.deallocateEvent(event);
}

} // namespace DNet
