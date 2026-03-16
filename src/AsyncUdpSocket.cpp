#include "DNet/AsyncUdpSocket.h"

namespace DNet
{

AsyncUdpSocket::AsyncUdpSocket(EventLoop &loop) : socketHandle(InvalidSocket), eventLoop(loop)
{
}

AsyncUdpSocket::~AsyncUdpSocket()
{
	close();
}

AsyncUdpSocket::AsyncUdpSocket(AsyncUdpSocket &&o) noexcept : socketHandle(o.socketHandle), eventLoop(o.eventLoop)
{
	o.socketHandle = InvalidSocket;
}

AsyncUdpSocket &AsyncUdpSocket::operator=(AsyncUdpSocket &&o) noexcept
{
	if (this != &o)
	{
		close();
		socketHandle = o.socketHandle;
		o.socketHandle = InvalidSocket;
	}
	return *this;
}

bool AsyncUdpSocket::isClosed()
{
	return socketHandle == InvalidSocket;
}

Error AsyncUdpSocket::asyncRecv(RecvCallback callback)
{
	return asyncRecv(Buffer::STACK_SIZE, std::move(callback));
}

}