#pragma once
#include "DNetDefs.h"
#include "Error.h"
#if DNET_LINUX
#include <netinet/tcp.h>
#endif

namespace DNet
{

// https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
template <typename T>
struct SocketOption
{
	Error setOpt(SocketHandle socket) const
	{
		return static_cast<const T *>(this)->setOpt(socket);
	}
};

struct ReuseAddress : public SocketOption<ReuseAddress>
{
	bool value;
	explicit ReuseAddress(bool v) : value(v)
	{
	}
	Error setOpt(SocketHandle socket) const
	{
		int opt = value ? 1 : 0;
		int optResult;
#if DNET_WINDOWS
		optResult = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&opt), sizeof(opt));
		if (optResult == SOCKET_ERROR) return Error(WSAGetLastError());
#elif DNET_LINUX
		optResult = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&opt), sizeof(opt));
		if (optResult < 0) return Error(errno);
#endif
		return Error();
	}
};

struct ReusePort : public SocketOption<ReusePort>
{
	bool value;
	explicit ReusePort(bool v) : value(v)
	{
	}
	Error setOpt(SocketHandle socket) const
	{
#if DNET_LINUX
		int opt = value ? 1 : 0;
		int optResult = setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<const char *>(&opt), sizeof(opt));
		if (optResult < 0) return Error(errno);
#endif
		return Error();
	}
};

struct KeepAlive : public SocketOption<KeepAlive>
{
	bool value;
	explicit KeepAlive(bool v) : value(v)
	{
	}
	Error setOpt(SocketHandle socket) const
	{
		int opt = value ? 1 : 0;
		int optResult;
#if DNET_WINDOWS
		optResult = setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char *>(&opt), sizeof(opt));
		if (optResult == SOCKET_ERROR) return Error(WSAGetLastError());
#elif DNET_LINUX
		optResult = setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char *>(&opt), sizeof(opt));
		if (optResult < 0) return Error(errno);
#endif
		return Error();
	}
};

struct TcpNoDelay : public SocketOption<TcpNoDelay>
{
	bool value;
	explicit TcpNoDelay(bool v) : value(v)
	{
	}
	Error setOpt(SocketHandle socket) const
	{
		int opt = value ? 1 : 0;
		int optResult;
#if DNET_WINDOWS
		optResult = setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&opt), sizeof(opt));
		if (optResult == SOCKET_ERROR) return Error(WSAGetLastError());
#elif DNET_LINUX
		optResult = setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&opt), sizeof(opt));
		if (optResult < 0) return Error(errno);
#endif
		return Error();
	}
};

struct ExclusiveAddress : public SocketOption<ExclusiveAddress>
{
	bool value;
	explicit ExclusiveAddress(bool v) : value(v)
	{
	}
	Error setOpt(SocketHandle socket) const
	{
#if DNET_WINDOWS
		int opt = value ? 1 : 0;
		int optResult =
			setsockopt(socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char *>(&opt), sizeof(opt));
		if (optResult == SOCKET_ERROR) return Error(WSAGetLastError());
#endif
		return Error();
	}
};

} // namespace DNet