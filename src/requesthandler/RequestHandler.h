#pragma once

#include <map>

#include "rpc/Request.h"
#include "rpc/RequestResult.h"
#include "../utils/Utils.h"

class RequestHandler;
typedef RequestResult(RequestHandler::*RequestMethodHandler)(const Request&);

class RequestHandler {
	public:
		RequestResult ProcessRequest(const Request& request);
		std::vector<std::string> GetRequestList();

	private:
		RequestResult GetVersion(const Request&);
		RequestResult BroadcastCustomEvent(const Request&);

		static const std::map<std::string, RequestMethodHandler> _handlerMap;
};
