/*
obs-websocket
Copyright (C) 2016-2019	Stéphane Lepin <stephane.lepin@gmail.com>
Copyright (C) 2017	Mikhail Swift <https://github.com/mikhailswift>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#pragma once

#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QSet>

#include <obs.hpp>
#include <obs-frontend-api.h>

#include "ConnectionProperties.h"

#include "rpc/RpcRequest.h"
#include "rpc/RpcResponse.h"

#include "obs-websocket.h"

class WSRequestHandler;
typedef RpcResponse(WSRequestHandler::*RpcMethodHandler)(const RpcRequest&);

class WSRequestHandler {
	public:
		explicit WSRequestHandler(ConnectionProperties& connProperties);
		RpcResponse processRequest(const RpcRequest& textMessage);

	private:
		ConnectionProperties& _connProperties;

		static const QHash<QString, RpcMethodHandler> messageMap;
		static const QSet<QString> authNotRequired;

		// Category: General
		RpcResponse GetVersion(const RpcRequest&);
		RpcResponse GetAuthRequired(const RpcRequest&);
		RpcResponse Authenticate(const RpcRequest&);
		RpcResponse SetHeartbeat(const RpcRequest&);
		RpcResponse SetFilenameFormatting(const RpcRequest&);
		RpcResponse GetFilenameFormatting(const RpcRequest&);
		RpcResponse GetStats(const RpcRequest&);
		RpcResponse BroadcastCustomMessage(const RpcRequest&);
		RpcResponse GetVideoInfo(const RpcRequest&);
		RpcResponse OpenProjector(const RpcRequest&);
		RpcResponse TriggerHotkeyByName(const RpcRequest&);
		RpcResponse TriggerHotkeyBySequence(const RpcRequest&);
		RpcResponse ExecuteBatch(const RpcRequest&);
		RpcResponse Sleep(const RpcRequest&);

		// Category: Media Control
		RpcResponse PlayPauseMedia(const RpcRequest&);
		RpcResponse RestartMedia(const RpcRequest&);
		RpcResponse StopMedia(const RpcRequest&);
		RpcResponse NextMedia(const RpcRequest&);
		RpcResponse PreviousMedia(const RpcRequest&);
		RpcResponse GetMediaDuration(const RpcRequest&);
		RpcResponse GetMediaTime(const RpcRequest&);
		RpcResponse SetMediaTime(const RpcRequest&);
		RpcResponse ScrubMedia(const RpcRequest&);
		RpcResponse GetMediaState(const RpcRequest&);
		RpcResponse GetMediaSourcesList(const RpcRequest&);

		// Category: Outputs
		RpcResponse ListOutputs(const RpcRequest&);
		RpcResponse GetOutputInfo(const RpcRequest&);
		RpcResponse StartOutput(const RpcRequest&);
		RpcResponse StopOutput(const RpcRequest&);

		// Category: Profiles
		RpcResponse SetCurrentProfile(const RpcRequest&);
		RpcResponse GetCurrentProfile(const RpcRequest&);
		RpcResponse ListProfiles(const RpcRequest&);

		// Category: Recording
		RpcResponse GetRecordingStatus(const RpcRequest&);
		RpcResponse StartStopRecording(const RpcRequest&);
		RpcResponse StartRecording(const RpcRequest&);
		RpcResponse StopRecording(const RpcRequest&);
		RpcResponse PauseRecording(const RpcRequest&);
		RpcResponse ResumeRecording(const RpcRequest&);
		RpcResponse SetRecordingFolder(const RpcRequest&);
		RpcResponse GetRecordingFolder(const RpcRequest&);

		// Category: Replay Buffer
		RpcResponse GetReplayBufferStatus(const RpcRequest&);
		RpcResponse StartStopReplayBuffer(const RpcRequest&);
		RpcResponse StartReplayBuffer(const RpcRequest&);
		RpcResponse StopReplayBuffer(const RpcRequest&);
		RpcResponse SaveReplayBuffer(const RpcRequest&);

		// Category: Scene Collections
		RpcResponse SetCurrentSceneCollection(const RpcRequest&);
		RpcResponse GetCurrentSceneCollection(const RpcRequest&);
		RpcResponse ListSceneCollections(const RpcRequest&);

		// Category: Scene Items
		RpcResponse GetSceneItemList(const RpcRequest&);
		RpcResponse GetSceneItemProperties(const RpcRequest&);
		RpcResponse SetSceneItemProperties(const RpcRequest&);
		RpcResponse ResetSceneItem(const RpcRequest&);
		RpcResponse SetSceneItemRender(const RpcRequest&);
		RpcResponse SetSceneItemPosition(const RpcRequest&);
		RpcResponse SetSceneItemTransform(const RpcRequest&);
		RpcResponse SetSceneItemCrop(const RpcRequest&);
		RpcResponse DeleteSceneItem(const RpcRequest&);
		RpcResponse AddSceneItem(const RpcRequest&);
		RpcResponse DuplicateSceneItem(const RpcRequest&);

		// Category: Scenes
		RpcResponse SetCurrentScene(const RpcRequest&);
		RpcResponse GetCurrentScene(const RpcRequest&);
		RpcResponse GetSceneList(const RpcRequest&);
		RpcResponse CreateScene(const RpcRequest&);
		RpcResponse ReorderSceneItems(const RpcRequest&);
		RpcResponse SetSceneTransitionOverride(const RpcRequest&);
		RpcResponse RemoveSceneTransitionOverride(const RpcRequest&);
		RpcResponse GetSceneTransitionOverride(const RpcRequest&);

		// Category: Sources
		RpcResponse CreateSource(const RpcRequest&);
		RpcResponse GetSourcesList(const RpcRequest&);
		RpcResponse GetSourceTypesList(const RpcRequest&);
		RpcResponse GetVolume(const RpcRequest&);
		RpcResponse SetVolume(const RpcRequest&);
		RpcResponse GetMute(const RpcRequest&);
		RpcResponse SetMute(const RpcRequest&);
		RpcResponse ToggleMute(const RpcRequest&);
		RpcResponse GetAudioActive(const RpcRequest&);
		RpcResponse SetSourceName(const RpcRequest&);
		RpcResponse SetSyncOffset(const RpcRequest&);
		RpcResponse GetSyncOffset(const RpcRequest&);
		RpcResponse GetSourceSettings(const RpcRequest&);
		RpcResponse SetSourceSettings(const RpcRequest&);
		RpcResponse GetTextGDIPlusProperties(const RpcRequest&);
		RpcResponse SetTextGDIPlusProperties(const RpcRequest&);
		RpcResponse GetTextFreetype2Properties(const RpcRequest&);
		RpcResponse SetTextFreetype2Properties(const RpcRequest&);
		RpcResponse GetBrowserSourceProperties(const RpcRequest&);
		RpcResponse SetBrowserSourceProperties(const RpcRequest&);
		RpcResponse GetSpecialSources(const RpcRequest&);
		RpcResponse GetSourceFilters(const RpcRequest&);
		RpcResponse GetSourceFilterInfo(const RpcRequest&);
		RpcResponse AddFilterToSource(const RpcRequest&);
		RpcResponse RemoveFilterFromSource(const RpcRequest&);
		RpcResponse ReorderSourceFilter(const RpcRequest&);
		RpcResponse MoveSourceFilter(const RpcRequest&);
		RpcResponse SetSourceFilterSettings(const RpcRequest&);
		RpcResponse SetSourceFilterVisibility(const RpcRequest&);
		RpcResponse GetAudioMonitorType(const RpcRequest&);
		RpcResponse SetAudioMonitorType(const RpcRequest&);
		RpcResponse GetSourceDefaultSettings(const RpcRequest&);
		RpcResponse TakeSourceScreenshot(const RpcRequest&);
		RpcResponse RefreshBrowserSource(const RpcRequest&);

		// Category: Streaming
		RpcResponse GetStreamingStatus(const RpcRequest&);
		RpcResponse StartStopStreaming(const RpcRequest&);
		RpcResponse StartStreaming(const RpcRequest&);
		RpcResponse StopStreaming(const RpcRequest&);
		RpcResponse SetStreamSettings(const RpcRequest&);
		RpcResponse GetStreamSettings(const RpcRequest&);
		RpcResponse SaveStreamSettings(const RpcRequest&);
		RpcResponse SendCaptions(const RpcRequest&);
		
		// Category: Virtual Cam
		RpcResponse GetVirtualCamStatus(const RpcRequest&);
		RpcResponse StartStopVirtualCam(const RpcRequest&);
		RpcResponse StartVirtualCam(const RpcRequest&);
		RpcResponse StopVirtualCam(const RpcRequest&);

		// Category: Studio Mode
		RpcResponse GetStudioModeStatus(const RpcRequest&);
		RpcResponse GetPreviewScene(const RpcRequest&);
		RpcResponse SetPreviewScene(const RpcRequest&);
		RpcResponse TransitionToProgram(const RpcRequest&);
		RpcResponse EnableStudioMode(const RpcRequest&);
		RpcResponse DisableStudioMode(const RpcRequest&);
		RpcResponse ToggleStudioMode(const RpcRequest&);

		// Category: Transitions
		RpcResponse GetTransitionList(const RpcRequest&);
		RpcResponse GetCurrentTransition(const RpcRequest&);
		RpcResponse SetCurrentTransition(const RpcRequest&);
		RpcResponse SetTransitionDuration(const RpcRequest&);
		RpcResponse GetTransitionDuration(const RpcRequest&);
		RpcResponse GetTransitionPosition(const RpcRequest&);
		RpcResponse GetTransitionSettings(const RpcRequest&);
		RpcResponse SetTransitionSettings(const RpcRequest&);
		RpcResponse ReleaseTBar(const RpcRequest&);
		RpcResponse SetTBarPosition(const RpcRequest&);
};
