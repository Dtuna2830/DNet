#pragma once
#include <WinSock2.h>
#include <MSWSock.h>

namespace DNet
{

class NetInit
{
public:
	static void Init();
	static void Clean();

	static LPFN_ACCEPTEX AcceptEx;
	static LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockaddrs;
	static LPFN_CONNECTEX ConnectEx;

private:
	static void LoadExtensions();

	static int InitCount;
};

} // namespace DNet