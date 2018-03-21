#include "RpcResponse.h"

RpcResponse::RpcResponse(const QString& id, const QString& methodName)
    : id(id),
      methodName(methodName),
      error(QString::Null())
{
}

RpcResponse RpcResponse::ofRequest(const RpcRequest& request)
{
    return RpcResponse(request.getId(), request.getMethodName());
}

const RpcResponse RpcResponse::ok(const RpcRequest& request, const QVariant& result)
{
    RpcResponse response = ofRequest(request);
    response.result = result;
    response.error = RpcError();
    return response;
}

const RpcResponse RpcResponse::fail(const RpcRequest& request,
                                    const QString& errorMessage,
                                    int errorCode)
{
    RpcResponse response = ofRequest(request);
    response.result = QVariant();
    response.error = RpcError(errorMessage, errorCode);
    return response;
}

const QString& RpcResponse::getId() const
{
    return id;
}

const QString& RpcResponse::getMethodName() const
{
    return methodName;
}

const QVariant& RpcResponse::getResult() const
{
    return result;
}

const RpcError& RpcResponse::getError() const
{
    return error;
}
