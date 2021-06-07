#pragma once

#include <map>
#include <obs.hpp>
#include <obs-frontend-api.h>

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
		// General
		RequestResult GetVersion(const Request&);
		RequestResult BroadcastCustomEvent(const Request&);
		RequestResult GetHotkeyList(const Request&);
		RequestResult TriggerHotkeyByName(const Request&);
		RequestResult TriggerHotkeyByKeySequence(const Request&);
		RequestResult GetStudioModeEnabled(const Request&);
		RequestResult SetStudioModeEnabled(const Request&);
		RequestResult Sleep(const Request&);

		// Config
		RequestResult GetSceneCollectionList(const Request&);
		RequestResult SetCurrentSceneCollection(const Request&);
		RequestResult GetProfileList(const Request&);
		RequestResult SetCurrentProfile(const Request&);
		RequestResult GetProfileParameter(const Request&);
		RequestResult SetProfileParameter(const Request&);

		static const std::map<std::string, RequestMethodHandler> _handlerMap;
};
