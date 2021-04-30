#pragma once

#include <string>
#include <nlohmann/json.hpp>

#include "WebSocketServer.h"
#include "WebSocketSession.h"
#include "requesthandler/RequestHandler.h"

namespace WebSocketProtocol {
	struct ProcessResult {
		WebSocketServer::WebSocketCloseCode closeCode = WebSocketServer::WebSocketCloseCode::DontClose;
		std::string closeReason;
		json result;
	};

	ProcessResult ProcessMessage(websocketpp::connection_hdl hdl, WebSocketSession *session, json incomingMessage);
}