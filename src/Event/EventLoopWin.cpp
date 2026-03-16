#include "DNet/Event/EventLoop.h"
#include "../NetInit.h"
#include <stdexcept>

namespace DNet
{

EventLoop::EventLoop(size_t poolSize) : pool(poolSize)
{
	NetInit::Init();
	eventHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (eventHandle == NULL)
	{
		throw std::runtime_error("CreateIoCompletionPort failed");
	}
}

EventLoop::~EventLoop()
{
	stop();
	if (eventHandle) CloseHandle(eventHandle);
	NetInit::Clean();
}

void EventLoop::run()
{
	runs = true;
	while (runs)
	{
		DWORD bytesTransferred = 0;
		ULONG_PTR completionKey = 0;
		LPOVERLAPPED overlapped = NULL;
		BOOL ok = GetQueuedCompletionStatus(eventHandle, &bytesTransferred, &completionKey, &overlapped, INFINITE);
		if (runs == false) break; // unblock thread for stopping
		Event *event = reinterpret_cast<Event *>(overlapped);
		if (ok == FALSE)
		{
			DWORD err = GetLastError();
			if (err != WAIT_TIMEOUT) // actually no need because INFINITE was used
			{
				if (event && event->eventCallback)
				{
					event->error = err;
					event->eventCallback(0, event);
				}
			}
			continue;
		}
		if (event && event->eventCallback) event->eventCallback(bytesTransferred, event);
	}
}

void EventLoop::stop()
{
	if (!runs) return;
	runs = false;
	PostQueuedCompletionStatus(eventHandle, 0, 0, NULL); // send empty message to unblock thread
}

Error EventLoop::registerHandle(EventHandle handle, ULONG_PTR key)
{
	HANDLE result = CreateIoCompletionPort(handle, eventHandle, key, 0);
	if (result == NULL)
	{
		DWORD err = GetLastError();
		return Error(err);
	}
	return Error();
}

} // namespace DNet