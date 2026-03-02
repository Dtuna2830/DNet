#pragma once
#include "../DNetDefs.h"
#include "EventPool.h"

namespace DNet
{

class EventLoop
{
public:
	EventLoop(size_t poolSize = 256);
	~EventLoop();

	void run();
	void stop();

#if DNET_WINDOWS
	void registerHandle(EventHandle handle, ULONG_PTR key);
#endif
	EventHandle getEventHandle();
	EventPool &getEventPool();

private:
	EventHandle eventHandle;
	bool runs = false;
	EventPool pool;
};

} // namespace DNet