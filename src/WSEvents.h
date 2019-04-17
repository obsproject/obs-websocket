/*
obs-websocket
Copyright (C) 2016-2019	Stéphane Lepin <stephane.lepin@gmail.com>
Copyright (C) 2017	Brendan Hagan <https://github.com/haganbmj>

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

#include <obs.hpp>
#include <obs-frontend-api.h>

#include <QListWidgetItem>
#include <QSharedPointer>
#include <QtCore/QTimer>

#include "WSServer.h"

class WSEvents;
typedef QSharedPointer<WSEvents> WSEventsPtr;

class WSEvents : public QObject
{
Q_OBJECT

public:
	static WSEventsPtr Current();
	static void ResetCurrent(WSServerPtr srv);

	explicit WSEvents(WSServerPtr srv);
	~WSEvents();
	static void FrontendEventHandler(
		enum obs_frontend_event event, void* privateData);

	void hookTransitionBeginEvent();

	uint64_t GetStreamingTime();
	const char* GetStreamingTimecode();
	uint64_t GetRecordingTime();
	const char* GetRecordingTimecode();

	bool HeartbeatIsActive;

private slots:
	void StreamStatus();
	void Heartbeat();
	void TransitionDurationChanged(int ms);

private:
	static WSEventsPtr _instance;

	WSServerPtr _srv;
	QTimer streamStatusTimer;
	QTimer heartbeatTimer;
	OBSSource currentScene;

	bool pulse;

	uint64_t _streamStarttime;
	uint64_t _recStarttime;

	uint64_t _lastBytesSent;
	uint64_t _lastBytesSentTime;

	void broadcastUpdate(const char* updateType,
		obs_data_t* additionalFields);

	void OnSceneChange();
	void OnSceneListChange();
	void OnSceneCollectionChange();
	void OnSceneCollectionListChange();

	void OnTransitionChange();
	void OnTransitionListChange();

	void OnProfileChange();
	void OnProfileListChange();

	void OnStreamStarting();
	void OnStreamStarted();
	void OnStreamStopping();
	void OnStreamStopped();

	void OnRecordingStarting();
	void OnRecordingStarted();
	void OnRecordingStopping();
	void OnRecordingStopped();

	void OnReplayStarting();
	void OnReplayStarted();
	void OnReplayStopping();
	void OnReplayStopped();

	void OnStudioModeSwitched(bool enabled);
	void OnPreviewSceneChanged();

	void OnExit();

	static void OnTransitionBegin(void* param, calldata_t* data);

	static void OnSourceCreate(void* param, calldata_t* data);

	static void OnSceneReordered(void* param, calldata_t* data);
	static void OnSceneItemAdd(void* param, calldata_t* data);
	static void OnSceneItemDelete(void* param, calldata_t* data);
	static void OnSceneItemVisibilityChanged(void* param, calldata_t* data);
	static void OnSceneItemTransform(void* param, calldata_t* data);
};
