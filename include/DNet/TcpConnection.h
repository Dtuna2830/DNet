#pragma once
#include <functional>
#include "Event/EventLoop.h"
#include "Event/EventHandler.h"
#include "Ip/Endpoint.h"
#include "Error.h"

namespace DNet
{

class TcpServer;
class TcpClient;

typedef std::function<void(size_t, const char *)> DataCallback;
typedef std::function<void()> CloseCallback;
typedef std::function<void()> ShutdownCallback;
typedef std::function<void(const Error &)> ErrorCallback;
typedef std::function<void()> ConnectCallback;

enum State
{
	Disconnected,
	Connecting,
	Connected
};

class TcpConnection : public EventHandler
{
	friend class TcpServer;
	friend class TcpClient;

public:
#if DNET_WINDOWS
	TcpConnection(SOCKET socket, Endpoint &remoteEndpoint, EventLoop &loop);
#elif DNET_LINUX
	TcpConnection(int socket, Endpoint &remoteEndpoint, EventLoop &loop);
#endif
	~TcpConnection();

	Error send(const char *data, size_t length);
	Error startReceiving();
	Error close();
	Error shutdown();
	const Endpoint &remoteEndpoint() const;
#if DNET_WINDOWS
	void handleEvent(DWORD bytesTransferred, Event *event) override;
#elif DNET_LINUX
	void handleEvent(int32_t bytesTransferred, Event *event) override;
#endif

	void onData(DataCallback callback);
	void onClose(CloseCallback callback);
	void onShutdown(ShutdownCallback callback);
	void onError(ErrorCallback callback);
	void onConnect(ConnectCallback callback);

private:
	void onInternalClose(CloseCallback callback);

#if DNET_WINDOWS
	SOCKET sock;
#elif DNET_LINUX
	int sock;
#endif
	Endpoint rEndpoint;
	EventLoop &eventLoop;

	bool shutdownReceived = false;
	bool shutdownSent = false;
	State state = State::Disconnected;

	DataCallback dataCallback;
	CloseCallback closeCallback;
	ShutdownCallback shutdownCallback;
	ErrorCallback errorCallback;
	CloseCallback internalCloseCallback;
	ConnectCallback connectCallback;
};

} // namespace DNet