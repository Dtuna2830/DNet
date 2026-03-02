#pragma once
#include <memory>
#include "TcpConnection.h"

namespace DNet
{

typedef std::function<void(std::shared_ptr<TcpConnection>)> ConnectionCallback;

class TcpServer : public EventHandler
{
public:
	TcpServer(Endpoint &localEndpoint, EventLoop &loop);
	~TcpServer();

	Error start();
	Error close();
	Error shutdown();
	std::vector<std::shared_ptr<TcpConnection>> getConnections() const;
#if DNET_WINDOWS
	void handleEvent(DWORD bytesTransferred, Event *event) override;
#elif DNET_LINUX
	void handleEvent(int32_t bytesTransferred, Event *event) override;
#endif

	void onConnection(ConnectionCallback callback);
	void onError(ErrorCallback callback);

private:
	void acceptConnection();
	void removeConnection(std::shared_ptr<TcpConnection> connection);

#if DNET_WINDOWS
	SOCKET listenSocket = INVALID_SOCKET;
#elif DNET_LINUX
	int listenSocket = -1;
#endif
	std::vector<std::shared_ptr<TcpConnection>> connections;

	Endpoint lEndpoint;
	EventLoop &eventLoop;

	ErrorCallback errorCallback;
	ConnectionCallback connectionCallback;
};

} // namespace DNet