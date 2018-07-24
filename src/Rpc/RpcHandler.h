#pragma once

#include <QtCore/QObject>
#include <QtCore/QHash>

#include "RpcRequest.h"
#include "RpcResponse.h"
#include "RpcMethod.h"

typedef QHash<QString, RpcMethod*> QRpcMethodHash;

class RpcHandler : public QObject
{
Q_OBJECT

public:
	RpcHandler();
	~RpcHandler();
	const RpcResponse processCall(const RpcRequest& request);

private:
	QRpcMethodHash _builtinMethods;
	QRpcMethodHash _thirdPartyMethods;
};
