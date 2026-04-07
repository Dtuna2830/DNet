#pragma once
#include <functional>
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

struct Event;

#if DNET_WINDOWS
typedef std::function<void(DWORD bytesTransferred, Event *event)> EventCallback;
#elif DNET_LINUX
typedef std::function<void(int32_t bytesTransferred, Event *event)> EventCallback;
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
	EventCallback eventCallback;
	EventType type;
	Buffer buffer;
#if DNET_WINDOWS
	DWORD error;
	SOCKADDR_STORAGE addr;
#elif DNET_LINUX
	int error;
	sockaddr_storage addr;
	msghdr msg;
	iovec iov;
#endif
	int addrLen;

#if DNET_WINDOWS
	Event(EventType t) : eventCallback(nullptr), type(t), error(ERROR_SUCCESS), buffer(Buffer::STACK_SIZE), addrLen(0)
	{
		memset(this, 0, sizeof(OVERLAPPED));
	}
#elif DNET_LINUX
	Event(EventType t) : eventCallback(nullptr), type(t), error(0), buffer(Buffer::STACK_SIZE), addrLen(0)
	{
	}
#endif
	void reset(EventType t)
	{
#if DNET_WINDOWS
		memset(this, 0, sizeof(OVERLAPPED));
		error = ERROR_SUCCESS;
		memset(&addr, 0, sizeof(SOCKADDR_STORAGE));
#elif DNET_LINUX
		error = 0;
		memset(&addr, 0, sizeof(sockaddr_storage));
		memset(&msg, 0, sizeof(msghdr));
		memset(&iov, 0, sizeof(iovec));
#endif
		eventCallback = nullptr;
		type = t;
		buffer.resize(0);
		addrLen = 0;
	}
};

} // namespace DNet