#include <obs-module.h>

#include "WebSocketProtocol.h"
#include "WebSocketSession.h"
#include "requesthandler/RequestHandler.h"
#include "requesthandler/rpc/RequestStatus.h"
#include "obs-websocket.h"
#include "Config.h"
#include "plugin-macros.generated.h"
#include "utils/Crypto.h"
#include "utils/Json.h"
#include "utils/Platform.h"

bool IsSupportedRpcVersion(uint8_t requestedVersion)
{
	for (auto version : WebSocketProtocol::SupportedRpcVersions) {
		if (requestedVersion == version)
			return true;
	}
	return false;
}

void SetSessionParameters(SessionPtr session, WebSocketProtocol::ProcessResult &ret, json payloadData)
{
	if (payloadData.contains("ignoreInvalidMessages")) {
		if (!payloadData["ignoreInvalidMessages"].is_boolean()) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::InvalidDataKeyType;
			ret.closeReason = "Your `ignoreInvalidMessages` is not a boolean.";
			return;
		}
		session->SetIgnoreInvalidMessages(payloadData["ignoreInvalidMessages"]);
	}

	if (payloadData.contains("ignoreNonFatalRequestChecks")) {
		if (!payloadData["ignoreNonFatalRequestChecks"].is_boolean()) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::InvalidDataKeyType;
			ret.closeReason = "Your `ignoreNonFatalRequestChecks` is not a boolean.";
			return;
		}
		session->SetIgnoreNonFatalRequestChecks(payloadData["ignoreNonFatalRequestChecks"]);
	}

	if (payloadData.contains("eventSubscriptions")) {
		if (!payloadData["eventSubscriptions"].is_number_unsigned()) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::InvalidDataKeyType;
			ret.closeReason = "Your `eventSubscriptions` is not an unsigned number.";
			return;
		}
		session->SetEventSubscriptions(payloadData["eventSubscriptions"]);
	}
}

void WebSocketProtocol::ProcessMessage(SessionPtr session, WebSocketProtocol::ProcessResult &ret, uint8_t opCode, json payloadData)
{
	if (!payloadData.is_object()) {
		if (payloadData.is_null()) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::MissingDataKey;
			ret.closeReason = "Your payload is missing data (`d`).";
		} else {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::InvalidDataKeyType;
			ret.closeReason = "Your payload's data (`d`) is not an object.";
		}
		return;
	}

	// Only `Identify` is allowed when not identified
	if (!session->IsIdentified() && opCode != 1) {
		ret.closeCode = WebSocketServer::WebSocketCloseCode::NotIdentified;
		ret.closeReason = "You attempted to send a non-Identify message while not identified.";
		return;
	}

	switch (opCode) {
		case 1: { // Identify
			std::unique_lock<std::mutex> sessionLock(session->OperationMutex);
			if (session->IsIdentified()) {
				if (!session->IgnoreInvalidMessages()) {
					ret.closeCode = WebSocketServer::WebSocketCloseCode::AlreadyIdentified;
					ret.closeReason = "You are already Identified with the obs-websocket server.";
				}
				return;
			}

			if (session->AuthenticationRequired()) {
				if (!payloadData.contains("authentication")) {
					ret.closeCode = WebSocketServer::WebSocketCloseCode::AuthenticationFailed;
					ret.closeReason = "Your payload's data is missing an `authentication` string, however authentication is required.";
					return;
				}
				if (!Utils::Crypto::CheckAuthenticationString(session->Secret(), session->Challenge(), payloadData["authentication"])) {
					auto conf = GetConfig();
					if (conf && conf->AlertsEnabled) {
						QString title = obs_module_text("OBSWebSocket.TrayNotification.AuthenticationFailed.Title");
						QString body = QString(obs_module_text("OBSWebSocket.TrayNotification.AuthenticationFailed.Body")).arg(QString::fromStdString(session->RemoteAddress()));
						Utils::Platform::SendTrayNotification(QSystemTrayIcon::Warning, title, body);
					}
					ret.closeCode = WebSocketServer::WebSocketCloseCode::AuthenticationFailed;
					ret.closeReason = "Authentication failed.";
					return;
				}
			}

			if (!payloadData.contains("rpcVersion")) {
				ret.closeCode = WebSocketServer::WebSocketCloseCode::MissingDataKey;
				ret.closeReason = "Your payload's data is missing an `rpcVersion`.";
				return;
			}

			if (!payloadData["rpcVersion"].is_number_unsigned()) {
				ret.closeCode = WebSocketServer::WebSocketCloseCode::InvalidDataKeyType;
				ret.closeReason = "Your `rpcVersion` is not an unsigned number.";
			}

			uint8_t requestedRpcVersion = payloadData["rpcVersion"];
			if (!IsSupportedRpcVersion(requestedRpcVersion)) {
				ret.closeCode = WebSocketServer::WebSocketCloseCode::UnsupportedRpcVersion;
				ret.closeReason = "Your requested RPC version is not supported by this server.";
				return;
			}
			session->SetRpcVersion(requestedRpcVersion);

			SetSessionParameters(session, ret, payloadData);
			if (ret.closeCode != WebSocketServer::WebSocketCloseCode::DontClose) {
				return;
			}

			session->SetIsIdentified(true);

			auto conf = GetConfig();
			if (conf && conf->AlertsEnabled) {
				QString title = obs_module_text("OBSWebSocket.TrayNotification.Identified.Title");
				QString body = QString(obs_module_text("OBSWebSocket.TrayNotification.Identified.Body")).arg(QString::fromStdString(session->RemoteAddress()));
				Utils::Platform::SendTrayNotification(QSystemTrayIcon::Information, title, body);
			}

			ret.result["op"] = 2;
			ret.result["d"]["negotiatedRpcVersion"] = session->RpcVersion();
			} return;
		case 3: { // Reidentify
			std::unique_lock<std::mutex> sessionLock(session->OperationMutex);

			SetSessionParameters(session, ret, payloadData);
			if (ret.closeCode != WebSocketServer::WebSocketCloseCode::DontClose) {
				return;
			}

			ret.result["op"] = 2;
			ret.result["d"]["negotiatedRpcVersion"] = session->RpcVersion();
			} return;
		case 6: { // Request
			// RequestID checking has to be done here where we are able to close the connection.
			if (!payloadData.contains("requestId")) {
				if (!session->IgnoreInvalidMessages()) {
					ret.closeCode = WebSocketServer::WebSocketCloseCode::MissingDataKey;
					ret.closeReason = "Your payload data is missing a `requestId`.";
				}
				return;
			}

			RequestHandler requestHandler;
			Request request(session, payloadData["requestType"], payloadData["requestData"]);

			RequestResult requestResult = requestHandler.ProcessRequest(request);

			json resultPayloadData;
			resultPayloadData["requestType"] = payloadData["requestType"];
			resultPayloadData["requestId"] = payloadData["requestId"];
			resultPayloadData["requestStatus"] = {
				{"result", requestResult.StatusCode == RequestStatus::Success},
				{"code", requestResult.StatusCode}
			};
			if (!requestResult.Comment.empty())
				resultPayloadData["requestStatus"]["comment"] = requestResult.Comment;
			if (requestResult.ResponseData.is_object())
				resultPayloadData["responseData"] = requestResult.ResponseData;
			ret.result["op"] = 7;
			ret.result["d"] = resultPayloadData;
			} return;
		case 8: { // RequestBatch
			// RequestID checking has to be done here where we are able to close the connection.
			if (!payloadData.contains("requestId")) {
				if (!session->IgnoreInvalidMessages()) {
					ret.closeCode = WebSocketServer::WebSocketCloseCode::MissingDataKey;
					ret.closeReason = "Your payload data is missing a `requestId`.";
				}
				return;
			}

			if (!payloadData.contains("requests")) {
				if (!session->IgnoreInvalidMessages()) {
					ret.closeCode = WebSocketServer::WebSocketCloseCode::MissingDataKey;
					ret.closeReason = "Your payload data is missing a `requests`.";
				}
				return;
			}

			if (!payloadData["requests"].is_array()) {
				if (!session->IgnoreInvalidMessages()) {
					ret.closeCode = WebSocketServer::WebSocketCloseCode::InvalidDataKeyType;
					ret.closeReason = "Your `requests` is not an array.";
				}
				return;
			}

			std::vector<json> requests = payloadData["requests"];
			json results = json::array();

			RequestHandler requestHandler;
			for (auto requestJson : requests) {
				Request request(session, requestJson["requestType"], requestJson["requestData"]);

				RequestResult requestResult = requestHandler.ProcessRequest(request);

				json result;
				result["requestType"] = requestJson["requestType"];

				if (requestJson.contains("requestId"))
					result["requestId"] = requestJson["requestId"];

				result["requestStatus"] = {
					{"result", requestResult.StatusCode == RequestStatus::Success},
					{"code", requestResult.StatusCode}
				};

				if (!requestResult.Comment.empty())
					result["requestStatus"]["comment"] = requestResult.Comment;

				if (requestResult.ResponseData.is_object())
					result["responseData"] = requestResult.ResponseData;

				results.push_back(result);
			}

			ret.result["op"] = 9;
			ret.result["d"]["requestId"] = payloadData["requestId"];
			ret.result["d"]["results"] = results;
			} return;
		default:
			if (!session->IgnoreInvalidMessages()) {
				ret.closeCode = WebSocketServer::WebSocketCloseCode::UnknownOpCode;
				ret.closeReason = std::string("Unknown OpCode: %s") + std::to_string(opCode);
			}
			return;
	}
}
