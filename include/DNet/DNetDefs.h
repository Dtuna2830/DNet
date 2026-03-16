#pragma once

#if DNET_WINDOWS
#include <WinSock2.h>
#elif DNET_LINUX
#include <liburing.h>
#endif

namespace DNet
{
#if DNET_WINDOWS
typedef HANDLE EventHandle;
typedef SOCKET SocketHandle;
constexpr SocketHandle InvalidSocket = INVALID_SOCKET;
#elif DNET_LINUX
typedef io_uring *EventHandle;
typedef int SocketHandle;
constexpr SocketHandle InvalidSocket = -1;
#endif
} // namespace DNet