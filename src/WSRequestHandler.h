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

#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QVariantHash>
#include <QtCore/QString>
#include <QtCore/QSharedPointer>

#include <obs.hpp>
#include <obs-frontend-api.h>

#include "ConnectionProperties.h"

#include "rpc/RpcRequest.h"
#include "rpc/RpcResponse.h"

#include "obs-websocket.h"

class WSRequestHandler {
	public:
		explicit WSRequestHandler(ConnectionProperties& connProperties);
		RpcResponse processRequest(const RpcRequest& textMessage);

	private:
		ConnectionProperties& _connProperties;

		static QHash<QString, RpcResponse(*)(const RpcRequest&)> messageMap;
		static QSet<QString> authNotRequired;

		static RpcResponse HandleGetVersion(const RpcRequest&);
		static RpcResponse HandleGetAuthRequired(const RpcRequest&);
		static RpcResponse HandleAuthenticate(const RpcRequest&);

		static RpcResponse HandleGetStats(const RpcRequest&);
		static RpcResponse HandleSetHeartbeat(const RpcRequest&);
		static RpcResponse HandleGetVideoInfo(const RpcRequest&);

		static RpcResponse HandleSetFilenameFormatting(const RpcRequest&);
		static RpcResponse HandleGetFilenameFormatting(const RpcRequest&);

		static RpcResponse HandleBroadcastCustomMessage(const RpcRequest&);

		static RpcResponse HandleSetCurrentScene(const RpcRequest&);
		static RpcResponse HandleGetCurrentScene(const RpcRequest&);
		static RpcResponse HandleGetSceneList(const RpcRequest&);

		static RpcResponse HandleSetSceneItemRender(const RpcRequest&);
		static RpcResponse HandleSetSceneItemPosition(const RpcRequest&);
		static RpcResponse HandleSetSceneItemTransform(const RpcRequest&);
		static RpcResponse HandleSetSceneItemCrop(const RpcRequest&);
		static RpcResponse HandleGetSceneItemProperties(const RpcRequest&);
		static RpcResponse HandleSetSceneItemProperties(const RpcRequest&);
		static RpcResponse HandleResetSceneItem(const RpcRequest&);
		static RpcResponse HandleDuplicateSceneItem(const RpcRequest&);
		static RpcResponse HandleDeleteSceneItem(const RpcRequest&);
		static RpcResponse HandleReorderSceneItems(const RpcRequest&);

		static RpcResponse HandleGetStreamingStatus(const RpcRequest&);
		static RpcResponse HandleStartStopStreaming(const RpcRequest&);
		static RpcResponse HandleStartStopRecording(const RpcRequest&);

		static RpcResponse HandleStartStreaming(const RpcRequest&);
		static RpcResponse HandleStopStreaming(const RpcRequest&);

		static RpcResponse HandleStartRecording(const RpcRequest&);
		static RpcResponse HandleStopRecording(const RpcRequest&);
		static RpcResponse HandlePauseRecording(const RpcRequest&);
		static RpcResponse HandleResumeRecording(const RpcRequest&);

		static RpcResponse HandleStartStopReplayBuffer(const RpcRequest&);
		static RpcResponse HandleStartReplayBuffer(const RpcRequest&);
		static RpcResponse HandleStopReplayBuffer(const RpcRequest&);
		static RpcResponse HandleSaveReplayBuffer(const RpcRequest&);

		static RpcResponse HandleSetRecordingFolder(const RpcRequest&);
		static RpcResponse HandleGetRecordingFolder(const RpcRequest&);

		static RpcResponse HandleGetTransitionList(const RpcRequest&);
		static RpcResponse HandleGetCurrentTransition(const RpcRequest&);
		static RpcResponse HandleSetCurrentTransition(const RpcRequest&);

		static RpcResponse HandleSetVolume(const RpcRequest&);
		static RpcResponse HandleGetVolume(const RpcRequest&);
		static RpcResponse HandleToggleMute(const RpcRequest&);
		static RpcResponse HandleSetMute(const RpcRequest&);
		static RpcResponse HandleGetMute(const RpcRequest&);
		static RpcResponse HandleSetSyncOffset(const RpcRequest&);
		static RpcResponse HandleGetSyncOffset(const RpcRequest&);
		static RpcResponse HandleGetSpecialSources(const RpcRequest&);
		static RpcResponse HandleGetSourcesList(const RpcRequest&);
		static RpcResponse HandleGetSourceTypesList(const RpcRequest&);
		static RpcResponse HandleGetSourceSettings(const RpcRequest&);
		static RpcResponse HandleSetSourceSettings(const RpcRequest&);
		static RpcResponse HandleTakeSourceScreenshot(const RpcRequest&);

		static RpcResponse HandleGetSourceFilters(const RpcRequest&);
		static RpcResponse HandleGetSourceFilterInfo(const RpcRequest&);
		static RpcResponse HandleAddFilterToSource(const RpcRequest&);
		static RpcResponse HandleRemoveFilterFromSource(const RpcRequest&);
		static RpcResponse HandleReorderSourceFilter(const RpcRequest&);
		static RpcResponse HandleMoveSourceFilter(const RpcRequest&);
		static RpcResponse HandleSetSourceFilterSettings(const RpcRequest&);
		static RpcResponse HandleSetSourceFilterVisibility(const RpcRequest&);

		static RpcResponse HandleSetCurrentSceneCollection(const RpcRequest&);
		static RpcResponse HandleGetCurrentSceneCollection(const RpcRequest&);
		static RpcResponse HandleListSceneCollections(const RpcRequest&);

		static RpcResponse HandleSetCurrentProfile(const RpcRequest&);
		static RpcResponse HandleGetCurrentProfile(const RpcRequest&);
		static RpcResponse HandleListProfiles(const RpcRequest&);

		static RpcResponse HandleSetStreamSettings(const RpcRequest&);
		static RpcResponse HandleGetStreamSettings(const RpcRequest&);
		static RpcResponse HandleSaveStreamSettings(const RpcRequest&);
#if BUILD_CAPTIONS
		static RpcResponse HandleSendCaptions(WSRequestHandler * req);
#endif

		static RpcResponse HandleSetTransitionDuration(const RpcRequest& request);
		static RpcResponse HandleGetTransitionDuration(const RpcRequest& request);

		static RpcResponse HandleGetStudioModeStatus(const RpcRequest& request);
		static RpcResponse HandleGetPreviewScene(const RpcRequest& request);
		static RpcResponse HandleSetPreviewScene(const RpcRequest& request);
		static RpcResponse HandleTransitionToProgram(const RpcRequest& request);
		static RpcResponse HandleEnableStudioMode(const RpcRequest& request);
		static RpcResponse HandleDisableStudioMode(const RpcRequest& request);
		static RpcResponse HandleToggleStudioMode(const RpcRequest& request);

		static RpcResponse HandleSetTextGDIPlusProperties(const RpcRequest& request);
		static RpcResponse HandleGetTextGDIPlusProperties(const RpcRequest& request);

		static RpcResponse HandleSetTextFreetype2Properties(const RpcRequest& request);
		static RpcResponse HandleGetTextFreetype2Properties(const RpcRequest& request);

		static RpcResponse HandleSetBrowserSourceProperties(const RpcRequest& request);
		static RpcResponse HandleGetBrowserSourceProperties(const RpcRequest& request);

		static RpcResponse HandleListOutputs(const RpcRequest& request);
		static RpcResponse HandleGetOutputInfo(const RpcRequest& request);
		static RpcResponse HandleStartOutput(const RpcRequest& request);
		static RpcResponse HandleStopOutput(const RpcRequest& request);
};
