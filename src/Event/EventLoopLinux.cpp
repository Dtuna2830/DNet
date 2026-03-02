#include "DNet/Event/EventLoop.h"
#include <exception>
#include "DNet/Event/EventHandler.h"

namespace DNet
{

EventLoop::EventLoop(size_t poolSize) : pool(poolSize)
{
	eventHandle = new io_uring();
	int res = io_uring_queue_init(poolSize, eventHandle, 0);
	if (res < 0)
	{
		delete eventHandle;
		eventHandle = nullptr;
		std::terminate();
	}
}

EventLoop::~EventLoop()
{
	stop();
	if (eventHandle)
	{
		io_uring_queue_exit(eventHandle);
		delete eventHandle;
		eventHandle = nullptr;
	}
}

void EventLoop::run()
{
	runs = true;
	while (runs)
	{
		io_uring_cqe *cqe = nullptr;
		int res = io_uring_wait_cqe(eventHandle, &cqe);
		if (res < 0)
		{
			if (runs == false) break;
			continue;
		}
		if (!runs)
		{
			io_uring_cqe_seen(eventHandle, cqe);
			break;
		}

		Event *event = reinterpret_cast<Event *>(io_uring_cqe_get_data(cqe));
		if (event)
		{
			EventHandler *handler = event->eventHandler;
			if (cqe->res < 0)
			{
				event->error = cqe->res;
				if (handler) handler->handleEvent(0, event);
			}
			else
			{
				if (handler) handler->handleEvent(cqe->res, event);
			}
		}
		io_uring_cqe_seen(eventHandle, cqe);
	}
}

void EventLoop::stop()
{
	if (!runs) return;
	runs = false;
	io_uring_sqe *sqe = io_uring_get_sqe(eventHandle);
	if (sqe)
	{
		io_uring_prep_nop(sqe);
		io_uring_sqe_set_data(sqe, nullptr); // send empty message to unblock thread
		io_uring_submit(eventHandle);
	}
}

} // namespace DNet