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
		event->reset(type);
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
