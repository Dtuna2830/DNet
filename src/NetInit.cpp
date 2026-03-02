#include "NetInit.h"
#include <exception>

namespace DNet
{

LPFN_ACCEPTEX NetInit::AcceptEx = nullptr;
LPFN_GETACCEPTEXSOCKADDRS NetInit::GetAcceptExSockaddrs = nullptr;
LPFN_CONNECTEX NetInit::ConnectEx = nullptr;
int NetInit::InitCount = 0;

void NetInit::Init()
{
	if (InitCount++ == 0)
	{
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(2, 2);
		int err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0)
		{
			std::terminate();
		}
		LoadExtensions();
	}
}

void NetInit::Clean()
{
	if (--InitCount == 0)
	{
		int err = WSACleanup();
		if (err != 0)
		{
			std::terminate();
		}
		AcceptEx = nullptr;
		GetAcceptExSockaddrs = nullptr;
	}
}

void NetInit::LoadExtensions()
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		std::terminate();
	}

	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;
	int err = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &AcceptEx,
					   sizeof(AcceptEx), &dwBytes, NULL, NULL);
	if (err == SOCKET_ERROR)
	{
		closesocket(s);
		std::terminate();
	}

	GUID GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	err = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockaddrs, sizeof(GuidGetAcceptExSockaddrs),
				   &GetAcceptExSockaddrs, sizeof(GetAcceptExSockaddrs), &dwBytes, NULL, NULL);
	if (err == SOCKET_ERROR)
	{
		closesocket(s);
		std::terminate();
	}

	GUID GuidConnectEx = WSAID_CONNECTEX;
	err = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx), &ConnectEx,
				   sizeof(ConnectEx), &dwBytes, NULL, NULL);
	if (err == SOCKET_ERROR)
	{
		closesocket(s);
		std::terminate();
	}

	closesocket(s);
}

} // namespace DNet