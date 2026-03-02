#include "DNet/TcpClient.h"

namespace DNet
{

void TcpClient::connect(std::function<void(std::shared_ptr<TcpConnection>, const Error &)> callback)
{
	if (connection && connection->state != State::Disconnected)
	{
		callback(nullptr, Error(ErrorCode::AlreadyConnected));
		return;
	}

	const int af = rEndpoint.address().adressFamily();
	if (af == AF_UNSPEC)
	{
		callback(nullptr, Error(ErrorCode::InvalidArgument));
		return;
	}

	int sock = socket(af, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
	{
		callback(nullptr, Error(errno));
		return;
	}

	connection = std::make_shared<TcpConnection>(sock, rEndpoint, eventLoop);
	connection->state = State::Connecting;

	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Connect);
	event->eventHandler = connection.get();

	io_uring_sqe *sqe = io_uring_get_sqe(eventLoop.getEventHandle());
	if (!sqe)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		close(sock);
		connection->state = State::Disconnected;
		callback(nullptr, Error(EBUSY));
		return;
	}

	io_uring_prep_connect(sqe, sock, rEndpoint.get(), rEndpoint.length());
	io_uring_sqe_set_data(sqe, event);

	int result = io_uring_submit(eventLoop.getEventHandle());
	if (result < 0)
	{
		eventLoop.getEventPool().deallocateEvent(event);
		close(sock);
		connection->state = State::Disconnected;
		callback(nullptr, Error(result));
		return;
	}
	callback(connection, Error());
}

} // namespace DNet