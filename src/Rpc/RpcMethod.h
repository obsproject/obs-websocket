#pragma once

#include "RpcRequest.h"
#include "RpcResponse.h"

class RpcMethod
{
public:
	virtual const RpcResponse handle(const RpcRequest& request) = 0;
};
