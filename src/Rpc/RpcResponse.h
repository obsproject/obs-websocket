#pragma once

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QHash>
#include "RpcRequest.h"
#include "RpcError.h"

class RpcResponse {
  public:
    static RpcResponse ofRequest(const RpcRequest& request);
    static const RpcResponse ok(const RpcRequest& request, const QVariant& result = QVariant());
    static const RpcResponse fail(
            const RpcRequest& request,
            const QString& errorMessage,
            int errorCode = RpcErrorCode::InternalError
    );

    RpcResponse(const QString& id, const QString& methodName);

    const QString& getId() const;
    const QString& getMethodName() const;
    const QVariant& getResult() const;
    const RpcError& getError() const;
  private:
    QString id;
    RpcError error;
    QString methodName;
    QVariant result;
};
