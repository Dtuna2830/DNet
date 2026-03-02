#pragma once
#include "Address.h"

namespace DNet
{

class Endpoint
{
public:
	Endpoint();
	Endpoint(const Address &address, unsigned short port);
	Endpoint(const sockaddr *addr);

	const Address &address() const;
	unsigned short port() const;

	const sockaddr *get() const;
	socklen_t length() const;

	static Endpoint AnyV4(unsigned short port);
	static Endpoint AnyV6(unsigned short port);

	static Endpoint LoopbackV4(unsigned short port);
	static Endpoint LoopbackV6(unsigned short port);

private:
	void setSockAddr();

	Address dAddr;
	unsigned short portNum;

	sockaddr_in addr4;
	sockaddr_in6 addr6;
};

} // namespace DNet