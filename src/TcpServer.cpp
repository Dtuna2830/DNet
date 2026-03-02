#include "DNet/TcpServer.h"

namespace DNet
{

TcpServer::TcpServer(Endpoint &localEndpoint, EventLoop &loop) : lEndpoint(localEndpoint), eventLoop(loop)
{
}

TcpServer::~TcpServer()
{
	close();
}

std::vector<std::shared_ptr<TcpConnection>> TcpServer::getConnections() const
{
	return connections;
}

void TcpServer::removeConnection(std::shared_ptr<TcpConnection> connection)
{
	connections.erase(std::remove(connections.begin(), connections.end(), connection), connections.end());
}

void TcpServer::onConnection(ConnectionCallback callback)
{
	connectionCallback = callback;
}

void TcpServer::onError(ErrorCallback callback)
{
	errorCallback = callback;
}

} // namespace DNet