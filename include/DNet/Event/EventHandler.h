#pragma once
#include <cstdint>
#include "Event.h"

namespace DNet
{

class EventHandler
{
public:
#if DNET_WINDOWS
	virtual void handleEvent(DWORD bytesTransferred, Event *event) = 0;
#elif DNET_LINUX
	virtual void handleEvent(int32_t bytesTransferred, Event *event) = 0;
#endif
};

} // namespace DNet