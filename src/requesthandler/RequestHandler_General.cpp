#include "../obs-websocket.h"
#include "../WebSocketServer.h"

#include "RequestHandler.h"

#include "../plugin-macros.generated.h"

RequestResult RequestHandler::GetVersion(const Request& request)
{
	return RequestResult::Success();
}

RequestResult RequestHandler::BroadcastCustomEvent(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateObject("eventData", statusCode, comment)) {
		return RequestResult::Error(statusCode, comment);
	}

	auto webSocketServer = GetWebSocketServer();
	if (!webSocketServer)
		return RequestResult::Error(RequestStatus::RequestProcessingFailed, "Unable to send event.");

	webSocketServer->BroadcastEvent((1 << 0), "CustomEvent", request.RequestData["eventData"]);

	return RequestResult::Success();
}
