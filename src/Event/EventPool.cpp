#include "DNet/Event/EventPool.h"

namespace DNet
{

EventPool::EventPool(size_t initialSize) : initSize(initialSize)
{
	events.reserve(initialSize);
	for (size_t i = 0; i < initialSize; ++i)
	{
		events.push_back(new Event(EventType::Read));
	}
}

EventPool::~EventPool()
{
	for (Event *event : events)
	{
		delete event;
	}
	events.clear();
}

Event *EventPool::allocateEvent(EventType type)
{
	Event *event = nullptr;
	if (!events.empty())
	{
		event = events.back();
		events.pop_back();
#if DNET_WINDOWS
		memset(event, 0, sizeof(OVERLAPPED));
		event->error = ERROR_SUCCESS;
		memset(&event->addr, 0, sizeof(SOCKADDR_STORAGE));
#elif DNET_LINUX
		event->error = 0;
		memset(&event->addr, 0, sizeof(sockaddr_storage));
		memset(&event->msg, 0, sizeof(msghdr));
		memset(&event->iov, 0, sizeof(iovec));
#endif
		event->eventCallback = nullptr;
		event->type = type;
		event->buffer.resize(0);
		event->addrLen = 0;
		return event;
	}
	event = new Event(type);
	return event;
}

void EventPool::deallocateEvent(Event *event)
{
	if (events.size() < initSize)
	{
		events.push_back(event);
	}
	else
	{
		delete event;
	}
}

} // namespace DNet
