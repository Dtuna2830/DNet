#include "DNet/Error.h"
#include <cstring>

namespace DNet
{

Error::Error(int nativeCode)
{
	nativeErrorCode = nativeCode;
	switch (nativeCode)
	{
	case 0:
		errorCode = ErrorCode::Success;
		break;
	case EINVAL:
		errorCode = ErrorCode::InvalidArgument;
		break;
	case ENOTCONN:
		errorCode = ErrorCode::NotConnected;
		break;
	case EISCONN:
		errorCode = ErrorCode::AlreadyConnected;
		break;
	case ECONNRESET:
		errorCode = ErrorCode::ConnectionReset;
		break;
	case ECONNABORTED:
		errorCode = ErrorCode::ConnectionAborted;
		break;
	case ECONNREFUSED:
		errorCode = ErrorCode::ConnectionRefused;
		break;
	case EADDRINUSE:
		errorCode = ErrorCode::AddressInUse;
		break;
	case EADDRNOTAVAIL:
		errorCode = ErrorCode::AddressNotAvailable;
		break;
	case EAFNOSUPPORT:
		errorCode = ErrorCode::AddressFamilyNotSupported;
		break;
	case EACCES:
		errorCode = ErrorCode::PermissionDenied;
		break;
	case EBUSY:
		errorCode = ErrorCode::Busy;
		break;
	default:
		errorCode = ErrorCode::Undefined;
		break;
	}
}

std::string Error::message() const
{
	const char *msg = strerror(nativeErrorCode);
	return msg ? std::string(msg) : "Error message failed";
}

} // namespace DNet