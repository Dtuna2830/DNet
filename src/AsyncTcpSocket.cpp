#include "DNet/AsyncTcpSocket.h"

namespace DNet
{

AsyncTcpSocket::AsyncTcpSocket(EventLoop &loop) : socketHandle(InvalidSocket), eventLoop(loop)
{
}

AsyncTcpSocket::~AsyncTcpSocket()
{
	close();
}

AsyncTcpSocket::AsyncTcpSocket(AsyncTcpSocket &&o) noexcept : socketHandle(o.socketHandle), eventLoop(o.eventLoop)
{
	o.socketHandle = InvalidSocket;
}

AsyncTcpSocket &AsyncTcpSocket::operator=(AsyncTcpSocket &&o) noexcept
{
	if (this != &o)
	{
		close();
		socketHandle = o.socketHandle;
		o.socketHandle = InvalidSocket;
	}
	return *this;
}

bool AsyncTcpSocket::isClosed()
{
	return socketHandle == InvalidSocket;
}

Error AsyncTcpSocket::asyncRecv(RecvCallback callback)
{
	return asyncRecv(Buffer::STACK_SIZE, std::move(callback));
}

} // namespace DNet