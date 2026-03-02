#include "DNet/Error.h"
#include <Windows.h>

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
	case WSAEINVAL:
		errorCode = ErrorCode::InvalidArgument;
		break;
	case WSAENOTCONN:
		errorCode = ErrorCode::NotConnected;
		break;
	case WSAEISCONN:
		errorCode = ErrorCode::AlreadyConnected;
		break;
	case WSAECONNRESET:
		errorCode = ErrorCode::ConnectionReset;
		break;
	case WSAECONNABORTED:
		errorCode = ErrorCode::ConnectionAborted;
		break;
	case WSAECONNREFUSED:
		errorCode = ErrorCode::ConnectionRefused;
		break;
	case WSAEADDRINUSE:
		errorCode = ErrorCode::AddressInUse;
		break;
	case WSAEADDRNOTAVAIL:
		errorCode = ErrorCode::AddressNotAvailable;
		break;
	case WSAEAFNOSUPPORT:
		errorCode = ErrorCode::AddressFamilyNotSupported;
		break;
	case WSAEACCES:
		errorCode = ErrorCode::PermissionDenied;
		break;
	default:
		errorCode = ErrorCode::Undefined;
		break;
	}
}

std::string Error::message() const
{
	LPVOID lpMsgBuf;
	DWORD len =
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
					  nativeErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	if (len == 0)
	{
		return "Error message failed";
	}

	std::string message((LPTSTR)lpMsgBuf, len);
	LocalFree(lpMsgBuf);
	return message;
}

} // namespace DNet