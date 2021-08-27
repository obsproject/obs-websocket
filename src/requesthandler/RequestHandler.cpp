#include "RequestHandler.h"
#include "../plugin-macros.generated.h"

const std::map<std::string, RequestMethodHandler> RequestHandler::_handlerMap
{
	// General
	{"GetVersion", &RequestHandler::GetVersion},
	{"BroadcastCustomEvent", &RequestHandler::BroadcastCustomEvent},
	{"GetHotkeyList", &RequestHandler::GetHotkeyList},
	{"TriggerHotkeyByName", &RequestHandler::TriggerHotkeyByName},
	{"TriggerHotkeyByKeySequence", &RequestHandler::TriggerHotkeyByKeySequence},
	{"GetStudioModeEnabled", &RequestHandler::GetStudioModeEnabled},
	{"SetStudioModeEnabled", &RequestHandler::SetStudioModeEnabled},
	{"Sleep", &RequestHandler::Sleep},

	// Config
	{"GetPersistentData", &RequestHandler::GetPersistentData},
	{"SetPersistentData", &RequestHandler::SetPersistentData},
	{"GetSceneCollectionList", &RequestHandler::GetSceneCollectionList},
	{"SetCurrentSceneCollection", &RequestHandler::SetCurrentSceneCollection},
	{"CreateSceneCollection", &RequestHandler::CreateSceneCollection},
	{"GetProfileList", &RequestHandler::GetProfileList},
	{"SetCurrentProfile", &RequestHandler::SetCurrentProfile},
	{"CreateProfile", &RequestHandler::CreateProfile},
	{"RemoveProfile", &RequestHandler::RemoveProfile},
	{"GetProfileParameter", &RequestHandler::GetProfileParameter},
	{"SetProfileParameter", &RequestHandler::SetProfileParameter},
	{"GetVideoSettings", &RequestHandler::GetVideoSettings},
	{"SetVideoSettings", &RequestHandler::SetVideoSettings},
	{"GetStreamServiceSettings", &RequestHandler::GetStreamServiceSettings},
	{"SetStreamServiceSettings", &RequestHandler::SetStreamServiceSettings},

	// Sources
	{"GetSourceActive", &RequestHandler::GetSourceActive},
	{"GetSourceScreenshot", &RequestHandler::GetSourceScreenshot},
	{"SaveSourceScreenshot", &RequestHandler::SaveSourceScreenshot},

	// Scenes
	{"GetSceneList", &RequestHandler::GetSceneList},
	{"GetCurrentProgramScene", &RequestHandler::GetCurrentProgramScene},
	{"SetCurrentProgramScene", &RequestHandler::SetCurrentProgramScene},
	{"GetCurrentPreviewScene", &RequestHandler::GetCurrentPreviewScene},
	{"SetCurrentPreviewScene", &RequestHandler::SetCurrentPreviewScene},
	{"CreateScene", &RequestHandler::CreateScene},
	{"RemoveScene", &RequestHandler::RemoveScene},
	{"SetSceneName", &RequestHandler::SetSceneName},

	// Inputs
	{"GetInputList", &RequestHandler::GetInputList},
	{"GetInputKindList", &RequestHandler::GetInputKindList},
	{"CreateInput", &RequestHandler::CreateInput},
	{"SetInputName", &RequestHandler::SetInputName},
	{"GetInputDefaultSettings", &RequestHandler::GetInputDefaultSettings},
	{"GetInputSettings", &RequestHandler::GetInputSettings},
	{"SetInputSettings", &RequestHandler::SetInputSettings},
	{"GetInputMute", &RequestHandler::GetInputMute},
	{"SetInputMute", &RequestHandler::SetInputMute},
	{"ToggleInputMute", &RequestHandler::ToggleInputMute},
	{"GetInputVolume", &RequestHandler::GetInputVolume},
	{"SetInputVolume", &RequestHandler::SetInputVolume},

	// Stream
	{"GetStreamStatus", &RequestHandler::GetStreamStatus},
	{"StartStream", &RequestHandler::StartStream},
	{"StopStream", &RequestHandler::StopStream},
};

RequestResult RequestHandler::ProcessRequest(const Request& request)
{
	if (!request.RequestData.is_null() && !request.RequestData.is_object())
		return RequestResult::Error(RequestStatus::InvalidRequestParameterDataType, "Your request data is not an object.");

	if (request.RequestType.empty())
		return RequestResult::Error(RequestStatus::MissingRequestType, "Your request is missing a `requestType`");

	RequestMethodHandler handler;
	try {
		handler = _handlerMap.at(request.RequestType);
	} catch (const std::out_of_range& oor) {
		return RequestResult::Error(RequestStatus::UnknownRequestType, "Your request type is not valid.");
	}

	return std::bind(handler, this, std::placeholders::_1)(request);
}

std::vector<std::string> RequestHandler::GetRequestList()
{
	std::vector<std::string> ret;
	for (auto const& [key, val] : _handlerMap) {
		ret.push_back(key);
	}

	return ret;
}
