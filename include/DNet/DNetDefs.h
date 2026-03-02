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
#elif DNET_LINUX
typedef io_uring *EventHandle;
#endif
} // namespace DNet