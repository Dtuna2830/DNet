#include "DNet/UdpSocket.h"

namespace DNet
{

Error UdpSocket::bind()
{
	if (sock >= 0) return Error(ErrorCode::AlreadyConnected);
	int af = lEndpoint.address().adressFamily();
	if (af == AF_UNSPEC)
	{
		return Error(ErrorCode::InvalidArgument);
	}

	sock = socket(af, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0)
	{
		return Error(errno);
	}

	const sockaddr *addr = lEndpoint.get();
	socklen_t addrLen = lEndpoint.length();

	int bindResult = ::bind(sock, addr, addrLen);
	if (bindResult < 0)
	{
		::close(sock);
		sock = -1;
		return Error(errno);
	}

	return Error();
}

Error UdpSocket::startReceiving()
{
	if (sock < 0) return Error(ErrorCode::NotConnected);

	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Read);
	event->eventHandler = this;

	event->buffer.reserve(recvSize);
	event->addrLen = sizeof(sockaddr_storage);
	event->iov.iov_base = event->buffer.data();
	event->iov.iov_len = event->buffer.capacity();

	event->msg.msg_name = &event->addr;
	event->msg.msg_namelen = sizeof(sockaddr_storage);
	event->msg.msg_iov = &event->iov;
	event->msg.msg_iovlen = 1;

	io_uring_sqe *sqe = io_uring_get_sqe(eventLoop.getEventHandle());
	if (!sqe)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(EBUSY);
	}

	io_uring_prep_recvmsg(sqe, sock, &event->msg, 0);
	io_uring_sqe_set_data(sqe, event);

	int result = io_uring_submit(eventLoop.getEventHandle());
	if (result < 0)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(result);
	}
	return Error();
}

Error UdpSocket::send(const char *data, size_t length, const Endpoint &endpoint)
{
	if (sock < 0) return Error(ErrorCode::NotConnected);

	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Write);
	event->eventHandler = this;
	event->buffer.assign(data, length);

	event->iov.iov_base = event->buffer.data();
	event->iov.iov_len = event->buffer.size();

	event->msg.msg_name = const_cast<sockaddr *>(endpoint.get());
	event->msg.msg_namelen = endpoint.length();
	event->msg.msg_iov = &event->iov;
	event->msg.msg_iovlen = 1;

	io_uring_sqe *sqe = io_uring_get_sqe(eventLoop.getEventHandle());
	if (!sqe)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(EBUSY);
	}

	io_uring_prep_sendmsg(sqe, sock, &event->msg, 0);
	io_uring_sqe_set_data(sqe, event);

	int result = io_uring_submit(eventLoop.getEventHandle());
	if (result < 0)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(result);
	}
	return Error();
}

Error UdpSocket::close()
{
	if (sock < 0) return Error();
	Error err;
	int closeResult = ::close(sock);
	if (closeResult < 0)
	{
		if (errno != EBADF)
		{
			err = Error(errno);
		}
	}
	sock = -1;
	return err;
}

void UdpSocket::handleEvent(int32_t bytesTransferred, Event *event)
{
	EventPool &pool = eventLoop.getEventPool();
	if (event->error != 0)
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
