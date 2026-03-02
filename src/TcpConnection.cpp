#include "DNet/TcpConnection.h"

namespace DNet
{

TcpConnection::~TcpConnection()
{
	close();
}

const Endpoint &TcpConnection::remoteEndpoint() const
{
	return rEndpoint;
}

void TcpConnection::onData(DataCallback callback)
{
	dataCallback = callback;
}

void TcpConnection::onClose(CloseCallback callback)
{
	closeCallback = callback;
}

void TcpConnection::onShutdown(ShutdownCallback callback)
{
	shutdownCallback = callback;
}

void TcpConnection::onError(ErrorCallback callback)
{
	errorCallback = callback;
}

void TcpConnection::onConnect(ConnectCallback callback)
{
	connectCallback = callback;
}

void TcpConnection::onInternalClose(CloseCallback callback)
{
	internalCloseCallback = callback;
}

} // namespace DNet