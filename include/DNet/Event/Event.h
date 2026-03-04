#pragma once
#include "../Buffer.h"

#if DNET_WINDOWS
#include <WinSock2.h>
// clang-format off
#define DNET_EVENT_EX : public OVERLAPPED
// clang-format on
#elif DNET_LINUX
#include <sys/socket.h>
#define DNET_EVENT_EX
#endif

namespace DNet
{
#if DNET_LINUX
class EventHandler;
#endif

enum EventType
{
	Read,
	Write,
	Accept,
	Connect
};

struct Event DNET_EVENT_EX
{
	EventType type;
	Buffer buffer;
#if DNET_WINDOWS
	DWORD error;
	HANDLE handle;
	SOCKADDR_STORAGE addr;
#elif DNET_LINUX
	EventHandler *eventHandler;
	int error;
	sockaddr_storage addr;
	msghdr msg;
	iovec iov;
#endif
	int addrLen;

#if DNET_WINDOWS
	Event(EventType t) : type(t), error(ERROR_SUCCESS), buffer(Buffer::STACK_SIZE), handle(NULL), addrLen(0)
	{
		memset(this, 0, sizeof(OVERLAPPED));
	}
#elif DNET_LINUX
	Event(EventType t) : type(t), error(0), buffer(Buffer::STACK_SIZE), addrLen(0), eventHandler(nullptr)
	{
	}
#endif
};

} // namespace DNet