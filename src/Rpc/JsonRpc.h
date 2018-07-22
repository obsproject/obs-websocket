#pragma once

#include <QJsonDocument>
#include <QString>

#include "RpcResponse.h"
#include "RpcRequest.h"
#include "RpcHandler.h"

class JsonRpc
{
public:
	JsonRpc(RpcHandler& handler);
	QString handleTextMessage(const QString& messageBody);
private:
	static const RpcRequest jsonToRequest(const QJsonDocument& requestBody);
	static QString responseToJson(const RpcResponse& response);

	static bool requestDocumentIsValid(const QJsonDocument& request);
	static QJsonObject errorToJson(const RpcError& error);
    static QString anonymousErrorResponse(const RpcError& error);

	RpcHandler& rpcHandler;
};