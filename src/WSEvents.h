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
#include <util/platform.h>

#include <QtWidgets/QListWidgetItem>
#include <QtCore/QSharedPointer>
#include <QtCore/QTimer>

#include "WSServer.h"

class WSEvents : public QObject
{
Q_OBJECT

public:
	explicit WSEvents(WSServerPtr srv);
	~WSEvents();

	void connectSourceSignals(obs_source_t* source);
	void disconnectSourceSignals(obs_source_t* source);

	void connectFilterSignals(obs_source_t* filter);
	void disconnectFilterSignals(obs_source_t* filter);

	void hookTransitionPlaybackEvents();
	void unhookTransitionPlaybackEvents();

	uint64_t getStreamingTime();
	uint64_t getRecordingTime();

	QString getStreamingTimecode();
	QString getRecordingTimecode();
	QString getVirtualCamTimecode();
	
	obs_data_t* GetStats();

	void OnBroadcastCustomMessage(QString realm, obs_data_t* data);

	bool HeartbeatIsActive;

private slots:
	void StreamStatus();
	void Heartbeat();
	void TransitionDurationChanged(int ms);

private:
	WSServerPtr _srv;
	QTimer streamStatusTimer;
	QTimer heartbeatTimer;
	os_cpu_usage_info_t* cpuUsageInfo;

	bool pulse;

	uint64_t _streamStarttime;

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
	void OnRecordingPaused();
	void OnRecordingResumed();
	
	void OnVirtualCamStarted();
	void OnVirtualCamStopped();

	void OnReplayStarting();
	void OnReplayStarted();
	void OnReplayStopping();
	void OnReplayStopped();

	void OnStudioModeSwitched(bool enabled);
	void OnPreviewSceneChanged();

	void OnExit();

	static void FrontendEventHandler(
		enum obs_frontend_event event, void* privateData);

	static void OnTransitionBegin(void* param, calldata_t* data);
	static void OnTransitionEnd(void* param, calldata_t* data);
	static void OnTransitionVideoEnd(void* param, calldata_t* data);

	static void OnSourceCreate(void* param, calldata_t* data);
	static void OnSourceDestroy(void* param, calldata_t* data);

	static void OnSourceVolumeChange(void* param, calldata_t* data);
	static void OnSourceMuteStateChange(void* param, calldata_t* data);
	static void OnSourceAudioSyncOffsetChanged(void* param, calldata_t* data);
	static void OnSourceAudioMixersChanged(void* param, calldata_t* data);
	static void OnSourceAudioActivated(void* param, calldata_t* data);
	static void OnSourceAudioDeactivated(void* param, calldata_t* data);

	static void OnSourceRename(void* param, calldata_t* data);

	static void OnSourceFilterAdded(void* param, calldata_t* data);
	static void OnSourceFilterRemoved(void* param, calldata_t* data);
	static void OnSourceFilterVisibilityChanged(void* param, calldata_t* data);
	static void OnSourceFilterOrderChanged(void* param, calldata_t* data);
	
	static void OnMediaPlaying(void* param, calldata_t* data);
	static void OnMediaPaused(void* param, calldata_t* data);
	static void OnMediaRestarted(void* param, calldata_t* data);
	static void OnMediaStopped(void* param, calldata_t* data);
	static void OnMediaNext(void* param, calldata_t* data);
	static void OnMediaPrevious(void* param, calldata_t* data);
	static void OnMediaStarted(void* param, calldata_t* data);
	static void OnMediaEnded(void* param, calldata_t* data);

	static void OnSceneReordered(void* param, calldata_t* data);
	static void OnSceneItemAdd(void* param, calldata_t* data);
	static void OnSceneItemDelete(void* param, calldata_t* data);
	static void OnSceneItemVisibilityChanged(void* param, calldata_t* data);
	static void OnSceneItemLockChanged(void* param, calldata_t* data);
	static void OnSceneItemTransform(void* param, calldata_t* data);
	static void OnSceneItemSelected(void* param, calldata_t* data);
	static void OnSceneItemDeselected(void* param, calldata_t* data);
};
