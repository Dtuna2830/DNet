#include <iostream>
#include <DNet/AsyncUdpSocket.h>

using namespace DNet;

int main()
{
	std::cout << "DNet UdpSocket" << std::endl;

	try
	{
		EventLoop loop;
		Endpoint localEndpoint = Endpoint::LoopbackV4(1234);
		AsyncUdpSocket socket(loop);

		Error err = socket.open(localEndpoint.address().type());
		if (!err.ok())
		{
			std::cout << "Failed to open socket: " << err.message() << std::endl;
			return 1;
		}

		err = socket.bind(localEndpoint);
		if (!err.ok())
		{
			std::cout << "Failed to bind socket: " << err.message() << std::endl;
			return 1;
		}

		std::function<void()> recv;
		recv = [&]()
		{
			err = socket.asyncRecv([&](const Error &recvErr, const char *data, size_t length, const Endpoint &remote)
			{
				if (!recvErr.ok())
				{
					std::cout << "Failed to async receive: " << recvErr.message() << std::endl;
					return;
				}

				std::cout << "Received data (" << length << " bytes) from " << remote.address().toString() << ":"
						  << remote.port() << ": " << std::string(data, length) << std::endl;
				recv();
			});

			if (!err.ok())
			{
				std::cout << "Failed to receive: " << err.message() << std::endl;
			}
		};
		recv();

		Endpoint remoteEndpoint = Endpoint::LoopbackV4(1235);
		std::string msg = "Hello DNet!";
		err = socket.asyncSend(msg.data(), msg.size(), remoteEndpoint, [&](const Error &sendErr, size_t)
		{
			if (!sendErr.ok())
			{
				std::cout << "Failed to async send: " << sendErr.message() << std::endl;
				return;
			}

			std::cout << "Sent data to " << remoteEndpoint.address().toString() << ":" << remoteEndpoint.port() << " "
					  << msg << std::endl;
		});
		if (!err.ok())
		{
			std::cout << "Failed to send: " << err.message() << std::endl;
		}

		loop.run();
	}
	catch (std::exception &e)
	{
		std::cout << "Fatal error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}