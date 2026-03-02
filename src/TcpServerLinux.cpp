#include "DNet/TcpServer.h"

namespace DNet
{

Error TcpServer::start()
{
	if (listenSocket >= 0) return Error(ErrorCode::AlreadyConnected);

	int af = lEndpoint.address().adressFamily();
	if (af == AF_UNSPEC)
	{
		return Error(ErrorCode::InvalidArgument);
	}

	listenSocket = socket(af, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket < 0)
	{
		return Error(errno);
	}

	int opt = 1;
	int optResult = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (optResult < 0)
	{
		::close(listenSocket);
		listenSocket = -1;
		return Error(errno);
	}

	const sockaddr *addr = lEndpoint.get();
	socklen_t addrLen = lEndpoint.length();

	int bindResult = bind(listenSocket, addr, addrLen);
	if (bindResult < 0)
	{
		::close(listenSocket);
		listenSocket = -1;
		return Error(errno);
	}

	int listenResult = listen(listenSocket, SOMAXCONN);
	if (listenResult < 0)
	{
		::close(listenSocket);
		listenSocket = -1;
		return Error(errno);
	}

	acceptConnection();
	return Error();
}

Error TcpServer::close()
{
	Error err;
	if (listenSocket >= 0)
	{
		int closeResult = ::close(listenSocket);
		if (closeResult < 0)
		{
			int errCode = errno;
			if (errCode != EBADF)
			{
				err = Error(errCode);
			}
		}
	}
	for (auto &conn : connections)
	{
		Error closeErr = conn->close();
		if (err.ok() && !closeErr.ok()) err = closeErr;
	}
	listenSocket = -1;
	connections.clear();
	return err;
}

Error TcpServer::shutdown()
{
	Error err;
	if (listenSocket >= 0)
	{
		int closeResult = ::close(listenSocket);
		if (closeResult < 0)
		{
			int errCode = errno;
			if (errCode != EBADF)
			{
				err = Error(errCode);
			}
		}
	}
	listenSocket = -1;
	return err;
}

void TcpServer::acceptConnection()
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Accept);
	event->eventHandler = this;
	event->addrLen = sizeof(sockaddr_storage);

	io_uring_sqe *sqe = io_uring_get_sqe(eventLoop.getEventHandle());
	if (!sqe)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		if (errorCallback) errorCallback(Error(EBUSY));
		return;
	}

	io_uring_prep_accept(sqe, listenSocket, reinterpret_cast<sockaddr *>(&event->addr),
						 reinterpret_cast<socklen_t *>(&event->addrLen), 0);
	io_uring_sqe_set_data(sqe, event);
	int result = io_uring_submit(eventLoop.getEventHandle());
	if (result < 0)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		if (errorCallback) errorCallback(Error(result));
	}
}

void TcpServer::handleEvent(int32_t bytesTransferred, Event *event)
{
	EventPool &pool = eventLoop.getEventPool();
	if (event->error != 0)
	{
		if (errorCallback) errorCallback(Error(event->error));
		pool.deallocateEvent(event);
		acceptConnection();
		return;
	}

	if (event->type == EventType::Accept)
	{
		if (bytesTransferred < 0)
		{
			pool.deallocateEvent(event);
			if (errorCallback) errorCallback(Error(bytesTransferred));
			acceptConnection();
			return;
		}
		int acceptSocket = bytesTransferred;
		Endpoint remoteEndpoint(reinterpret_cast<sockaddr *>(&event->addr));
		std::shared_ptr<TcpConnection> connection =
			std::make_shared<TcpConnection>(acceptSocket, remoteEndpoint, eventLoop);
		connection->state = State::Connected;
		connection->onInternalClose([this, connection]() { removeConnection(connection); });
		connections.push_back(connection);
		if (connectionCallback) connectionCallback(connection);
		connection->startReceiving();
		pool.deallocateEvent(event);
		acceptConnection();
		return;
	}
	pool.deallocateEvent(event);
}

} // namespace DNet
