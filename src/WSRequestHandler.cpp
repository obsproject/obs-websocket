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

#include <obs-data.h>

#include "Config.h"
#include "Utils.h"

#include "WSRequestHandler.h"

QHash<QString, HandlerResponse(*)(WSRequestHandler*)> WSRequestHandler::messageMap {
	{ "GetVersion", WSRequestHandler::HandleGetVersion },
	{ "GetAuthRequired", WSRequestHandler::HandleGetAuthRequired },
	{ "Authenticate", WSRequestHandler::HandleAuthenticate },

	{ "GetStats", WSRequestHandler::HandleGetStats },
	{ "SetHeartbeat", WSRequestHandler::HandleSetHeartbeat },
	{ "GetVideoInfo", WSRequestHandler::HandleGetVideoInfo },

	{ "SetFilenameFormatting", WSRequestHandler::HandleSetFilenameFormatting },
	{ "GetFilenameFormatting", WSRequestHandler::HandleGetFilenameFormatting },

	{ "BroadcastWebSocketMessage", WSRequestHandler::HandleBroadcastWebSocketMessage },

	{ "SetCurrentScene", WSRequestHandler::HandleSetCurrentScene },
	{ "GetCurrentScene", WSRequestHandler::HandleGetCurrentScene },
	{ "GetSceneList", WSRequestHandler::HandleGetSceneList },

	{ "SetSourceRender", WSRequestHandler::HandleSetSceneItemRender }, // Retrocompat
	{ "SetSceneItemRender", WSRequestHandler::HandleSetSceneItemRender },
	{ "SetSceneItemPosition", WSRequestHandler::HandleSetSceneItemPosition },
	{ "SetSceneItemTransform", WSRequestHandler::HandleSetSceneItemTransform },
	{ "SetSceneItemCrop", WSRequestHandler::HandleSetSceneItemCrop },
	{ "GetSceneItemProperties", WSRequestHandler::HandleGetSceneItemProperties },
	{ "SetSceneItemProperties", WSRequestHandler::HandleSetSceneItemProperties },
	{ "ResetSceneItem", WSRequestHandler::HandleResetSceneItem },
	{ "DeleteSceneItem", WSRequestHandler::HandleDeleteSceneItem },
	{ "DuplicateSceneItem", WSRequestHandler::HandleDuplicateSceneItem },
	{ "ReorderSceneItems", WSRequestHandler::HandleReorderSceneItems },

	{ "GetStreamingStatus", WSRequestHandler::HandleGetStreamingStatus },
	{ "StartStopStreaming", WSRequestHandler::HandleStartStopStreaming },
	{ "StartStopRecording", WSRequestHandler::HandleStartStopRecording },
	{ "StartStreaming", WSRequestHandler::HandleStartStreaming },
	{ "StopStreaming", WSRequestHandler::HandleStopStreaming },
	{ "StartRecording", WSRequestHandler::HandleStartRecording },
	{ "StopRecording", WSRequestHandler::HandleStopRecording },

	{ "StartStopReplayBuffer", WSRequestHandler::HandleStartStopReplayBuffer },
	{ "StartReplayBuffer", WSRequestHandler::HandleStartReplayBuffer },
	{ "StopReplayBuffer", WSRequestHandler::HandleStopReplayBuffer },
	{ "SaveReplayBuffer", WSRequestHandler::HandleSaveReplayBuffer },

	{ "SetRecordingFolder", WSRequestHandler::HandleSetRecordingFolder },
	{ "GetRecordingFolder", WSRequestHandler::HandleGetRecordingFolder },

	{ "GetTransitionList", WSRequestHandler::HandleGetTransitionList },
	{ "GetCurrentTransition", WSRequestHandler::HandleGetCurrentTransition },
	{ "SetCurrentTransition", WSRequestHandler::HandleSetCurrentTransition },
	{ "SetTransitionDuration", WSRequestHandler::HandleSetTransitionDuration },
	{ "GetTransitionDuration", WSRequestHandler::HandleGetTransitionDuration },

	{ "SetVolume", WSRequestHandler::HandleSetVolume },
	{ "GetVolume", WSRequestHandler::HandleGetVolume },
	{ "ToggleMute", WSRequestHandler::HandleToggleMute },
	{ "SetMute", WSRequestHandler::HandleSetMute },
	{ "GetMute", WSRequestHandler::HandleGetMute },
	{ "SetSyncOffset", WSRequestHandler::HandleSetSyncOffset },
	{ "GetSyncOffset", WSRequestHandler::HandleGetSyncOffset },
	{ "GetSpecialSources", WSRequestHandler::HandleGetSpecialSources },
	{ "GetSourcesList", WSRequestHandler::HandleGetSourcesList },
	{ "GetSourceTypesList", WSRequestHandler::HandleGetSourceTypesList },
	{ "GetSourceSettings", WSRequestHandler::HandleGetSourceSettings },
	{ "SetSourceSettings", WSRequestHandler::HandleSetSourceSettings },
	{ "TakeSourceScreenshot", WSRequestHandler::HandleTakeSourceScreenshot },

	{ "GetSourceFilters", WSRequestHandler::HandleGetSourceFilters },
	{ "AddFilterToSource", WSRequestHandler::HandleAddFilterToSource },
	{ "RemoveFilterFromSource", WSRequestHandler::HandleRemoveFilterFromSource },
	{ "ReorderSourceFilter", WSRequestHandler::HandleReorderSourceFilter },
	{ "MoveSourceFilter", WSRequestHandler::HandleMoveSourceFilter },
	{ "SetSourceFilterSettings", WSRequestHandler::HandleSetSourceFilterSettings },

	{ "SetCurrentSceneCollection", WSRequestHandler::HandleSetCurrentSceneCollection },
	{ "GetCurrentSceneCollection", WSRequestHandler::HandleGetCurrentSceneCollection },
	{ "ListSceneCollections", WSRequestHandler::HandleListSceneCollections },

	{ "SetCurrentProfile", WSRequestHandler::HandleSetCurrentProfile },
	{ "GetCurrentProfile", WSRequestHandler::HandleGetCurrentProfile },
	{ "ListProfiles", WSRequestHandler::HandleListProfiles },

	{ "SetStreamSettings", WSRequestHandler::HandleSetStreamSettings },
	{ "GetStreamSettings", WSRequestHandler::HandleGetStreamSettings },
	{ "SaveStreamSettings", WSRequestHandler::HandleSaveStreamSettings },
#if BUILD_CAPTIONS
	{ "SendCaptions", WSRequestHandler::HandleSendCaptions },
#endif

	{ "GetStudioModeStatus", WSRequestHandler::HandleGetStudioModeStatus },
	{ "GetPreviewScene", WSRequestHandler::HandleGetPreviewScene },
	{ "SetPreviewScene", WSRequestHandler::HandleSetPreviewScene },
	{ "TransitionToProgram", WSRequestHandler::HandleTransitionToProgram },
	{ "EnableStudioMode", WSRequestHandler::HandleEnableStudioMode },
	{ "DisableStudioMode", WSRequestHandler::HandleDisableStudioMode },
	{ "ToggleStudioMode", WSRequestHandler::HandleToggleStudioMode },

	{ "SetTextGDIPlusProperties", WSRequestHandler::HandleSetTextGDIPlusProperties },
	{ "GetTextGDIPlusProperties", WSRequestHandler::HandleGetTextGDIPlusProperties },

	{ "SetTextFreetype2Properties", WSRequestHandler::HandleSetTextFreetype2Properties },
	{ "GetTextFreetype2Properties", WSRequestHandler::HandleGetTextFreetype2Properties },

	{ "GetBrowserSourceProperties", WSRequestHandler::HandleGetBrowserSourceProperties },
	{ "SetBrowserSourceProperties", WSRequestHandler::HandleSetBrowserSourceProperties },

	{ "ListOutputs", WSRequestHandler::HandleListOutputs },
	{ "GetOutputInfo", WSRequestHandler::HandleGetOutputInfo },
	{ "StartOutput", WSRequestHandler::HandleStartOutput },
	{ "StopOutput", WSRequestHandler::HandleStopOutput }
};

QSet<QString> WSRequestHandler::authNotRequired {
	"GetVersion",
	"GetAuthRequired",
	"Authenticate"
};

WSRequestHandler::WSRequestHandler(ConnectionProperties& connProperties) :
	_messageId(0),
	_requestType(""),
	data(nullptr),
	_connProperties(connProperties)
{
}

std::string WSRequestHandler::processIncomingMessage(std::string& textMessage) {
	if (GetConfig()->DebugEnabled) {
		blog(LOG_INFO, "Request >> '%s'", textMessage.c_str());
	}

	OBSDataAutoRelease responseData = processRequest(textMessage);
	std::string response = obs_data_get_json(responseData);

	if (GetConfig()->DebugEnabled) {
		blog(LOG_INFO, "Response << '%s'", response.c_str());
	}

	return response;
}

HandlerResponse WSRequestHandler::processRequest(std::string& textMessage){
	std::string msgContainer(textMessage);
	const char* msg = msgContainer.c_str();

	data = obs_data_create_from_json(msg);
	if (!data) {
		blog(LOG_ERROR, "invalid JSON payload received for '%s'", msg);
		return SendErrorResponse("invalid JSON payload");
	}

	if (!hasField("request-type") || !hasField("message-id")) {
		return SendErrorResponse("missing request parameters");
	}

	_requestType = obs_data_get_string(data, "request-type");
	_messageId = obs_data_get_string(data, "message-id");

	if (GetConfig()->AuthRequired
		&& (!authNotRequired.contains(_requestType))
		&& (!_connProperties.isAuthenticated()))
	{
		return SendErrorResponse("Not Authenticated");
	}

	HandlerResponse (*handlerFunc)(WSRequestHandler*) = (messageMap[_requestType]);
	if (!handlerFunc) {
		return SendErrorResponse("invalid request type");
	}

	return handlerFunc(this);
}

WSRequestHandler::~WSRequestHandler() {
}

HandlerResponse WSRequestHandler::SendOKResponse(obs_data_t* additionalFields) {
	return SendResponse("ok", additionalFields);
}

HandlerResponse WSRequestHandler::SendErrorResponse(QString errorMessage) {
	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "error", errorMessage.toUtf8().constData());

	return SendResponse("error", fields);
}

HandlerResponse WSRequestHandler::SendErrorResponse(obs_data_t* additionalFields) {
	return SendResponse("error", additionalFields);
}

HandlerResponse WSRequestHandler::SendResponse(const char* status, obs_data_t* fields) {
	obs_data_t* response = obs_data_create();
	obs_data_set_string(response, "message-id", _messageId);
	obs_data_set_string(response, "status", status);

	if (fields) {
		obs_data_apply(response, fields);
	}

	return response;
}

bool WSRequestHandler::hasField(QString name) {
	if (!data || name.isEmpty() || name.isNull())
		return false;

	return obs_data_has_user_value(data, name.toUtf8());
}
