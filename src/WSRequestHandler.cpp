/**
 * obs-websocket
 * Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>
 * Copyright (C) 2017	Mikhail Swift <https://github.com/mikhailswift>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>
 */

#include <functional>

#include <obs-data.h>

#include "Config.h"
#include "Utils.h"

#include "WSRequestHandler.h"

using namespace std::placeholders;

const QHash<QString, RpcMethodHandler> WSRequestHandler::messageMap {
	{ "GetVersion", &WSRequestHandler::GetVersion },
	{ "GetAuthRequired", &WSRequestHandler::GetAuthRequired },
	{ "Authenticate", &WSRequestHandler::Authenticate },

	{ "GetStats", &WSRequestHandler::GetStats },
	{ "SetHeartbeat", &WSRequestHandler::SetHeartbeat },
	{ "GetVideoInfo", &WSRequestHandler::GetVideoInfo },

	{ "SetFilenameFormatting", &WSRequestHandler::SetFilenameFormatting },
	{ "GetFilenameFormatting", &WSRequestHandler::GetFilenameFormatting },

	{ "BroadcastCustomMessage", &WSRequestHandler::BroadcastCustomMessage },

	{ "SetCurrentScene", &WSRequestHandler::SetCurrentScene },
	{ "GetCurrentScene", &WSRequestHandler::GetCurrentScene },
	{ "GetSceneList", &WSRequestHandler::GetSceneList },

	{ "SetSourceRender", &WSRequestHandler::SetSceneItemRender }, // Retrocompat
	{ "SetSceneItemRender", &WSRequestHandler::SetSceneItemRender },
	{ "SetSceneItemPosition", &WSRequestHandler::SetSceneItemPosition },
	{ "SetSceneItemTransform", &WSRequestHandler::SetSceneItemTransform },
	{ "SetSceneItemCrop", &WSRequestHandler::SetSceneItemCrop },
	{ "GetSceneItemProperties", &WSRequestHandler::GetSceneItemProperties },
	{ "SetSceneItemProperties", &WSRequestHandler::SetSceneItemProperties },
	{ "ResetSceneItem", &WSRequestHandler::ResetSceneItem },
	{ "DeleteSceneItem", &WSRequestHandler::DeleteSceneItem },
	{ "DuplicateSceneItem", &WSRequestHandler::DuplicateSceneItem },
	{ "ReorderSceneItems", &WSRequestHandler::ReorderSceneItems },

	{ "GetStreamingStatus", &WSRequestHandler::GetStreamingStatus },
	{ "StartStopStreaming", &WSRequestHandler::StartStopStreaming },
	{ "StartStopRecording", &WSRequestHandler::StartStopRecording },

	{ "StartStreaming", &WSRequestHandler::StartStreaming },
	{ "StopStreaming", &WSRequestHandler::StopStreaming },

	{ "StartRecording", &WSRequestHandler::StartRecording },
	{ "StopRecording", &WSRequestHandler::StopRecording },
	{ "PauseRecording", &WSRequestHandler::PauseRecording },
	{ "ResumeRecording", &WSRequestHandler::ResumeRecording },

	{ "StartStopReplayBuffer", &WSRequestHandler::StartStopReplayBuffer },
	{ "StartReplayBuffer", &WSRequestHandler::StartReplayBuffer },
	{ "StopReplayBuffer", &WSRequestHandler::StopReplayBuffer },
	{ "SaveReplayBuffer", &WSRequestHandler::SaveReplayBuffer },

	{ "SetRecordingFolder", &WSRequestHandler::SetRecordingFolder },
	{ "GetRecordingFolder", &WSRequestHandler::GetRecordingFolder },

	{ "GetTransitionList", &WSRequestHandler::GetTransitionList },
	{ "GetCurrentTransition", &WSRequestHandler::GetCurrentTransition },
	{ "SetCurrentTransition", &WSRequestHandler::SetCurrentTransition },
	{ "SetTransitionDuration", &WSRequestHandler::SetTransitionDuration },
	{ "GetTransitionDuration", &WSRequestHandler::GetTransitionDuration },

	{ "SetVolume", &WSRequestHandler::SetVolume },
	{ "GetVolume", &WSRequestHandler::GetVolume },
	{ "ToggleMute", &WSRequestHandler::ToggleMute },
	{ "SetMute", &WSRequestHandler::SetMute },
	{ "GetMute", &WSRequestHandler::GetMute },
	{ "SetSyncOffset", &WSRequestHandler::SetSyncOffset },
	{ "GetSyncOffset", &WSRequestHandler::GetSyncOffset },
	{ "GetSpecialSources", &WSRequestHandler::GetSpecialSources },
	{ "GetSourcesList", &WSRequestHandler::GetSourcesList },
	{ "GetSourceTypesList", &WSRequestHandler::GetSourceTypesList },
	{ "GetSourceSettings", &WSRequestHandler::GetSourceSettings },
	{ "SetSourceSettings", &WSRequestHandler::SetSourceSettings },
	{ "TakeSourceScreenshot", &WSRequestHandler::TakeSourceScreenshot },

	{ "GetSourceFilters", &WSRequestHandler::GetSourceFilters },
	{ "GetSourceFilterInfo", &WSRequestHandler::GetSourceFilterInfo },
	{ "AddFilterToSource", &WSRequestHandler::AddFilterToSource },
	{ "RemoveFilterFromSource", &WSRequestHandler::RemoveFilterFromSource },
	{ "ReorderSourceFilter", &WSRequestHandler::ReorderSourceFilter },
	{ "MoveSourceFilter", &WSRequestHandler::MoveSourceFilter },
	{ "SetSourceFilterSettings", &WSRequestHandler::SetSourceFilterSettings },
	{ "SetSourceFilterVisibility", &WSRequestHandler::SetSourceFilterVisibility },

	{ "SetCurrentSceneCollection", &WSRequestHandler::SetCurrentSceneCollection },
	{ "GetCurrentSceneCollection", &WSRequestHandler::GetCurrentSceneCollection },
	{ "ListSceneCollections", &WSRequestHandler::ListSceneCollections },

	{ "SetCurrentProfile", &WSRequestHandler::SetCurrentProfile },
	{ "GetCurrentProfile", &WSRequestHandler::GetCurrentProfile },
	{ "ListProfiles", &WSRequestHandler::ListProfiles },

	{ "SetStreamSettings", &WSRequestHandler::SetStreamSettings },
	{ "GetStreamSettings", &WSRequestHandler::GetStreamSettings },
	{ "SaveStreamSettings", &WSRequestHandler::SaveStreamSettings },
#if BUILD_CAPTIONS
	{ "SendCaptions", &WSRequestHandler::SendCaptions },
#endif

	{ "GetStudioModeStatus", &WSRequestHandler::GetStudioModeStatus },
	{ "GetPreviewScene", &WSRequestHandler::GetPreviewScene },
	{ "SetPreviewScene", &WSRequestHandler::SetPreviewScene },
	{ "TransitionToProgram", &WSRequestHandler::TransitionToProgram },
	{ "EnableStudioMode", &WSRequestHandler::EnableStudioMode },
	{ "DisableStudioMode", &WSRequestHandler::DisableStudioMode },
	{ "ToggleStudioMode", &WSRequestHandler::ToggleStudioMode },

	{ "SetTextGDIPlusProperties", &WSRequestHandler::SetTextGDIPlusProperties },
	{ "GetTextGDIPlusProperties", &WSRequestHandler::GetTextGDIPlusProperties },

	{ "SetTextFreetype2Properties", &WSRequestHandler::SetTextFreetype2Properties },
	{ "GetTextFreetype2Properties", &WSRequestHandler::GetTextFreetype2Properties },

	{ "GetBrowserSourceProperties", &WSRequestHandler::GetBrowserSourceProperties },
	{ "SetBrowserSourceProperties", &WSRequestHandler::SetBrowserSourceProperties },

	{ "ListOutputs", &WSRequestHandler::ListOutputs },
	{ "GetOutputInfo", &WSRequestHandler::GetOutputInfo },
	{ "StartOutput", &WSRequestHandler::StartOutput },
	{ "StopOutput", &WSRequestHandler::StopOutput }
};

const QSet<QString> WSRequestHandler::authNotRequired {
	"GetVersion",
	"GetAuthRequired",
	"Authenticate"
};

WSRequestHandler::WSRequestHandler(ConnectionProperties& connProperties) :
	_connProperties(connProperties)
{
}

// std::string WSRequestHandler::processIncomingMessage(std::string& textMessage) {
// 	if (GetConfig()->DebugEnabled) {
// 		blog(LOG_INFO, "Request >> '%s'", textMessage.c_str());
// 	}

// 	OBSDataAutoRelease responseData = processRequest(textMessage);
// 	std::string response = obs_data_get_json(responseData);

// 	if (GetConfig()->DebugEnabled) {
// 		blog(LOG_INFO, "Response << '%s'", response.c_str());
// 	}

// 	return response;
// }

RpcResponse WSRequestHandler::processRequest(const RpcRequest& request){
	// std::string msgContainer(textMessage);
	// const char* msg = msgContainer.c_str();

	// data = obs_data_create_from_json(msg);
	// if (!data) {
	// 	blog(LOG_ERROR, "invalid JSON payload received for '%s'", msg);
	// 	return SendErrorResponse("invalid JSON payload");
	// }

	// if (!hasField("request-type") || !hasField("message-id")) {
	// 	return SendErrorResponse("missing request parameters");
	// }

	// _requestType = obs_data_get_string(data, "request-type");
	// _messageId = obs_data_get_string(data, "message-id");

	if (GetConfig()->AuthRequired
		&& (!authNotRequired.contains(request.methodName()))
		&& (!_connProperties.isAuthenticated()))
	{
		return RpcResponse::fail(request, "Not Authenticated");
	}

	RpcMethodHandler handlerFunc = messageMap[request.methodName()];
	if (!handlerFunc) {
		return RpcResponse::fail(request, "invalid request type");
	}

	return std::bind(handlerFunc, this, _1)(request);
}

// HandlerResponse WSRequestHandler::SendOKResponse(obs_data_t* additionalFields) {
// 	return SendResponse("ok", additionalFields);
// }

// HandlerResponse WSRequestHandler::SendErrorResponse(QString errorMessage) {
// 	OBSDataAutoRelease fields = obs_data_create();
// 	obs_data_set_string(fields, "error", errorMessage.toUtf8().constData());

// 	return SendResponse("error", fields);
// }

// HandlerResponse WSRequestHandler::SendErrorResponse(obs_data_t* additionalFields) {
// 	return SendResponse("error", additionalFields);
// }

// HandlerResponse WSRequestHandler::SendResponse(const char* status, obs_data_t* fields) {
// 	obs_data_t* response = obs_data_create();
// 	obs_data_set_string(response, "message-id", _messageId);
// 	obs_data_set_string(response, "status", status);

// 	if (fields) {
// 		obs_data_apply(response, fields);
// 	}

// 	return response;
// }
