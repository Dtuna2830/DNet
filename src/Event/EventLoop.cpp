#include "DNet/Event/EventLoop.h"

namespace DNet
{

EventHandle EventLoop::getEventHandle()
{
	return eventHandle;
}

EventPool &EventLoop::getEventPool()
{
	return pool;
}

} // namespace DNet