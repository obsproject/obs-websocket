#include "RequestHandler.h"

const std::map<std::string, RequestMethodHandler> RequestHandler::_handlerMap
{
	// General
	{"GetVersion", &RequestHandler::GetVersion},
	{"BroadcastCustomEvent", &RequestHandler::BroadcastCustomEvent},
	{"GetStats", &RequestHandler::GetStats},
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
	//{"RemoveInput", &RequestHandler::RemoveInput}, // Disabled for now. Pending obs-studio#5276
	{"SetInputName", &RequestHandler::SetInputName},
	{"GetInputDefaultSettings", &RequestHandler::GetInputDefaultSettings},
	{"GetInputSettings", &RequestHandler::GetInputSettings},
	{"SetInputSettings", &RequestHandler::SetInputSettings},
	{"GetInputMute", &RequestHandler::GetInputMute},
	{"SetInputMute", &RequestHandler::SetInputMute},
	{"ToggleInputMute", &RequestHandler::ToggleInputMute},
	{"GetInputVolume", &RequestHandler::GetInputVolume},
	{"SetInputVolume", &RequestHandler::SetInputVolume},
	{"GetInputAudioSyncOffset", &RequestHandler::GetInputAudioSyncOffset},
	{"SetInputAudioSyncOffset", &RequestHandler::SetInputAudioSyncOffset},
	{"GetInputAudioMonitorType", &RequestHandler::GetInputAudioMonitorType},
	{"SetInputAudioMonitorType", &RequestHandler::SetInputAudioMonitorType},
	{"GetInputPropertiesListPropertyItems", &RequestHandler::GetInputPropertiesListPropertyItems},
	{"PressInputPropertiesButton", &RequestHandler::PressInputPropertiesButton},

	// Scene Items
	{"GetSceneItemList", &RequestHandler::GetSceneItemList},
	{"GetGroupSceneItemList", &RequestHandler::GetGroupSceneItemList},
	{"GetSceneItemId", &RequestHandler::GetSceneItemId},
	{"CreateSceneItem", &RequestHandler::CreateSceneItem},
	{"RemoveSceneItem", &RequestHandler::RemoveSceneItem},
	{"DuplicateSceneItem", &RequestHandler::DuplicateSceneItem},
	{"GetSceneItemTransform", &RequestHandler::GetSceneItemTransform},
	{"SetSceneItemTransform", &RequestHandler::SetSceneItemTransform},
	{"GetSceneItemEnabled", &RequestHandler::GetSceneItemEnabled},
	{"SetSceneItemEnabled", &RequestHandler::SetSceneItemEnabled},
	{"GetSceneItemLocked", &RequestHandler::GetSceneItemLocked},
	{"SetSceneItemLocked", &RequestHandler::SetSceneItemLocked},
	{"GetSceneItemIndex", &RequestHandler::GetSceneItemIndex},
	{"SetSceneItemIndex", &RequestHandler::SetSceneItemIndex},

	// Stream
	{"GetStreamStatus", &RequestHandler::GetStreamStatus},
	{"ToggleStream", &RequestHandler::ToggleStream},
	{"StartStream", &RequestHandler::StartStream},
	{"StopStream", &RequestHandler::StopStream},

	// Record
	{"GetRecordStatus", &RequestHandler::GetRecordStatus},
	{"ToggleRecord", &RequestHandler::ToggleRecord},
	{"StartRecord", &RequestHandler::StartRecord},
	{"StopRecord", &RequestHandler::StopRecord},
	{"ToggleRecordPause", &RequestHandler::ToggleRecordPause},
	{"PauseRecord", &RequestHandler::PauseRecord},
	{"ResumeRecord", &RequestHandler::ResumeRecord},
	//{"GetRecordDirectory", &RequestHandler::GetRecordDirectory},
};

RequestHandler::RequestHandler(SessionPtr session) :
	_session(session)
{
}

RequestResult RequestHandler::ProcessRequest(const Request& request)
{
	if (!request.RequestData.is_object() && !request.RequestData.is_null())
		return RequestResult::Error(RequestStatus::InvalidRequestParameterType, "Your request data is not an object.");

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
