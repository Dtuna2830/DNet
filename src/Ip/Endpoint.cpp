#include "DNet/Ip/Endpoint.h"
#if DNET_LINUX
#include <cstring>
#endif

namespace DNet
{

Endpoint::Endpoint()
{
	dAddr = Address::LoopbackV4();
	portNum = 0;
	setSockAddr();
}

Endpoint::Endpoint(const Address &address, unsigned short port)
{
	dAddr = address;
	portNum = port;
	setSockAddr();
}

Endpoint::Endpoint(const sockaddr *addr)
{
	if (addr->sa_family == AF_INET)
	{
		const sockaddr_in *addrIn = reinterpret_cast<const sockaddr_in *>(addr);
		dAddr = Address(addrIn->sin_addr);
		portNum = ntohs(addrIn->sin_port);
	}
	else if (addr->sa_family == AF_INET6)
	{
		const sockaddr_in6 *addrIn6 = reinterpret_cast<const sockaddr_in6 *>(addr);
		dAddr = Address(addrIn6->sin6_addr);
		portNum = ntohs(addrIn6->sin6_port);
	}
	setSockAddr();
}

const Address &Endpoint::address() const
{
	return dAddr;
}

unsigned short Endpoint::port() const
{
	return portNum;
}

const sockaddr *Endpoint::get() const
{
	if (dAddr.type() == AddressType::IPv4) return reinterpret_cast<const sockaddr *>(&addr4);
	if (dAddr.type() == AddressType::IPv6) return reinterpret_cast<const sockaddr *>(&addr6);
	return nullptr;
}

socklen_t Endpoint::length() const
{
	if (dAddr.type() == AddressType::IPv4) return sizeof(sockaddr_in);
	if (dAddr.type() == AddressType::IPv6) return sizeof(sockaddr_in6);
	return 0;
}

Endpoint Endpoint::AnyV4(unsigned short port)
{
	return Endpoint(Address::AnyV4(), port);
}

Endpoint Endpoint::AnyV6(unsigned short port)
{
	return Endpoint(Address::AnyV6(), port);
}

Endpoint Endpoint::LoopbackV4(unsigned short port)
{
	return Endpoint(Address::LoopbackV4(), port);
}

Endpoint Endpoint::LoopbackV6(unsigned short port)
{
	return Endpoint(Address::LoopbackV6(), port);
}

void Endpoint::setSockAddr()
{
	if (dAddr.type() == AddressType::IPv4)
	{
		memset(&addr4, 0, sizeof(addr4));
		addr4.sin_family = AF_INET;
		addr4.sin_port = htons(portNum);
		addr4.sin_addr = dAddr.get().ipv4;
	}
	else if (dAddr.type() == AddressType::IPv6)
	{
		memset(&addr6, 0, sizeof(addr6));
		addr6.sin6_family = AF_INET6;
		addr6.sin6_port = htons(portNum);
		addr6.sin6_addr = dAddr.get().ipv6;
	}
}

} // namespace DNet