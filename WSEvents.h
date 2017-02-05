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

#ifndef WSEVENTS_H
#define WSEVENTS_H

#include <QtWebSockets/QWebSocket>
#include <QTimer>
#include <obs-frontend-api.h>
#include <util/platform.h>
#include "WSServer.h"

class WSEvents : public QObject 
{
	Q_OBJECT

	public:
		explicit WSEvents(WSServer *server);
		~WSEvents();
		static void FrontendEventHandler(enum obs_frontend_event event, void *private_data);

	private Q_SLOTS:
		void StreamStatus();

	private:
		WSServer *_srv;
		uint64_t _stream_starttime;
		uint64_t _rec_starttime;
		uint64_t _lastBytesSent;
		uint64_t _lastBytesSentTime;
		void broadcastUpdate(const char *updateType, obs_data_t *additionalFields);

		void OnSceneChange();
		void OnSceneListChange();

		void OnTransitionChange();
		void OnTransitionListChange();

		void OnStreamStarting();
		void OnStreamStarted();
		void OnStreamStopping();
		void OnStreamStopped();

		void OnRecordingStarting();
		void OnRecordingStarted();
		void OnRecordingStopping();
		void OnRecordingStopped();

		void OnExit();
};

#endif // WSEVENTS_H