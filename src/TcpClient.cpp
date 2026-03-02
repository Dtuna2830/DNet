#include "DNet/TcpClient.h"

namespace DNet
{

TcpClient::TcpClient(Endpoint &remoteEndpoint, EventLoop &loop) : rEndpoint(remoteEndpoint), eventLoop(loop)
{
}

} // namespace DNet