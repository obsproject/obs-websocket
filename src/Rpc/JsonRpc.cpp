#include "JsonRpc.h"

#include <QByteArray>
#include <QJsonObject>

JsonRpc::JsonRpc(RpcHandler& handler)
	: rpcHandler(handler)
{
}

void JsonRpc::handleTextMessage(QWebSocket* client, QString& messageBody)
{
	QJsonParseError parseError;
	QJsonDocument requestJson =
		QJsonDocument::fromJson(messageBody.toUtf8(), &parseError);
	if (parseError.error != QJsonParseError::NoError) {
		QString errorMsg =
			QString("Parse error: %1").arg(parseError.errorString());
		sendToClient(client, anonymousError(
				RpcError(errorMsg, RpcErrorCode::ParseError)));
		return;
	}

	if (!requestDocumentIsValid(requestJson)) {
		sendToClient(client, anonymousError(
				RpcError("Invalid Request object",
						 RpcErrorCode::InvalidRequest)));
		return;
	}

	const RpcRequest request = jsonToRequest(requestJson);
	const RpcResponse response = rpcHandler.processCall(request);
	sendToClient(client, responseToJson(response));
}

const RpcRequest JsonRpc::jsonToRequest(const QJsonDocument& requestBody)
{
	QJsonObject obj = requestBody.object();

	QString id = obj.value("id").toString();
	QString method = obj.value("method").toString();
	QJsonValue params = obj.value("params");

	QHash<QString, QVariant> parameters;
	if (params.isObject()) {
		QJsonObject paramsObject = params.toObject();
		parameters = paramsObject.toVariantHash();
	}

	return RpcRequest(id, method, parameters);
}

QJsonDocument JsonRpc::responseToJson(const RpcResponse& response)
{
	QJsonObject obj;
	obj.insert("jsonrpc", "2.0");
	obj.insert("id", response.getId());

	QVariant result = response.getResult();
	RpcError error = response.getError();
	if (error.isValid()) {
		obj.insert("error", errorToJson(error));
	} else {
		obj.insert("result", QJsonValue::fromVariant(result));
	}

	return QJsonDocument(obj);
}

bool JsonRpc::requestDocumentIsValid(const QJsonDocument& request)
{
	if (!request.isObject()) return false;
	QJsonObject obj = request.object();

	QJsonValue jsonrpc = obj.value("jsonrpc"),
		method = obj.value("method"),
		id = obj.value("id");

	if (jsonrpc.isUndefined() || method.isUndefined() || id.isUndefined())
	{
		return false;
	}

	if ((!jsonrpc.isString()) || jsonrpc.toString() != QString("2.0"))
		return false;

	if (!method.isString()) return false;

	if (!id.isString()) return false;

	return true;
}

QJsonObject JsonRpc::errorToJson(const RpcError& error)
{
	QJsonObject errorObj;
	errorObj.insert("code", error.getCode());
	errorObj.insert("message", error.getMessage());
	return errorObj;
}

QJsonDocument JsonRpc::anonymousError(const RpcError& error)
{
	QJsonObject obj;
	obj.insert("jsonrpc", "2.0");
	obj.insert("error", errorToJson(error));
	return QJsonDocument(obj);
}

void JsonRpc::sendToClient(QWebSocket* client, const QJsonDocument& document)
{
	if (!client)
		return;

	client->sendTextMessage(document.toJson(QJsonDocument::Indented));
}