#pragma once
#include <string>

namespace DNet
{

enum ErrorCode
{
	Success,
	InvalidArgument,
	NotConnected,
	AlreadyConnected,
	ConnectionReset,
	ConnectionAborted,
	ConnectionRefused,
	AddressInUse,
	AddressNotAvailable,
	AddressFamilyNotSupported,
	PermissionDenied,
	Busy,
	Undefined,
};

class Error
{
public:
	Error() = default;
	Error(ErrorCode code);
	Error(ErrorCode code, int nativeCode);
	Error(int nativeCode);

	bool ok() const;
	std::string message() const;
	ErrorCode code() const;
	int nativeCode() const;

private:
	ErrorCode errorCode = ErrorCode::Success;
	int nativeErrorCode = 0;
};

} // namespace DNet