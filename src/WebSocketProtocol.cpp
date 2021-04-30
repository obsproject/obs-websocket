#include "WebSocketProtocol.h"
#include "obs-websocket.h"
#include "utils/Utils.h"

#include "plugin-macros.generated.h"

WebSocketProtocol::ProcessResult SetSessionParameters(SessionPtr session, json incomingMessage)
{
	WebSocketProtocol::ProcessResult ret;

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
	} else {
		if (!session->IgnoreInvalidMessages()) {
			ret.closeCode = WebSocketServer::WebSocketCloseCode::UnknownMessageType;
			ret.closeReason = std::string("Unknown message type: %s") + messageType;
		}
		return ret;
	}

	return ret;
}
