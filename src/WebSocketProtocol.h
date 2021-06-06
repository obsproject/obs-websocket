#pragma once

#include <vector>
#include <string>
#include <nlohmann/json.hpp>

#include "WebSocketServer.h"
#include "WebSocketSession.h"

namespace WebSocketProtocol {
	const std::vector<uint8_t> SupportedRpcVersions{
		1
	};

	struct ProcessResult {
		WebSocketServer::WebSocketCloseCode closeCode = WebSocketServer::WebSocketCloseCode::DontClose;
		std::string closeReason;
		json result;
	};

	ProcessResult ProcessMessage(SessionPtr session, json incomingMessage);
}
