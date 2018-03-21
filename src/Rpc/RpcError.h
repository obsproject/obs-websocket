#pragma once

#include <QtCore/QString>

enum RpcErrorCode
{
    Invalid = 0,
    ParseError = -32603,
    InvalidRequest = -32600,
    MethodNotFound = -32601,
    InvalidParams = -32602,
    InternalError = -32603
};

class RpcError
{
    public:
        RpcError();
        RpcError(const QString& message, int code = RpcErrorCode::InternalError);

        bool isValid() const;
        const QString& getMessage() const;
        const int getCode() const;
    private:
        QString message;
        int code;
};
