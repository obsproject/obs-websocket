#pragma once

#include <map>
#include <obs.hpp>
#include <obs-frontend-api.h>

#include "rpc/Request.h"
#include "rpc/RequestResult.h"
#include "../obs-websocket.h"
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
		RequestResult GetPersistentData(const Request&);
		RequestResult SetPersistentData(const Request&);
		RequestResult GetSceneCollectionList(const Request&);
		RequestResult SetCurrentSceneCollection(const Request&);
		RequestResult CreateSceneCollection(const Request&);
		RequestResult GetProfileList(const Request&);
		RequestResult SetCurrentProfile(const Request&);
		RequestResult CreateProfile(const Request&);
		RequestResult RemoveProfile(const Request&);
		RequestResult GetProfileParameter(const Request&);
		RequestResult SetProfileParameter(const Request&);
		RequestResult GetVideoSettings(const Request&);
		RequestResult SetVideoSettings(const Request&);
		RequestResult GetStreamServiceSettings(const Request&);
		RequestResult SetStreamServiceSettings(const Request&);

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
		RequestResult CreateScene(const Request&);
		RequestResult RemoveScene(const Request&);
		RequestResult SetSceneName(const Request&);

		// Inputs
		RequestResult GetInputList(const Request&);
		RequestResult GetInputKindList(const Request&);
		RequestResult CreateInput(const Request&);
		RequestResult SetInputName(const Request&);
		RequestResult GetInputDefaultSettings(const Request&);
		RequestResult GetInputSettings(const Request&);
		RequestResult SetInputSettings(const Request&);
		RequestResult GetInputMute(const Request&);
		RequestResult SetInputMute(const Request&);
		RequestResult ToggleInputMute(const Request&);
		RequestResult GetInputVolume(const Request&);
		RequestResult SetInputVolume(const Request&);

		// Stream
		RequestResult GetStreamStatus(const Request&);
		RequestResult StartStream(const Request&);
		RequestResult StopStream(const Request&);

		static const std::map<std::string, RequestMethodHandler> _handlerMap;
};
