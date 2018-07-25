#pragma once

#include "src/Rpc/RpcMethod.h"
#include "src/Utils.h"
#include "src/obs-websocket.h"

#define RPCMETHOD_BOILERPLATE(x) class x : public RpcMethod {public: const RpcResponse handle(const RpcRequest& request); };

#define RPCMETHOD_HANDLER(x) const RpcResponse x ## ::handle(const RpcRequest &request)

RPCMETHOD_BOILERPLATE(GetVersion)
