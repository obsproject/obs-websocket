#pragma once

#include <map>
#include <obs.hpp>
#include <obs-frontend-api.h>

#include "../obs-websocket.h"
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

		// Sources
		RequestResult GetSourceActive(const Request&);
		RequestResult GetSourceScreenshot(const Request&);
		RequestResult SaveSourceScreenshot(const Request&);

		// Scenes
		RequestResult GetSceneList(const Request&);
		RequestResult GetCurrentProgramScene(const Request&);
		RequestResult SetCurrentProgramScene(const Request&);
		RequestResult GetCurrentPreviewScene(const Request&);
		RequestResult SetCurrentPreviewScene(const Request&);
		RequestResult SetSceneName(const Request&);
		RequestResult CreateScene(const Request&);

		static const std::map<std::string, RequestMethodHandler> _handlerMap;
};
