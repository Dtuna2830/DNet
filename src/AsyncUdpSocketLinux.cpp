#include "DNet/AsyncUdpSocket.h"

namespace DNet
{

Error AsyncUdpSocket::open(AddressType addrType)
{
	int af = Address::AddressFamily(addrType);

	socketHandle = socket(af, SOCK_DGRAM, IPPROTO_UDP);
	if (socketHandle < 0) return Error(errno);
	return Error();
}

Error AsyncUdpSocket::bind(const Endpoint &endpoint)
{
	int bindResult = ::bind(socketHandle, endpoint.get(), endpoint.length());
	if (bindResult < 0) return Error(errno);
	return Error();
}

Error AsyncUdpSocket::close()
{
	int closeResult = ::close(socketHandle);
	socketHandle = InvalidSocket;
	if (closeResult < 0) return Error(errno);
	return Error();
}

Error AsyncUdpSocket::asyncRecv(size_t size, RecvCallback callback)
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Read);
	event->buffer.reserve(size);
	event->addrLen = sizeof(sockaddr_storage);
	event->iov.iov_base = event->buffer.data();
	event->iov.iov_len = event->buffer.capacity();
	event->msg.msg_name = &event->addr;
	event->msg.msg_namelen = sizeof(sockaddr_storage);
	event->msg.msg_iov = &event->iov;
	event->msg.msg_iovlen = 1;
	event->eventCallback = [this, callback](int32_t bytesReceived, Event *ev)
	{
		Error err(ev->error);
		Endpoint remoteEndpoint;
		if (err.ok())
		{
			remoteEndpoint = Endpoint(reinterpret_cast<sockaddr *>(&ev->addr));
		}
		callback(err, ev->buffer.data(), bytesReceived, remoteEndpoint);
		eventLoop.getEventPool().deallocateEvent(ev);
	};

	io_uring_sqe *sqe = io_uring_get_sqe(eventLoop.getEventHandle());
	if (!sqe)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(EBUSY);
	}
	io_uring_prep_recvmsg(sqe, socketHandle, &event->msg, 0);
	io_uring_sqe_set_data(sqe, event);

	int result = io_uring_submit(eventLoop.getEventHandle());
	if (result < 0)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		return Error(result);
	}
	return Error();
}

Error AsyncUdpSocket::asyncSend(const char *data, size_t size, const Endpoint &endpoint, SendCallback callback)
{
	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Write);
	event->buffer.assign(data, size);
	memcpy(&event->addr, endpoint.get(), endpoint.length());
	// event->addrLen = endpoint.length(); used: event->msg.msg_namelen
	event->iov.iov_base = event->buffer.data();
	event->iov.iov_len = event->buffer.size();
	event->msg.msg_name = &event->addr;
	event->msg.msg_namelen = endpoint.length();
	event->msg.msg_iov = &event->iov;
	event->msg.msg_iovlen = 1;
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

	io_uring_prep_sendmsg(sqe, socketHandle, &event->msg, 0);
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