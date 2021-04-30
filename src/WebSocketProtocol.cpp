#include "WebSocketProtocol.h"
#include "obs-websocket.h"
#include "utils/Utils.h"

#include "plugin-macros.generated.h"

bool IsSupportedRpcVersion(uint8_t requestedVersion)
{
	for (auto version : WebSocketProtocol::SupportedRpcVersions) {
		if (requestedVersion = version)
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
			ret.closeReason = "Your request is missing a `messageType`";
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
		;
	} else if (messageType == "RequestBatch") {
		;
	} else if (messageType == "Identify") {
		std::unique_lock<std::mutex> sessionLock(session->OperationMutex);
		if (session->IsIdentified()) {
			if (!session->IgnoreInvalidMessages()) {
				ret.closeCode = WebSocketServer::WebSocketCloseCode::AlreadyIdentified;
				ret.closeReason = "You are already Identified with the obs-websocket server.";
			}
			return ret;
		}

		auto webSocketServer = GetWebSocketServer();
		if (!webSocketServer) {
			blog(LOG_ERROR, "[WebSocketProtocol::ProcessMessage] Unable to fetch websocket server instance!");
			return ret;
		}

		if (webSocketServer->AuthenticationRequired) {
			if (!incomingMessage.contains("authentication")) {
				ret.closeCode = WebSocketServer::WebSocketCloseCode::InvalidIdentifyParameter;
				ret.closeReason = "Your `Identify` payload is missing an `authentication` string, however authentication is required.";
				return ret;
			}
			if (!Utils::Crypto::CheckAuthenticationString(webSocketServer->AuthenticationSecret, session->Challenge(), incomingMessage["authentication"])) {
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
