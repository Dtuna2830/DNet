#pragma once
#include <vector>
#include "Event.h"

namespace DNet
{

class EventPool
{
public:
	EventPool(size_t initialSize);
	~EventPool();

	Event *allocateEvent(EventType type);
	void deallocateEvent(Event *event);

private:
	std::vector<Event *> events;
};

} // namespace DNet