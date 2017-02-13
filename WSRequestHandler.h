/*
obs-websocket
Copyright (C) 2016	Stéphane Lepin <stephane.lepin@gmail.com>

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

#include <QtWebSockets/QWebSocket>
#include <obs-frontend-api.h>

class WSRequestHandler : public QObject
{
	Q_OBJECT

	public:
		explicit WSRequestHandler(QWebSocket *client);
		~WSRequestHandler();
		void sendTextMessage(QString textMessage);
		bool isAuthenticated();

	private Q_SLOTS:
		void processTextMessage(QString textMessage);
		void socketDisconnected();

	Q_SIGNALS:
		void disconnected();

	private:
		QWebSocket *_client;
		bool _authenticated;
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
		static void HandleSetSourceRender(WSRequestHandler *owner);
		
		static void HandleGetStreamingStatus(WSRequestHandler *owner);
		static void HandleStartStopStreaming(WSRequestHandler *owner);
		static void HandleStartStopRecording(WSRequestHandler *owner);

		static void HandleGetTransitionList(WSRequestHandler *owner);
		static void HandleGetCurrentTransition(WSRequestHandler *owner);
		static void HandleSetCurrentTransition(WSRequestHandler *owner);

		static void HandleSetVolume(WSRequestHandler *owner);
		static void HandleGetVolume(WSRequestHandler *owner);
		static void ToggleMute(WSRequestHandler *owner);
		static void SetMute(WSRequestHandler *owner);
};

#endif // WSPROTOCOL_H
