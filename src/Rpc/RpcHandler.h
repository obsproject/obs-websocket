#pragma once

#include <QtCore/QObject>
#include <QtCore/QHash>
#include "RpcRequest.h"
#include "RpcResponse.h"
#include "RpcMethod.h"

class RpcHandler : public QObject {
  Q_OBJECT
  public:
    RpcHandler();
    ~RpcHandler();
    const RpcResponse processCall(const RpcRequest& request);
  private:
    QHash<QString, RpcMethod*> builtinMethods;
    QHash<QString, RpcMethod*> thirdPartyMethods;
};
