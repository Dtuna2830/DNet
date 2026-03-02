#include "DNet/TcpClient.h"
#include "NetInit.h"

namespace DNet
{

void TcpClient::connect(std::function<void(std::shared_ptr<TcpConnection>, const Error &)> callback)
{
	if (connection && connection->state != State::Disconnected)
	{
		callback(nullptr, Error(ErrorCode::AlreadyConnected));
		return;
	}

	AddressType addrType = rEndpoint.address().type();
	Endpoint lEndpoint;
	if (addrType == AddressType::IPv4) lEndpoint = Endpoint::AnyV4(0);
	else if (addrType == AddressType::IPv6) lEndpoint = Endpoint::AnyV6(0);
	else
	{
		callback(nullptr, Error(ErrorCode::InvalidArgument));
		return;
	}

	int af = rEndpoint.address().adressFamily();

	SOCKET sock = WSASocket(af, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (sock == INVALID_SOCKET)
	{
		callback(nullptr, Error(WSAGetLastError()));
		return;
	}

	const sockaddr *addr = lEndpoint.get();
	socklen_t addrLen = lEndpoint.length();

	int bindResult = bind(sock, addr, addrLen);
	if (bindResult == SOCKET_ERROR)
	{
		closesocket(sock);
		callback(nullptr, Error(WSAGetLastError()));
		return;
	}

	connection = std::make_shared<TcpConnection>(sock, rEndpoint, eventLoop);
	connection->state = State::Connecting;
	eventLoop.registerHandle(reinterpret_cast<HANDLE>(sock), reinterpret_cast<ULONG_PTR>(connection.get()));

	Event *event = eventLoop.getEventPool().allocateEvent(EventType::Connect);
	DWORD bytesSent = 0;
	BOOL result = NetInit::ConnectEx(sock, rEndpoint.get(), rEndpoint.length(), nullptr, NULL, &bytesSent, event);
	if (result == FALSE)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING)
		{
			closesocket(sock);
			connection->state = State::Disconnected;
			eventLoop.getEventPool().deallocateEvent(event);
			callback(nullptr, Error(err));
			return;
		}
	}
	callback(connection, Error());
}

} // namespace DNet