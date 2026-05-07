#include "DNet/AsyncTcpSocket.h"

namespace DNet
{

Error AsyncTcpSocket::open(AddressType addrType)
{
	int af = Address::AddressFamily(addrType);

	socketHandle = socket(af, SOCK_STREAM, IPPROTO_TCP);
	if (socketHandle < 0) return Error(errno);
	return Error();
}

Error AsyncTcpSocket::bind(const Endpoint &endpoint)
{
	int bindResult = ::bind(socketHandle, endpoint.get(), endpoint.length());
	if (bindResult < 0) return Error(errno);
	return Error();
}

Error AsyncTcpSocket::listen(int backlog)
{
	int listenResult = ::listen(socketHandle, backlog);
	if (listenResult < 0) return Error(errno);
	return Error();
}

Error AsyncTcpSocket::close()
{
	int closeResult = ::close(socketHandle);
	socketHandle = InvalidSocket;
	if (closeResult < 0) return Error(errno);
	return Error();
}

Error AsyncTcpSocket::shutdown(ShutdownType type)
{
	int how;
	if (type == ShutdownType::Recv) how = SHUT_RD;
	else if (type == ShutdownType::Send) how = SHUT_WR;
	else if (type == ShutdownType::Both) how = SHUT_RDWR;
	else return Error(ErrorCode::InvalidArgument);

	int shutdownResult = ::shutdown(socketHandle, how);
	if (shutdownResult < 0) return Error(errno);
	return Error();
}

Error AsyncTcpSocket::asyncConnect(const Endpoint &endpoint, ConnectCallback callback)
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Connect);
	memcpy(&event->addr, endpoint.get(), endpoint.length());
	event->eventCallback = [this, callback](int32_t, Event *ev)
	{
		Error err(ev->error);
		callback(err);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	io_uring_sqe *sqe = io_uring_get_sqe(eventLoop.getEventHandle());
	if (!sqe)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(EBUSY);
	}

	io_uring_prep_connect(sqe, socketHandle, reinterpret_cast<sockaddr *>(&event->addr), endpoint.length());
	io_uring_sqe_set_data(sqe, event);

	int result = io_uring_submit(eventLoop.getEventHandle());
	if (result < 0)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(result);
	}
	return Error();
}

Error AsyncTcpSocket::asyncAccept(AsyncTcpSocket &acceptSocket, AcceptCallback callback)
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Accept);
	event->addrLen = sizeof(sockaddr_storage);
	event->eventCallback = [this, callback, &acceptSocket](int32_t fd, Event *ev)
	{
		Error err(ev->error);
		Endpoint remoteEndpoint;
		if (err.ok())
		{
			acceptSocket.socketHandle = fd;
			remoteEndpoint = Endpoint(reinterpret_cast<sockaddr *>(&ev->addr));
		}
		callback(err, remoteEndpoint);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	io_uring_sqe *sqe = io_uring_get_sqe(eventLoop.getEventHandle());
	if (!sqe)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(EBUSY);
	}

	io_uring_prep_accept(sqe, socketHandle, reinterpret_cast<sockaddr *>(&event->addr),
						 reinterpret_cast<socklen_t *>(&event->addrLen), 0);
	io_uring_sqe_set_data(sqe, event);

	int result = io_uring_submit(eventLoop.getEventHandle());
	if (result < 0)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(result);
	}
	return Error();
}

Error AsyncTcpSocket::asyncRecv(size_t size, RecvCallback callback)
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Read);
	event->buffer.reserve(Buffer::STACK_SIZE);
	event->eventCallback = [this, callback](int32_t bytesReceived, Event *ev)
	{
		Error err(ev->error);
		callback(err, ev->buffer.data(), bytesReceived);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	io_uring_sqe *sqe = io_uring_get_sqe(eventLoop.getEventHandle());
	if (!sqe)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(EBUSY);
	}

	io_uring_prep_recv(sqe, socketHandle, event->buffer.data(), event->buffer.capacity(), 0);
	io_uring_sqe_set_data(sqe, event);

	int result = io_uring_submit(eventLoop.getEventHandle());
	if (result < 0)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(result);
	}
	return Error();
}

Error AsyncTcpSocket::asyncSend(const char *data, size_t size, SendCallback callback)
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Write);
	event->buffer.assign(data, size);
	event->eventCallback = [this, callback](int32_t bytesSent, Event *ev)
	{
		Error err(ev->error);
		callback(err, bytesSent);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	io_uring_sqe *sqe = io_uring_get_sqe(eventLoop.getEventHandle());
	if (!sqe)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(EBUSY);
	}

	io_uring_prep_send(sqe, socketHandle, event->buffer.data(), event->buffer.size(), 0);
	io_uring_sqe_set_data(sqe, event);

	int result = io_uring_submit(eventLoop.getEventHandle());
	if (result < 0)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(result);
	}
	return Error();
}

Error AsyncTcpSocket::asyncRecvZc(char *data, size_t size, RecvCallback callback)
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Read);
	event->eventCallback = [this, callback, data](int32_t bytesReceived, Event *ev)
	{
		Error err(ev->error);
		callback(err, data, bytesReceived);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	io_uring_sqe *sqe = io_uring_get_sqe(eventLoop.getEventHandle());
	if (!sqe)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(EBUSY);
	}

	io_uring_prep_recv(sqe, socketHandle, data, size, 0);
	io_uring_sqe_set_data(sqe, event);

	int result = io_uring_submit(eventLoop.getEventHandle());
	if (result < 0)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(result);
	}
	return Error();
}

Error AsyncTcpSocket::asyncSendZc(const char *data, size_t size, SendCallback callback)
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Write);
	event->eventCallback = [this, callback](int32_t bytesSent, Event *ev)
	{
		Error err(ev->error);
		callback(err, bytesSent);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	io_uring_sqe *sqe = io_uring_get_sqe(eventLoop.getEventHandle());
	if (!sqe)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(EBUSY);
	}

	io_uring_prep_send(sqe, socketHandle, data, size, 0);
	io_uring_sqe_set_data(sqe, event);

	int result = io_uring_submit(eventLoop.getEventHandle());
	if (result < 0)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(result);
	}
	return Error();
}

} // namespace DNet