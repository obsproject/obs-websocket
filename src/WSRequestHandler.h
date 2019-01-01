/*
obs-websocket
Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>
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

#ifndef WSREQUESTHANDLER_H
#define WSREQUESTHANDLER_H

#include <QHash>
#include <QSet>
#include <QVariantHash>

#include <obs.hpp>
#include <obs-frontend-api.h>

#include "obs-websocket.h"

class WSRequestHandler : public QObject {
	Q_OBJECT

	public:
		explicit WSRequestHandler(QVariantHash& connProperties);
		~WSRequestHandler();
		std::string processIncomingMessage(std::string& textMessage);
		bool hasField(QString name);

	private:
		const char* _messageId;
		const char* _requestType;
		QVariantHash& _connProperties;
		OBSDataAutoRelease data;

		std::string SendOKResponse(obs_data_t* additionalFields = NULL);
		std::string SendErrorResponse(const char* errorMessage);
		std::string SendErrorResponse(obs_data_t* additionalFields = NULL);
		std::string SendResponse(obs_data_t* response);

		static QHash<QString, std::string(*)(WSRequestHandler*)> messageMap;
		static QSet<QString> authNotRequired;

		static std::string HandleGetVersion(WSRequestHandler* req);
		static std::string HandleGetAuthRequired(WSRequestHandler* req);
		static std::string HandleAuthenticate(WSRequestHandler* req);

		static std::string HandleSetHeartbeat(WSRequestHandler* req);

		static std::string HandleSetFilenameFormatting(WSRequestHandler* req);
		static std::string HandleGetFilenameFormatting(WSRequestHandler* req);

		static std::string HandleSetCurrentScene(WSRequestHandler* req);
		static std::string HandleGetCurrentScene(WSRequestHandler* req);
		static std::string HandleGetSceneList(WSRequestHandler* req);

		static std::string HandleSetSceneItemRender(WSRequestHandler* req);
		static std::string HandleSetSceneItemPosition(WSRequestHandler* req);
		static std::string HandleSetSceneItemTransform(WSRequestHandler* req);
		static std::string HandleSetSceneItemCrop(WSRequestHandler* req);
		static std::string HandleGetSceneItemProperties(WSRequestHandler* req);
		static std::string HandleSetSceneItemProperties(WSRequestHandler* req);
		static std::string HandleResetSceneItem(WSRequestHandler* req);
		static std::string HandleDuplicateSceneItem(WSRequestHandler* req);
		static std::string HandleDeleteSceneItem(WSRequestHandler* req);
		static std::string HandleReorderSceneItems(WSRequestHandler* req);

		static std::string HandleGetStreamingStatus(WSRequestHandler* req);
		static std::string HandleStartStopStreaming(WSRequestHandler* req);
		static std::string HandleStartStopRecording(WSRequestHandler* req);
		static std::string HandleStartStreaming(WSRequestHandler* req);
		static std::string HandleStopStreaming(WSRequestHandler* req);
		static std::string HandleStartRecording(WSRequestHandler* req);
		static std::string HandleStopRecording(WSRequestHandler* req);

		static std::string HandleStartStopReplayBuffer(WSRequestHandler* req);
		static std::string HandleStartReplayBuffer(WSRequestHandler* req);
		static std::string HandleStopReplayBuffer(WSRequestHandler* req);
		static std::string HandleSaveReplayBuffer(WSRequestHandler* req);

		static std::string HandleSetRecordingFolder(WSRequestHandler* req);
		static std::string HandleGetRecordingFolder(WSRequestHandler* req);

		static std::string HandleGetTransitionList(WSRequestHandler* req);
		static std::string HandleGetCurrentTransition(WSRequestHandler* req);
		static std::string HandleSetCurrentTransition(WSRequestHandler* req);

		static std::string HandleSetVolume(WSRequestHandler* req);
		static std::string HandleGetVolume(WSRequestHandler* req);
		static std::string HandleToggleMute(WSRequestHandler* req);
		static std::string HandleSetMute(WSRequestHandler* req);
		static std::string HandleGetMute(WSRequestHandler* req);
		static std::string HandleSetSyncOffset(WSRequestHandler* req);
		static std::string HandleGetSyncOffset(WSRequestHandler* req);
		static std::string HandleGetSpecialSources(WSRequestHandler* req);
		static std::string HandleGetSourcesList(WSRequestHandler* req);
		static std::string HandleGetSourceTypesList(WSRequestHandler* req);
		static std::string HandleGetSourceSettings(WSRequestHandler* req);
		static std::string HandleSetSourceSettings(WSRequestHandler* req);

		static std::string HandleGetSourceFilters(WSRequestHandler* req);
		static std::string HandleAddFilterToSource(WSRequestHandler* req);
		static std::string HandleRemoveFilterFromSource(WSRequestHandler* req);
		static std::string HandleReorderSourceFilter(WSRequestHandler* req);
		static std::string HandleMoveSourceFilter(WSRequestHandler* req);
		static std::string HandleSetSourceFilterSettings(WSRequestHandler* req);

		static std::string HandleSetCurrentSceneCollection(WSRequestHandler* req);
		static std::string HandleGetCurrentSceneCollection(WSRequestHandler* req);
		static std::string HandleListSceneCollections(WSRequestHandler* req);

		static std::string HandleSetCurrentProfile(WSRequestHandler* req);
		static std::string HandleGetCurrentProfile(WSRequestHandler* req);
		static std::string HandleListProfiles(WSRequestHandler* req);

		static std::string HandleSetStreamSettings(WSRequestHandler* req);
		static std::string HandleGetStreamSettings(WSRequestHandler* req);
		static std::string HandleSaveStreamSettings(WSRequestHandler* req);

		static std::string HandleSetTransitionDuration(WSRequestHandler* req);
		static std::string HandleGetTransitionDuration(WSRequestHandler* req);

		static std::string HandleGetStudioModeStatus(WSRequestHandler* req);
		static std::string HandleGetPreviewScene(WSRequestHandler* req);
		static std::string HandleSetPreviewScene(WSRequestHandler* req);
		static std::string HandleTransitionToProgram(WSRequestHandler* req);
		static std::string HandleEnableStudioMode(WSRequestHandler* req);
		static std::string HandleDisableStudioMode(WSRequestHandler* req);
		static std::string HandleToggleStudioMode(WSRequestHandler* req);

		static std::string HandleSetTextGDIPlusProperties(WSRequestHandler* req);
		static std::string HandleGetTextGDIPlusProperties(WSRequestHandler* req);

		static std::string HandleSetTextFreetype2Properties(WSRequestHandler* req);
		static std::string HandleGetTextFreetype2Properties(WSRequestHandler* req);

		static std::string HandleSetBrowserSourceProperties(WSRequestHandler* req);
		static std::string HandleGetBrowserSourceProperties(WSRequestHandler* req);
};

#endif // WSPROTOCOL_H
