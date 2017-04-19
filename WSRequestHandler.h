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

#include <obs-frontend-api.h>
#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>

class WSRequestHandler : public QObject
{
	Q_OBJECT

	public:
		explicit WSRequestHandler(QWebSocket *client);
		~WSRequestHandler();
		void processIncomingMessage(QString textMessage);

	private:
		QWebSocket *_client;
		const char *_messageId;
		const char *_requestType;
		obs_data_t *_requestData;

		QMap<QString, void(*)(WSRequestHandler*)> messageMap;
		QSet<QString> authNotRequired;

		void SendOKResponse(obs_data_t *additionalFields = NULL);
		void SendErrorResponse(const char *errorMessage);
		static void ErrNotImplemented(WSRequestHandler *owner);
		
		static void HandleGetVersion(WSRequestHandler *owner);
		static void HandleGetAuthRequired(WSRequestHandler *owner);
		static void HandleAuthenticate(WSRequestHandler *owner);

		static void HandleSetCurrentScene(WSRequestHandler *owner);
		static void HandleGetCurrentScene(WSRequestHandler *owner);
		static void HandleGetSceneList(WSRequestHandler *owner);
		
		static void HandleSetSceneItemRender(WSRequestHandler *owner);
		static void HandleSetSceneItemPosition(WSRequestHandler *owner);
		static void HandleSetSceneItemTransform(WSRequestHandler *owner);
		static void HandleSetSceneItemCrop(WSRequestHandler *owner);

		static void HandleGetStreamingStatus(WSRequestHandler *owner);
		static void HandleStartStopStreaming(WSRequestHandler *owner);
		static void HandleStartStopRecording(WSRequestHandler *owner);
		static void HandleStartStreaming(WSRequestHandler *owner);
		static void HandleStopStreaming(WSRequestHandler *owner);
		static void HandleStartRecording(WSRequestHandler *owner);
		static void HandleStopRecording(WSRequestHandler *owner);

		static void HandleGetTransitionList(WSRequestHandler *owner);
		static void HandleGetCurrentTransition(WSRequestHandler *owner);
		static void HandleSetCurrentTransition(WSRequestHandler *owner);

		static void HandleSetVolume(WSRequestHandler *owner);
		static void HandleGetVolume(WSRequestHandler *owner);
		static void HandleToggleMute(WSRequestHandler *owner);
		static void HandleSetMute(WSRequestHandler *owner);
		// TODO : GetMute

		static void HandleSetCurrentSceneCollection(WSRequestHandler *owner);
		static void HandleGetCurrentSceneCollection(WSRequestHandler *owner);
		static void HandleListSceneCollections(WSRequestHandler *owner);

		static void HandleSetCurrentProfile(WSRequestHandler *owner);
		static void HandleGetCurrentProfile(WSRequestHandler *owner);
		static void HandleListProfiles(WSRequestHandler *owner);

		static void HandleSetTransitionDuration(WSRequestHandler *owner);
		static void HandleGetTransitionDuration(WSRequestHandler *owner);

		static void HandleGetStudioModeStatus(WSRequestHandler *owner);
		static void HandleEnableStudioMode(WSRequestHandler *owner);
		static void HandleDisableStudioMode(WSRequestHandler *owner);
		static void HandleToggleStudioMode(WSRequestHandler *owner);
};

#endif // WSPROTOCOL_H
