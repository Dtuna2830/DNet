#include "DNet/TcpConnection.h"

namespace DNet
{

TcpConnection::TcpConnection(int socket, Endpoint &remoteEndpoint, EventLoop &loop)
	: sock(socket), rEndpoint(remoteEndpoint), eventLoop(loop)
{
}

Error TcpConnection::send(const char *data, size_t length)
{
	if (state != State::Connected) return Error(ErrorCode::NotConnected);

	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Write);
	event->buffer.assign(data, length);
	event->eventHandler = this;

	io_uring_sqe *sqe = io_uring_get_sqe(eventLoop.getEventHandle());
	if (!sqe)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(EBUSY);
	}

	io_uring_prep_send(sqe, sock, event->buffer.data(), event->buffer.size(), 0);
	io_uring_sqe_set_data(sqe, event);

	int result = io_uring_submit(eventLoop.getEventHandle());
	if (result < 0)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(result);
	}
	return Error();
}

Error TcpConnection::startReceiving()
{
	if (state != State::Connected) return Error(ErrorCode::NotConnected);

	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Read);
	event->buffer.reserve(Buffer::STACK_SIZE);
	event->eventHandler = this;

	io_uring_sqe *sqe = io_uring_get_sqe(eventLoop.getEventHandle());
	if (!sqe)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(EBUSY);
	}

	io_uring_prep_recv(sqe, sock, event->buffer.data(), event->buffer.capacity(), 0);
	io_uring_sqe_set_data(sqe, event);

	int result = io_uring_submit(eventLoop.getEventHandle());
	if (result < 0)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(result);
	}
	return Error();
}

Error TcpConnection::close()
{
	if (sock < 0) return Error();
	Error err;
	int closeResult = ::close(sock);
	if (closeResult < 0)
	{
		err = Error(errno);
	}
	sock = -1;
	state = State::Disconnected;
	return err;
}

Error TcpConnection::shutdown()
{
	if (sock < 0) return Error();
	if (shutdownSent) return Error();
	Error err;

	int shutdownResult = ::shutdown(sock, SHUT_WR);
	if (shutdownResult < 0)
	{
		err = Error(errno);
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

void TcpConnection::handleEvent(int32_t bytesTransferred, Event *event)
{
	EventPool &pool = eventLoop.getEventPool();
	if (event->error < 0)
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
			if (shutdownSent && sock >= 0)
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
		state = State::Connected;
		if (connectCallback) connectCallback();
		startReceiving();
		return;
	}
	pool.deallocateEvent(event);
}

} // namespace DNet
