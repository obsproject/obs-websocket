#include "RequestHandler.h"

#include "../plugin-macros.generated.h"

const std::map<std::string, RequestMethodHandler> RequestHandler::_handlerMap
{
	// General
	{"GetVersion", &RequestHandler::GetVersion},
	{"BroadcastCustomEvent", &RequestHandler::BroadcastCustomEvent},
};

RequestResult RequestHandler::ProcessRequest(const Request& request)
{
	if (!request.RequestData.is_null() && !request.RequestData.is_object())
		return RequestResult::Error(RequestStatus::InvalidRequestParameterDataType, "Your request data is not an object.");

	RequestMethodHandler handler;
	try {
		handler = _handlerMap.at(request.RequestType);
	} catch (const std::out_of_range& oor) {
		return RequestResult::Error(RequestStatus::UnknownRequestType, "Your request type is not valid.");
	}

	return std::bind(handler, this, std::placeholders::_1)(request);
}