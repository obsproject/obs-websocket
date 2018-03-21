#pragma once

#include <QJsonDocument>
#include <QWebSocket>
#include <QString>

#include "RpcResponse.h"
#include "RpcRequest.h"
#include "RpcHandler.h"

class JsonRpc {
public:
	JsonRpc(RpcHandler& handler);
	void handleTextMessage(QWebSocket* client, QString& messageBody);
private:
	static const RpcRequest jsonToRequest(const QJsonDocument& requestBody);
	static QJsonDocument responseToJson(const RpcResponse& response);

	static bool requestDocumentIsValid(const QJsonDocument& request);
	static QJsonObject errorToJson(const RpcError& error);
    static QJsonDocument anonymousError(const RpcError& error);
	static void sendToClient(QWebSocket* client, const QJsonDocument& document);

	RpcHandler& rpcHandler;
};