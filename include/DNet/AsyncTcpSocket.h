#pragma once
#include <functional>
#include "Event/EventLoop.h"
#include "Ip/Endpoint.h"
#include "SocketOption.h"

namespace DNet
{

class AsyncTcpSocket
{
public:
	explicit AsyncTcpSocket(EventLoop &loop);
	~AsyncTcpSocket();

	AsyncTcpSocket(const AsyncTcpSocket &) = delete;
	AsyncTcpSocket &operator=(const AsyncTcpSocket &) = delete;
	AsyncTcpSocket(AsyncTcpSocket &&o) noexcept;
	AsyncTcpSocket &operator=(AsyncTcpSocket &&o) noexcept;

	enum ShutdownType
	{
		Recv,
		Send,
		Both
	};

	Error open(AddressType addrType);
	Error bind(const Endpoint &endpoint);
	Error listen(int backlog = SOMAXCONN);
	Error close();
	Error shutdown(ShutdownType type);
	bool isClosed();
	template <typename T>
	Error setOption(const SocketOption<T> &option);

	typedef std::function<void(const Error &)> ConnectCallback;
	typedef std::function<void(const Error &, const Endpoint &)> AcceptCallback;
	typedef std::function<void(const Error &, const char *, size_t)> RecvCallback;
	typedef std::function<void(const Error &, size_t)> SendCallback;

	Error asyncConnect(const Endpoint &endpoint, ConnectCallback callback);
	Error asyncAccept(AsyncTcpSocket &acceptSocket, AcceptCallback callback);
	Error asyncRecv(RecvCallback callback);
	Error asyncRecv(size_t size, RecvCallback callback);
	Error asyncSend(const char *data, size_t size, SendCallback callback);

private:
	SocketHandle socketHandle;
	EventLoop &eventLoop;
};

template <typename T>
Error AsyncTcpSocket::setOption(const SocketOption<T> &option)
{
	return option.set(socketHandle);
}

}