#include <obs-module.h>

#include "WebSocketProtocol.h"
#include "requesthandler/RequestHandler.h"
#include "requesthandler/rpc/RequestStatus.h"

#include "obs-websocket.h"
#include "Config.h"
#include "utils/Utils.h"
#include "plugin-macros.generated.h"

bool IsSupportedRpcVersion(uint8_t requestedVersion)
{
	for (auto version : WebSocketProtocol::SupportedRpcVersions) {
		if (requestedVersion == version)
			return true;
	}
	return false;
}

WebSocketProtocol::ProcessResult SetSessionParameters(SessionPtr session, json incomingMessage)
{
	WebSocketProtocol::ProcessResult ret;

	if (incomingMessage.contains("ignoreInvalidMessages")) {
		if (!incomingMessage["ignoreInvalidMessages"].is_boolean()) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::InvalidIdentifyParameter;
			ret.closeReason = "You specified `ignoreInvalidMessages` but the value is not boolean.";
			return ret;
		}
		session->SetIgnoreInvalidMessages(incomingMessage["ignoreInvalidMessages"]);
	}

	if (incomingMessage.contains("ignoreNonFatalRequestChecks")) {
		if (!incomingMessage["ignoreNonFatalRequestChecks"].is_boolean()) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::InvalidIdentifyParameter;
			ret.closeReason = "You specified `ignoreNonFatalRequestChecks` but the value is not boolean.";
			return ret;
		}
		session->SetIgnoreNonFatalRequestChecks(incomingMessage["ignoreNonFatalRequestChecks"]);
	}

	if (incomingMessage.contains("eventSubscriptions")) {
		if (!incomingMessage["eventSubscriptions"].is_number_unsigned()) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::InvalidIdentifyParameter;
			ret.closeReason = "You specified `eventSubscriptions` but the value is not an unsigned integer.";
			return ret;
		}
		session->SetEventSubscriptions(incomingMessage["eventSubscriptions"]);
	}

	return ret;
}

WebSocketProtocol::ProcessResult WebSocketProtocol::ProcessMessage(SessionPtr session, json incomingMessage)
{
	WebSocketProtocol::ProcessResult ret;

	if (!incomingMessage.is_object()) {
		if (!session->IgnoreInvalidMessages()) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::MessageDecodeError;
			ret.closeReason = "You sent a non-object payload.";
		}
		return ret;
	}

	if (!incomingMessage.contains("messageType")) {
		if (incomingMessage.contains("request-type")) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::UnsupportedProtocolVersion;
			ret.closeReason = "You appear to be attempting to connect with the pre-5.0.0 plugin protocol. Check to make sure your client is updated.";
			return ret;
		}
		if (!session->IgnoreInvalidMessages()) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::UnknownMessageType;
			ret.closeReason = "Your request is missing a `messageType`.";
		}
		return ret;
	}

	std::string messageType = incomingMessage["messageType"];

	if (!session->IsIdentified() && messageType != "Identify") {
		ret.closeCode = WebSocketServer::WebSocketCloseCode::NotIdentified;
		ret.closeReason = "You attempted to send a non-`Identify` message while not identified.";
		return ret;
	}

	if (messageType == "Request") {
		// RequestID checking has to be done here where we are able to close the connection.
		if (!incomingMessage.contains("requestId")) {
			if (!session->IgnoreInvalidMessages()) {
				ret.closeCode = WebSocketServer::WebSocketCloseCode::RequestMissingRequiredField;
				ret.closeReason = "Your request is missing a `requestId`.";
			}
			return ret;
		}

		if (!incomingMessage["requestType"].is_string()) {
			incomingMessage["requestType"] = "";
		}

		RequestHandler requestHandler;
		Request request(session->RpcVersion(), session->IgnoreNonFatalRequestChecks(), incomingMessage["requestType"], incomingMessage["requestData"]);

		RequestResult requestResult = requestHandler.ProcessRequest(request);

		ret.result["messageType"] = "RequestResponse";
		ret.result["requestType"] = incomingMessage["requestType"];
		ret.result["requestId"] = incomingMessage["requestId"];
		ret.result["requestStatus"] = {
			{"result", requestResult.StatusCode == RequestStatus::Success},
			{"code", requestResult.StatusCode}
		};
		if (!requestResult.Comment.empty())
			ret.result["requestStatus"]["comment"] = requestResult.Comment;
		if (requestResult.ResponseData.is_object())
			ret.result["responseData"] = requestResult.ResponseData;
		
		return ret;
	} else if (messageType == "RequestBatch") {
		// RequestID checking has to be done here where we are able to close the connection.
		if (!incomingMessage.contains("requestId")) {
			if (!session->IgnoreInvalidMessages()) {
				ret.closeCode = WebSocketServer::WebSocketCloseCode::RequestMissingRequiredField;
				ret.closeReason = "Your request batch is missing a `requestId`.";
			}
			return ret;
		}

		if (!incomingMessage["requests"].is_array()) {
			if (!session->IgnoreInvalidMessages()) {
				ret.closeCode = WebSocketServer::WebSocketCloseCode::RequestMissingRequiredField;
				ret.closeReason = "Your request batch is missing a `requests` or it is not an array.";
			}
			return ret;
		}

		auto requests = incomingMessage["requests"].get<std::vector<json>>();
		json results = json::array();

		RequestHandler requestHandler;
		for (auto requestJson : requests) {
			if (!requestJson["requestType"].is_string())
				requestJson["requestType"] = "";

			Request request(session->RpcVersion(), session->IgnoreNonFatalRequestChecks(), requestJson["requestType"], requestJson["requestData"]);

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

		ret.result["messageType"] = "RequestBatchResponse";
		ret.result["requestId"] = incomingMessage["requestId"];
		ret.result["results"] = results;

		return ret;
	} else if (messageType == "Identify") {
		std::unique_lock<std::mutex> sessionLock(session->OperationMutex);
		if (session->IsIdentified()) {
			if (!session->IgnoreInvalidMessages()) {
				ret.closeCode = WebSocketServer::WebSocketCloseCode::AlreadyIdentified;
				ret.closeReason = "You are already Identified with the obs-websocket server.";
			}
			return ret;
		}

		if (session->AuthenticationRequired()) {
			if (!incomingMessage.contains("authentication")) {
				ret.closeCode = WebSocketServer::WebSocketCloseCode::InvalidIdentifyParameter;
				ret.closeReason = "Your `Identify` payload is missing an `authentication` string, however authentication is required.";
				return ret;
			}
			if (!Utils::Crypto::CheckAuthenticationString(session->Secret(), session->Challenge(), incomingMessage["authentication"])) {
				auto conf = GetConfig();
				if (conf && conf->AlertsEnabled) {
					obs_frontend_push_ui_translation(obs_module_get_string);
					QString title = QObject::tr("OBSWebSocket.TrayNotification.AuthenticationFailed.Title");
					QString body = QObject::tr("OBSWebSocket.TrayNotification.AuthenticationFailed.Body");
					obs_frontend_pop_ui_translation();
					Utils::Platform::SendTrayNotification(QSystemTrayIcon::Warning, title, body);
				}
				ret.closeCode = WebSocketServer::WebSocketCloseCode::AuthenticationFailed;
				ret.closeReason = "Authentication failed.";
				return ret;
			}
		}

		if (!incomingMessage.contains("rpcVersion") || !incomingMessage["rpcVersion"].is_number_unsigned()) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::InvalidIdentifyParameter;
			ret.closeReason = "Your Identify is missing `rpcVersion` or is not an integer.";
			return ret;
		}
		uint8_t requestedRpcVersion = incomingMessage["rpcVersion"];
		if (!IsSupportedRpcVersion(requestedRpcVersion)) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::UnsupportedProtocolVersion;
			ret.closeReason = "Your requested RPC version is not supported by this server.";
			return ret;
		}
		session->SetRpcVersion(requestedRpcVersion);

		WebSocketProtocol::ProcessResult parameterResult = SetSessionParameters(session, incomingMessage);
		if (ret.closeCode != WebSocketServer::WebSocketCloseCode::DontClose) {
			return parameterResult;
		}

		session->SetIsIdentified(true);

		auto conf = GetConfig();
		if (conf && conf->AlertsEnabled) {
			obs_frontend_push_ui_translation(obs_module_get_string);
			QString title = QObject::tr("OBSWebSocket.TrayNotification.Identified.Title");
			QString body = QObject::tr("OBSWebSocket.TrayNotification.Identified.Body");
			obs_frontend_pop_ui_translation();
			Utils::Platform::SendTrayNotification(QSystemTrayIcon::Information, title, body);
		}

		ret.result["messageType"] = "Identified";
		ret.result["negotiatedRpcVersion"] = session->RpcVersion();
		return ret;
	} else if (messageType == "Reidentify") {
		std::unique_lock<std::mutex> sessionLock(session->OperationMutex);

		WebSocketProtocol::ProcessResult parameterResult = SetSessionParameters(session, incomingMessage);
		if (ret.closeCode != WebSocketServer::WebSocketCloseCode::DontClose) {
			return parameterResult;
		}

		ret.result["messageType"] = "Identified";
		ret.result["negotiatedRpcVersion"] = session->RpcVersion();
		return ret;
	} else {
		if (!session->IgnoreInvalidMessages()) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::UnknownMessageType;
			ret.closeReason = std::string("Unknown message type: %s") + messageType;
		}
		return ret;
	}

	return ret;
}
