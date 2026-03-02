#include "DNet/UdpSocket.h"

namespace DNet
{

UdpSocket::UdpSocket(Endpoint &localEndpoint, EventLoop &loop, size_t receiveSize)
	: lEndpoint(localEndpoint), eventLoop(loop), recvSize(receiveSize)
{
}

UdpSocket::~UdpSocket()
{
	close();
}

void UdpSocket::setReceiveSize(size_t size)
{
	recvSize = size;
}

void UdpSocket::onData(DataCallback callback)
{
	dataCallback = callback;
}

void UdpSocket::onError(ErrorCallback callback)
{
	errorCallback = callback;
}

} // namespace DNet