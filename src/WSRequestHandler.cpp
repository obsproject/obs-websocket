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

const QHash<QString, RpcMethodHandler> WSRequestHandler::messageMap{
	// Category: General
	{ "GetVersion", &WSRequestHandler::GetVersion },
	{ "GetAuthRequired", &WSRequestHandler::GetAuthRequired },
	{ "Authenticate", &WSRequestHandler::Authenticate },
	{ "SetHeartbeat", &WSRequestHandler::SetHeartbeat },
	{ "SetFilenameFormatting", &WSRequestHandler::SetFilenameFormatting },
	{ "GetFilenameFormatting", &WSRequestHandler::GetFilenameFormatting },
	{ "GetStats", &WSRequestHandler::GetStats },
	{ "BroadcastCustomMessage", &WSRequestHandler::BroadcastCustomMessage },
	{ "GetVideoInfo", &WSRequestHandler::GetVideoInfo },
	{ "OpenProjector", &WSRequestHandler::OpenProjector },
	{ "TriggerHotkeyByName", &WSRequestHandler::TriggerHotkeyByName },
	{ "TriggerHotkeyBySequence", &WSRequestHandler::TriggerHotkeyBySequence },
	{ "ExecuteBatch", &WSRequestHandler::ExecuteBatch },
	{ "Sleep", &WSRequestHandler::Sleep },

	// Category: Media Control
	{ "PlayPauseMedia", &WSRequestHandler::PlayPauseMedia },
	{ "RestartMedia", &WSRequestHandler::RestartMedia },
	{ "StopMedia", &WSRequestHandler::StopMedia },
	{ "NextMedia", &WSRequestHandler::NextMedia },
	{ "PreviousMedia", &WSRequestHandler::PreviousMedia },
	{ "GetMediaDuration", &WSRequestHandler::GetMediaDuration },
	{ "GetMediaTime", &WSRequestHandler::GetMediaTime },
	{ "SetMediaTime", &WSRequestHandler::SetMediaTime },
	{ "ScrubMedia", &WSRequestHandler::ScrubMedia },
	{ "GetMediaState", &WSRequestHandler::GetMediaState },
	{ "GetMediaSourcesList", &WSRequestHandler::GetMediaSourcesList },

	// Category: Outputs
	{ "ListOutputs", &WSRequestHandler::ListOutputs },
	{ "GetOutputInfo", &WSRequestHandler::GetOutputInfo },
	{ "StartOutput", &WSRequestHandler::StartOutput },
	{ "StopOutput", &WSRequestHandler::StopOutput },

	// Category: Profiles
	{ "SetCurrentProfile", &WSRequestHandler::SetCurrentProfile },
	{ "GetCurrentProfile", &WSRequestHandler::GetCurrentProfile },
	{ "ListProfiles", &WSRequestHandler::ListProfiles },

	// Category: Recording
	{ "GetRecordingStatus", &WSRequestHandler::GetRecordingStatus },
	{ "StartStopRecording", &WSRequestHandler::StartStopRecording },
	{ "StartRecording", &WSRequestHandler::StartRecording },
	{ "StopRecording", &WSRequestHandler::StopRecording },
	{ "PauseRecording", &WSRequestHandler::PauseRecording },
	{ "ResumeRecording", &WSRequestHandler::ResumeRecording },
	{ "SetRecordingFolder", &WSRequestHandler::SetRecordingFolder },
	{ "GetRecordingFolder", &WSRequestHandler::GetRecordingFolder },

	// Category: Replay Buffer
	{ "GetReplayBufferStatus", &WSRequestHandler::GetReplayBufferStatus },
	{ "StartStopReplayBuffer", &WSRequestHandler::StartStopReplayBuffer },
	{ "StartReplayBuffer", &WSRequestHandler::StartReplayBuffer },
	{ "StopReplayBuffer", &WSRequestHandler::StopReplayBuffer },
	{ "SaveReplayBuffer", &WSRequestHandler::SaveReplayBuffer },

	// Category: Scene Collections
	{ "SetCurrentSceneCollection", &WSRequestHandler::SetCurrentSceneCollection },
	{ "GetCurrentSceneCollection", &WSRequestHandler::GetCurrentSceneCollection },
	{ "ListSceneCollections", &WSRequestHandler::ListSceneCollections },

	// Category: Scene Items
	{ "GetSceneItemList", &WSRequestHandler::GetSceneItemList },
	{ "GetSceneItemProperties", &WSRequestHandler::GetSceneItemProperties },
	{ "SetSceneItemProperties", &WSRequestHandler::SetSceneItemProperties },
	{ "ResetSceneItem", &WSRequestHandler::ResetSceneItem },
	{ "SetSceneItemRender", &WSRequestHandler::SetSceneItemRender },
	{ "SetSceneItemPosition", &WSRequestHandler::SetSceneItemPosition },
	{ "SetSceneItemTransform", &WSRequestHandler::SetSceneItemTransform },
	{ "SetSceneItemCrop", &WSRequestHandler::SetSceneItemCrop },
	{ "SetSourceRender", &WSRequestHandler::SetSceneItemRender }, // Retrocompat TODO: Remove in 5.0.0
	{ "DeleteSceneItem", &WSRequestHandler::DeleteSceneItem },
	{ "AddSceneItem", &WSRequestHandler::AddSceneItem },
	{ "DuplicateSceneItem", &WSRequestHandler::DuplicateSceneItem },

	// Category: Scenes
	{ "SetCurrentScene", &WSRequestHandler::SetCurrentScene },
	{ "GetCurrentScene", &WSRequestHandler::GetCurrentScene },
	{ "GetSceneList", &WSRequestHandler::GetSceneList },
	{ "CreateScene", &WSRequestHandler::CreateScene },
	{ "ReorderSceneItems", &WSRequestHandler::ReorderSceneItems },
	{ "SetSceneTransitionOverride", &WSRequestHandler::SetSceneTransitionOverride },
	{ "RemoveSceneTransitionOverride", &WSRequestHandler::RemoveSceneTransitionOverride },
	{ "GetSceneTransitionOverride", &WSRequestHandler::GetSceneTransitionOverride },

	// Category: Sources
	{ "CreateSource", &WSRequestHandler::CreateSource },
	{ "GetSourcesList", &WSRequestHandler::GetSourcesList },
	{ "GetSourceTypesList", &WSRequestHandler::GetSourceTypesList },
	{ "GetVolume", &WSRequestHandler::GetVolume },
	{ "SetVolume", &WSRequestHandler::SetVolume },
	{ "SetAudioTracks", &WSRequestHandler::SetAudioTracks },
	{ "GetAudioTracks", &WSRequestHandler::GetAudioTracks },
	{ "GetMute", &WSRequestHandler::GetMute },
	{ "SetMute", &WSRequestHandler::SetMute },
	{ "ToggleMute", &WSRequestHandler::ToggleMute },
	{ "GetSourceActive", &WSRequestHandler::GetSourceActive },
	{ "GetAudioActive", &WSRequestHandler::GetAudioActive },
	{ "SetSourceName", &WSRequestHandler::SetSourceName },
	{ "SetSyncOffset", &WSRequestHandler::SetSyncOffset },
	{ "GetSyncOffset", &WSRequestHandler::GetSyncOffset },
	{ "GetSourceSettings", &WSRequestHandler::GetSourceSettings },
	{ "SetSourceSettings", &WSRequestHandler::SetSourceSettings },
	{ "GetTextGDIPlusProperties", &WSRequestHandler::GetTextGDIPlusProperties },
	{ "SetTextGDIPlusProperties", &WSRequestHandler::SetTextGDIPlusProperties },
	{ "GetTextFreetype2Properties", &WSRequestHandler::GetTextFreetype2Properties },
	{ "SetTextFreetype2Properties", &WSRequestHandler::SetTextFreetype2Properties },
	{ "GetBrowserSourceProperties", &WSRequestHandler::GetBrowserSourceProperties },
	{ "SetBrowserSourceProperties", &WSRequestHandler::SetBrowserSourceProperties },
	{ "GetSpecialSources", &WSRequestHandler::GetSpecialSources },
	{ "GetSourceFilters", &WSRequestHandler::GetSourceFilters },
	{ "GetSourceFilterInfo", &WSRequestHandler::GetSourceFilterInfo },
	{ "AddFilterToSource", &WSRequestHandler::AddFilterToSource },
	{ "RemoveFilterFromSource", &WSRequestHandler::RemoveFilterFromSource },
	{ "ReorderSourceFilter", &WSRequestHandler::ReorderSourceFilter },
	{ "MoveSourceFilter", &WSRequestHandler::MoveSourceFilter },
	{ "SetSourceFilterSettings", &WSRequestHandler::SetSourceFilterSettings },
	{ "SetSourceFilterVisibility", &WSRequestHandler::SetSourceFilterVisibility },
	{ "GetAudioMonitorType", &WSRequestHandler::GetAudioMonitorType },
	{ "SetAudioMonitorType", &WSRequestHandler::SetAudioMonitorType },
	{ "GetSourceDefaultSettings", &WSRequestHandler::GetSourceDefaultSettings },
	{ "TakeSourceScreenshot", &WSRequestHandler::TakeSourceScreenshot },
	{ "RefreshBrowserSource", &WSRequestHandler::RefreshBrowserSource },

	// Category: Streaming
	{ "GetStreamingStatus", &WSRequestHandler::GetStreamingStatus },
	{ "StartStopStreaming", &WSRequestHandler::StartStopStreaming },
	{ "StartStreaming", &WSRequestHandler::StartStreaming },
	{ "StopStreaming", &WSRequestHandler::StopStreaming },
	{ "SetStreamSettings", &WSRequestHandler::SetStreamSettings },
	{ "GetStreamSettings", &WSRequestHandler::GetStreamSettings },
	{ "SaveStreamSettings", &WSRequestHandler::SaveStreamSettings },
	{ "SendCaptions", &WSRequestHandler::SendCaptions },
	
	// Category: VirtualCam
	{ "GetVirtualCamStatus", &WSRequestHandler::GetVirtualCamStatus },
	{ "StartStopVirtualCam", &WSRequestHandler::StartStopVirtualCam },
	{ "StartVirtualCam", &WSRequestHandler::StartVirtualCam },
	{ "StopVirtualCam", &WSRequestHandler::StopVirtualCam },

	// Category: Studio Mode
	{ "GetStudioModeStatus", &WSRequestHandler::GetStudioModeStatus },
	{ "GetPreviewScene", &WSRequestHandler::GetPreviewScene },
	{ "SetPreviewScene", &WSRequestHandler::SetPreviewScene },
	{ "TransitionToProgram", &WSRequestHandler::TransitionToProgram },
	{ "EnableStudioMode", &WSRequestHandler::EnableStudioMode },
	{ "DisableStudioMode", &WSRequestHandler::DisableStudioMode },
	{ "ToggleStudioMode", &WSRequestHandler::ToggleStudioMode },

	// Category: Transitions
	{ "GetTransitionList", &WSRequestHandler::GetTransitionList },
	{ "GetCurrentTransition", &WSRequestHandler::GetCurrentTransition },
	{ "SetCurrentTransition", &WSRequestHandler::SetCurrentTransition },
	{ "SetTransitionDuration", &WSRequestHandler::SetTransitionDuration },
	{ "GetTransitionDuration", &WSRequestHandler::GetTransitionDuration },
	{ "GetTransitionPosition", &WSRequestHandler::GetTransitionPosition },
	{ "GetTransitionSettings", &WSRequestHandler::GetTransitionSettings },
	{ "SetTransitionSettings", &WSRequestHandler::SetTransitionSettings },
	{ "ReleaseTBar", &WSRequestHandler::ReleaseTBar	},
	{ "SetTBarPosition", &WSRequestHandler::SetTBarPosition	}
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

RpcResponse WSRequestHandler::processRequest(const RpcRequest& request) {
	auto config = GetConfig();
	if ((config && config->AuthRequired)
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
