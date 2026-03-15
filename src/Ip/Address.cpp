#include "DNet/Ip/Address.h"

namespace DNet
{

Address::Address() : addr{}, addrType(AddressType::None)
{
}

Address::Address(const std::string &address) : addr{}, addrType(AddressType::None)
{
	in_addr v4{};
	in6_addr v6{};
	if (inet_pton(AF_INET, address.c_str(), &v4) == 1) addr.ipv4 = v4, addrType = AddressType::IPv4;
	else if (inet_pton(AF_INET6, address.c_str(), &v6) == 1) addr.ipv6 = v6, addrType = AddressType::IPv6;
	else addrType = AddressType::None;
}

Address::Address(const in_addr &v4) : addr{}, addrType(AddressType::IPv4)
{
	addr.ipv4 = v4;
	addrType = AddressType::IPv4;
}

Address::Address(const in6_addr &v6) : addr{}, addrType(AddressType::IPv6)
{
	addr.ipv6 = v6;
	addrType = AddressType::IPv6;
}

bool Address::isValid() const
{
	return addrType != AddressType::None;
}

AddressType Address::type() const
{
	return addrType;
}

int Address::addressFamily() const
{
	if (addrType == AddressType::IPv4) return AF_INET;
	else if (addrType == AddressType::IPv6) return AF_INET6;
	else return AF_UNSPEC;
}

std::string Address::toString() const
{
	if (addrType == AddressType::IPv4)
	{
		char buffer[INET_ADDRSTRLEN];
		if (inet_ntop(AF_INET, &addr.ipv4, buffer, sizeof(buffer))) return buffer;
	}
	else if (addrType == AddressType::IPv6)
	{
		char buffer[INET6_ADDRSTRLEN];
		if (inet_ntop(AF_INET6, &addr.ipv6, buffer, sizeof(buffer))) return buffer;
	}
	return "";
}

const Addr &Address::get() const
{
	return addr;
}

Address Address::AnyV4()
{
	Address a;
	a.addrType = AddressType::IPv4;
#if DNET_WINDOWS
	a.addr.ipv4.S_un.S_addr = INADDR_ANY;
#elif DNET_LINUX
	a.addr.ipv4.s_addr = INADDR_ANY;
#endif
	return a;
}

Address Address::AnyV6()
{
	Address a;
	a.addrType = AddressType::IPv6;
	a.addr.ipv6 = IN6ADDR_ANY_INIT;
	return a;
}

Address Address::LoopbackV4()
{
	Address a;
	a.addrType = AddressType::IPv4;
#if DNET_WINDOWS
	a.addr.ipv4.S_un.S_addr = htonl(INADDR_LOOPBACK);
#elif DNET_LINUX
	a.addr.ipv4.s_addr = htonl(INADDR_LOOPBACK);
#endif
	return a;
}

Address Address::LoopbackV6()
{
	Address a;
	a.addrType = AddressType::IPv6;
	a.addr.ipv6 = IN6ADDR_LOOPBACK_INIT;
	return a;
}

int Address::AddressFamily(AddressType type)
{
	switch (type)
	{
	case AddressType::IPv4:
		return AF_INET;
	case AddressType::IPv6:
		return AF_INET6;
	default:
		return AF_UNSPEC;
	}
}

} // namespace DNet