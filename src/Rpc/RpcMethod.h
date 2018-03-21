#pragma once

#include "RpcRequest.h"
#include "RpcResponse.h"

#define RPCMETHOD_BOILERPLATE(x) class x : public RpcMethod {public: const RpcResponse handle(const RpcRequest& request); };

class RpcMethod {
  public:
    virtual const RpcResponse handle(const RpcRequest& request) = 0;
};
