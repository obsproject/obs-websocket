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

#include "obs-websocket.h"

typedef obs_data_t* HandlerResponse;

class WSRequestHandler : public QObject {
	Q_OBJECT

	public:
		explicit WSRequestHandler(ConnectionProperties& connProperties);
		~WSRequestHandler();
		std::string processIncomingMessage(std::string& textMessage);
		bool hasField(QString name);

	private:
		const char* _messageId;
		const char* _requestType;
		ConnectionProperties& _connProperties;
		OBSDataAutoRelease data;

		HandlerResponse processRequest(std::string& textMessage);

		HandlerResponse SendOKResponse(obs_data_t* additionalFields = nullptr);
		HandlerResponse SendErrorResponse(const char* errorMessage);
		HandlerResponse SendErrorResponse(obs_data_t* additionalFields = nullptr);
		HandlerResponse SendResponse(const char* status, obs_data_t* additionalFields = nullptr);

		static QHash<QString, HandlerResponse(*)(WSRequestHandler*)> messageMap;
		static QSet<QString> authNotRequired;

		static HandlerResponse HandleGetVersion(WSRequestHandler* req);
		static HandlerResponse HandleGetAuthRequired(WSRequestHandler* req);
		static HandlerResponse HandleAuthenticate(WSRequestHandler* req);

		static HandlerResponse HandleGetStats(WSRequestHandler* req);
		static HandlerResponse HandleSetHeartbeat(WSRequestHandler* req);
		static HandlerResponse HandleGetVideoInfo(WSRequestHandler* req);

		static HandlerResponse HandleSetFilenameFormatting(WSRequestHandler* req);
		static HandlerResponse HandleGetFilenameFormatting(WSRequestHandler* req);

		static HandlerResponse HandleSetCurrentScene(WSRequestHandler* req);
		static HandlerResponse HandleGetCurrentScene(WSRequestHandler* req);
		static HandlerResponse HandleGetSceneList(WSRequestHandler* req);

		static HandlerResponse HandleSetSceneItemRender(WSRequestHandler* req);
		static HandlerResponse HandleSetSceneItemPosition(WSRequestHandler* req);
		static HandlerResponse HandleSetSceneItemTransform(WSRequestHandler* req);
		static HandlerResponse HandleSetSceneItemCrop(WSRequestHandler* req);
		static HandlerResponse HandleGetSceneItemProperties(WSRequestHandler* req);
		static HandlerResponse HandleSetSceneItemProperties(WSRequestHandler* req);
		static HandlerResponse HandleResetSceneItem(WSRequestHandler* req);
		static HandlerResponse HandleDuplicateSceneItem(WSRequestHandler* req);
		static HandlerResponse HandleDeleteSceneItem(WSRequestHandler* req);
		static HandlerResponse HandleReorderSceneItems(WSRequestHandler* req);

		static HandlerResponse HandleAddNewScene(WSRequestHandler * req);

		static HandlerResponse HandleGetStreamingStatus(WSRequestHandler* req);
		static HandlerResponse HandleStartStopStreaming(WSRequestHandler* req);
		static HandlerResponse HandleStartStopRecording(WSRequestHandler* req);
		static HandlerResponse HandleStartStreaming(WSRequestHandler* req);
		static HandlerResponse HandleStopStreaming(WSRequestHandler* req);
		static HandlerResponse HandleStartRecording(WSRequestHandler* req);
		static HandlerResponse HandleStopRecording(WSRequestHandler* req);

		static HandlerResponse HandleStartStopReplayBuffer(WSRequestHandler* req);
		static HandlerResponse HandleStartReplayBuffer(WSRequestHandler* req);
		static HandlerResponse HandleStopReplayBuffer(WSRequestHandler* req);
		static HandlerResponse HandleSaveReplayBuffer(WSRequestHandler* req);

		static HandlerResponse HandleSetRecordingFolder(WSRequestHandler* req);
		static HandlerResponse HandleGetRecordingFolder(WSRequestHandler* req);

		static HandlerResponse HandleGetTransitionList(WSRequestHandler* req);
		static HandlerResponse HandleGetCurrentTransition(WSRequestHandler* req);
		static HandlerResponse HandleSetCurrentTransition(WSRequestHandler* req);

		static HandlerResponse HandleSetVolume(WSRequestHandler* req);
		static HandlerResponse HandleGetVolume(WSRequestHandler* req);
		static HandlerResponse HandleToggleMute(WSRequestHandler* req);
		static HandlerResponse HandleSetMute(WSRequestHandler* req);
		static HandlerResponse HandleGetMute(WSRequestHandler* req);
		static HandlerResponse HandleSetSyncOffset(WSRequestHandler* req);
		static HandlerResponse HandleGetSyncOffset(WSRequestHandler* req);
		static HandlerResponse HandleGetSpecialSources(WSRequestHandler* req);
		static HandlerResponse HandleGetSourcesList(WSRequestHandler* req);
		static HandlerResponse HandleGetSourceTypesList(WSRequestHandler* req);
		static HandlerResponse HandleGetSourceSettings(WSRequestHandler* req);
		static HandlerResponse HandleSetSourceSettings(WSRequestHandler* req);
		static HandlerResponse HandleTakeSourceScreenshot(WSRequestHandler* req);

		static HandlerResponse HandleCreateNewSource(WSRequestHandler * req);

		static HandlerResponse HandleGetSourceFilters(WSRequestHandler* req);
		static HandlerResponse HandleAddFilterToSource(WSRequestHandler* req);
		static HandlerResponse HandleRemoveFilterFromSource(WSRequestHandler* req);
		static HandlerResponse HandleReorderSourceFilter(WSRequestHandler* req);
		static HandlerResponse HandleMoveSourceFilter(WSRequestHandler* req);
		static HandlerResponse HandleSetSourceFilterSettings(WSRequestHandler* req);

		static HandlerResponse HandleSetCurrentSceneCollection(WSRequestHandler* req);
		static HandlerResponse HandleGetCurrentSceneCollection(WSRequestHandler* req);
		static HandlerResponse HandleListSceneCollections(WSRequestHandler* req);

		static HandlerResponse HandleSetCurrentProfile(WSRequestHandler* req);
		static HandlerResponse HandleGetCurrentProfile(WSRequestHandler* req);
		static HandlerResponse HandleListProfiles(WSRequestHandler* req);

		static HandlerResponse HandleSetStreamSettings(WSRequestHandler* req);
		static HandlerResponse HandleGetStreamSettings(WSRequestHandler* req);
		static HandlerResponse HandleSaveStreamSettings(WSRequestHandler* req);
#if BUILD_CAPTIONS
		static HandlerResponse HandleSendCaptions(WSRequestHandler * req);
#endif

		static HandlerResponse HandleSetTransitionDuration(WSRequestHandler* req);
		static HandlerResponse HandleGetTransitionDuration(WSRequestHandler* req);

		static HandlerResponse HandleGetStudioModeStatus(WSRequestHandler* req);
		static HandlerResponse HandleGetPreviewScene(WSRequestHandler* req);
		static HandlerResponse HandleSetPreviewScene(WSRequestHandler* req);
		static HandlerResponse HandleTransitionToProgram(WSRequestHandler* req);
		static HandlerResponse HandleEnableStudioMode(WSRequestHandler* req);
		static HandlerResponse HandleDisableStudioMode(WSRequestHandler* req);
		static HandlerResponse HandleToggleStudioMode(WSRequestHandler* req);

		static HandlerResponse HandleSetTextGDIPlusProperties(WSRequestHandler* req);
		static HandlerResponse HandleGetTextGDIPlusProperties(WSRequestHandler* req);

		static HandlerResponse HandleSetTextFreetype2Properties(WSRequestHandler* req);
		static HandlerResponse HandleGetTextFreetype2Properties(WSRequestHandler* req);

		static HandlerResponse HandleSetBrowserSourceProperties(WSRequestHandler* req);
		static HandlerResponse HandleGetBrowserSourceProperties(WSRequestHandler* req);
};
