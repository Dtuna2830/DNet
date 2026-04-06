#pragma once
#include <functional>
#include "Event/EventLoop.h"
#include "Ip/Endpoint.h"
#include "SocketOption.h"

namespace DNet
{

class AsyncUdpSocket
{
public:
	explicit AsyncUdpSocket(EventLoop &loop);
	~AsyncUdpSocket();

	AsyncUdpSocket(const AsyncUdpSocket &) = delete;
	AsyncUdpSocket &operator=(const AsyncUdpSocket &) = delete;
	AsyncUdpSocket(AsyncUdpSocket &&o) noexcept;
	AsyncUdpSocket &operator=(AsyncUdpSocket &&o) noexcept;

	Error open(AddressType addrType);
	Error bind(const Endpoint &endpoint);
	Error close();
	bool isClosed();
	template <typename T>
	Error setOption(const SocketOption<T> &option);

	typedef std::function<void(const Error &, const char *, size_t, const Endpoint &)> RecvCallback;
	typedef std::function<void(const Error &, size_t)> SendCallback;

	Error asyncRecv(RecvCallback callback);
	Error asyncRecv(size_t size, RecvCallback callback);
	Error asyncSend(const char *data, size_t size, const Endpoint &endpoint, SendCallback callback);

private:
	SocketHandle socketHandle;
	EventLoop &eventLoop;
};

template <typename T>
Error AsyncUdpSocket::setOption(const SocketOption<T> &option)
{
	return option.set(socketHandle);
}

} // namespace DNet