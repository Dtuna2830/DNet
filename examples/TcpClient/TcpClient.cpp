#include <iostream>
#include <DNet/TcpClient.h>

using namespace DNet;

int main()
{
	std::cout << "DNet TcpClient" << std::endl;

	EventLoop loop;
	Endpoint serverEndpoint = Endpoint::LoopbackV4(1234);
	TcpClient client(serverEndpoint, loop);

	client.connect(
		[](std::shared_ptr<TcpConnection> conn, const Error &error)
		{
			if (!error.ok())
			{
				std::cout << "Failed to connect:\n"
						  << "Code: " << error.code() << "\nNative Code: " << error.nativeCode()
						  << "\nMessage: " << error.message() << std::endl;
				return;
			}

			conn->onConnect(
				[conn]()
				{
					std::cout << "Connected to server at " << conn->remoteEndpoint().address().toString() << ":"
							  << conn->remoteEndpoint().port() << std::endl;
				});

			conn->onData(
				[conn](size_t length, const char *data)
				{
					std::cout << "Received data (" << length << " bytes): " << std::string(data, length) << std::endl;
				});

			conn->onClose(
				[conn]()
				{
					std::cout << "Connection closed from " << conn->remoteEndpoint().address().toString() << ":"
							  << conn->remoteEndpoint().port() << std::endl;
				});

			conn->onShutdown(
				[conn]()
				{
					std::cout << "Shutdown received from " << conn->remoteEndpoint().address().toString() << ":"
							  << conn->remoteEndpoint().port() << std::endl;
				});

			conn->onError(
				[](const Error &error)
				{
					std::cout << "Connection Error:\n"
							  << "Code: " << error.code() << "\nNative Code: " << error.nativeCode()
							  << "\nMessage: " << error.message() << std::endl;
				});
		});

	loop.run();
	return 0;
}