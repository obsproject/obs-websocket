#include "RpcRequest.h"

RpcRequest::RpcRequest(const QString &id, const QString &methodName,
    const QVariantHash& params)
        : id(id),
          methodName(methodName),
          parameters(params)
{
}

const QVariantHash RpcRequest::immutableParams() const
{
    return QVariantHash(parameters);
}

const QString& RpcRequest::getId() const
{
    return this->id;
}

const QString& RpcRequest::getMethodName() const
{
    return this->methodName;
}

const QVariant& RpcRequest::param(QString name) const
{
    return this->parameters.value(name);
}
