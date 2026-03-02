#include <iostream>
#include <DNet/UdpSocket.h>

using namespace DNet;

int main()
{
	std::cout << "DNet UdpSocket" << std::endl;

	EventLoop loop;
	Endpoint localEndpoint = Endpoint::LoopbackV4(1234);
	UdpSocket socket(localEndpoint, loop);

	socket.onData(
		[](size_t length, const char *data, const Endpoint &endpoint)
		{
			std::cout << "Received data (" << length << " bytes) from " << endpoint.address().toString() << ":"
					  << endpoint.port() << ": " << std::string(data, length) << std::endl;
		});

	socket.onError(
		[](const Error &error)
		{
			std::cout << "Socket Error:\n"
					  << "Code: " << error.code() << "\nNative Code: " << error.nativeCode()
					  << "\nMessage: " << error.message() << std::endl;
		});

	Error err = socket.bind();
	if (!err.ok())
	{
		std::cout << "Failed to bind socket:\n"
				  << "Code: " << err.code() << "\nNative Code: " << err.nativeCode() << "\nMessage: " << err.message()
				  << std::endl;
		return 1;
	}

	Error recvErr = socket.startReceiving();
	if (!recvErr.ok())
	{
		std::cout << "Failed to start receiving:\n"
				  << "Code: " << recvErr.code() << "\nNative Code: " << recvErr.nativeCode()
				  << "\nMessage: " << recvErr.message() << std::endl;
		return 1;
	}

	Endpoint remoteEndpoint = Endpoint::LoopbackV4(1235);
	std::string msg = "Hello DNet!";
	Error sendErr = socket.send(msg.data(), msg.size(), remoteEndpoint);
	if (!sendErr.ok())
	{
		std::cout << "Failed to send data:\n"
				  << "Code: " << sendErr.code() << "\nNative Code: " << sendErr.nativeCode()
				  << "\nMessage: " << sendErr.message() << std::endl;
		return 1;
	}
	else
	{
		std::cout << "Sent data to " << remoteEndpoint.address().toString() << ":" << remoteEndpoint.port() << " "
				  << msg << std::endl;
	}

	loop.run();
	return 0;
}