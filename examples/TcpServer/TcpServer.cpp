#include <iostream>
#include <DNet/TcpServer.h>

using namespace DNet;

int main()
{
	std::cout << "DNet TcpServer" << std::endl;

	EventLoop loop;
	Endpoint listenEndpoint = Endpoint::LoopbackV4(1234);
	TcpServer server(listenEndpoint, loop);

	server.onConnection(
		[](std::shared_ptr<TcpConnection> conn)
		{
			std::cout << "New connection from " << conn->remoteEndpoint().address().toString() << ":"
					  << conn->remoteEndpoint().port() << std::endl;
			conn->onData(
				[](size_t length, const char *data)
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

	server.onError(
		[](const Error &error)
		{
			std::cout << "Server Error:\n"
					  << "Code: " << error.code() << "\nNative Code: " << error.nativeCode()
					  << "\nMessage: " << error.message() << std::endl;
		});

	Error startError = server.start();
	if (!startError.ok())
	{
		std::cout << "Failed to start server:\n"
				  << "Code: " << startError.code() << "\nNative Code: " << startError.nativeCode()
				  << "\nMessage: " << startError.message() << std::endl;
		return -1;
	}

	loop.run();
	return 0;
}