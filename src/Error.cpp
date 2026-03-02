#include "DNet/Error.h"

namespace DNet
{

Error::Error(ErrorCode code) : errorCode(code)
{
}

Error::Error(ErrorCode code, int nativeCode) : errorCode(code), nativeErrorCode(nativeCode)
{
}

bool Error::ok() const
{
	return errorCode == ErrorCode::Success;
}

ErrorCode Error::code() const
{
	return errorCode;
}

int Error::nativeCode() const
{
	return nativeErrorCode;
}

} // namespace DNet