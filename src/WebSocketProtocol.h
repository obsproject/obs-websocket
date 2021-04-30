#pragma once

#include <string>

#include "WebSocketServer.h"
#include "WebSocketSession.h"
#include "requesthandler/RequestHandler.h"

namespace WebSocketProtocol {
	struct ProcessResult {
		WebSocketServer::WebSocketCloseCode closeCode;
		std::string closeReason;
		json result;
	};

	ProcessResult Process(websocketpp::connection_hdl hdl, WebSocketSession *session, json incomingMessage);
}