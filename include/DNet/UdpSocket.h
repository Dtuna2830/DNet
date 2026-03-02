#pragma once
#include <functional>
#include "Event/EventHandler.h"
#include "Event/EventLoop.h"
#include "Ip/Endpoint.h"
#include "Error.h"

namespace DNet
{

typedef std::function<void(size_t, const char *, const Endpoint &)> DataCallback;
typedef std::function<void(const Error &)> ErrorCallback;

class UdpSocket : public EventHandler
{
public:
	UdpSocket(Endpoint &localEndpoint, EventLoop &loop, size_t receiveSize = 65536);
	~UdpSocket();

	Error bind();
	Error startReceiving();
	Error send(const char *data, size_t length, const Endpoint &endpoint);
	Error close();
	void setReceiveSize(size_t size);

#if DNET_WINDOWS
	void handleEvent(DWORD bytesTransferred, Event *event) override;
#elif DNET_LINUX
	void handleEvent(int32_t bytesTransferred, Event *event) override;
#endif

	void onData(DataCallback callback);
	void onError(ErrorCallback callback);

private:
#if DNET_WINDOWS
	SOCKET sock = INVALID_SOCKET;
#elif DNET_LINUX
	int sock = -1;
#endif
	Endpoint lEndpoint;
	EventLoop &eventLoop;
	size_t recvSize;

	DataCallback dataCallback;
	ErrorCallback errorCallback;
};

} // namespace DNet