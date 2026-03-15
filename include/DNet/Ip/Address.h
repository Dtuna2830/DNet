#pragma once
#if DNET_WINDOWS
#include <WS2tcpip.h>
#elif DNET_LINUX
#include <arpa/inet.h>
#include <netinet/in.h>
#endif
#include <string>

namespace DNet
{

enum AddressType
{
	IPv4,
	IPv6,
	None
};

union Addr
{
	in_addr ipv4;
	in6_addr ipv6;
};

class Address
{
public:
	Address();
	explicit Address(const std::string &address);
	Address(const in_addr &v4);
	Address(const in6_addr &v6);

	bool isValid() const;
	AddressType type() const;
	int addressFamily() const;
	std::string toString() const;
	const Addr &get() const;

	static Address AnyV4();
	static Address AnyV6();

	static Address LoopbackV4();
	static Address LoopbackV6();

private:
	Addr addr;
	AddressType addrType;
};

} // namespace DNet