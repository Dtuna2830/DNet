#pragma once
#include <memory>
#include "TcpConnection.h"

namespace DNet
{

class TcpClient
{
public:
	TcpClient(Endpoint &remoteEndpoint, EventLoop &loop);
	~TcpClient() = default;

	void connect(std::function<void(std::shared_ptr<TcpConnection>, const Error &)> callback);

private:
	Endpoint rEndpoint;
	EventLoop &eventLoop;

	std::shared_ptr<TcpConnection> connection;
};

} // namespace DNet