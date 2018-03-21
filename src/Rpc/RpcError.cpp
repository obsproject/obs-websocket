#include "RpcError.h"

// Default constructor for invalid errors
RpcError::RpcError()
        : message(), code(RpcErrorCode::Invalid)
{
}

RpcError::RpcError(const QString& message, int code)
        : message(message), code(code)
{
}

bool RpcError::isValid() const
{
    return ((!this->message.isNull()) && (this->code != 0));
}

const QString& RpcError::getMessage() const
{
    return this->message;
}

const int RpcError::getCode() const
{
    return this->code;
}
