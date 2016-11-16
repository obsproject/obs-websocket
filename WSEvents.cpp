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

#include "WSEvents.h"

WSEvents::WSEvents(WSServer *server) {
	_srv = server;
	obs_frontend_add_event_callback(WSEvents::FrontendEventHandler, this);

	QTimer *statusTimer = new QTimer();
	connect(statusTimer, SIGNAL(timeout()), this, SLOT(StreamStatus()));
	statusTimer->start(1000);
}

WSEvents::~WSEvents() {
	obs_frontend_remove_event_callback(WSEvents::FrontendEventHandler, this);
}

void WSEvents::FrontendEventHandler(enum obs_frontend_event event, void *private_data)
{
	WSEvents *owner = static_cast<WSEvents *>(private_data);

	// TODO : implement SourceChanged, SourceOrderChanged and RepopulateSources

	if (event == OBS_FRONTEND_EVENT_SCENE_CHANGED) {
		owner->OnSceneChange();
	}
	else if (event == OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED) {
		owner->OnSceneListChange();
	}
	else if (event == OBS_FRONTEND_EVENT_STREAMING_STARTING) {
		owner->OnStreamStarting();
	}
	else if (event == OBS_FRONTEND_EVENT_STREAMING_STARTED) {
		owner->OnStreamStarted();
	}
	else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPING) {
		owner->OnStreamStopping();
	}
	else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPED) {
		owner->OnStreamStopped();
	}
	else if (event == OBS_FRONTEND_EVENT_RECORDING_STARTING) {
		owner->OnRecordingStarting();
	}
	else if (event == OBS_FRONTEND_EVENT_RECORDING_STARTED) {
		owner->OnRecordingStarted();
	}
	else if (event == OBS_FRONTEND_EVENT_RECORDING_STARTING) {
		owner->OnRecordingStarting();
	}
	else if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPED) {
		owner->OnRecordingStopped();
	}
	else if (event == OBS_FRONTEND_EVENT_EXIT) {
		obs_frontend_save();
		owner->OnExit();
	}
}

void WSEvents::broadcastUpdate(const char *updateType, obs_data_t *additionalFields = NULL) {
	obs_source_t *source = obs_frontend_get_current_scene();
	const char *name = obs_source_get_name(source);

	obs_data_t *update = obs_data_create();

	obs_data_set_string(update, "update-type", updateType);
	if (additionalFields != NULL) {
		obs_data_apply(update, additionalFields);
	}

	_srv->broadcast(obs_data_get_json(update));

	obs_data_release(update);
	obs_source_release(source);
}

void WSEvents::OnSceneChange() {
	// Implements an existing update type from bilhamil's OBS Remote
	obs_source_t *source = obs_frontend_get_current_scene();
	const char *name = obs_source_get_name(source);

	obs_data_t *data = obs_data_create();
	obs_data_set_string(data, "scene-name", name);

	broadcastUpdate("SwitchScenes", data);

	obs_data_release(data);
	obs_source_release(source);
}

void WSEvents::OnSceneListChange() {
	broadcastUpdate("ScenesChanged");
}

void WSEvents::OnStreamStarting() {
	// Implements an existing update type from bilhamil's OBS Remote
	obs_data_t *data = obs_data_create();
	obs_data_set_bool(data, "preview-only", false);

	broadcastUpdate("StreamStarting", data);

	obs_data_release(data);
}

void WSEvents::OnStreamStarted() {
	// New update type specific to OBS Studio
	_streamStartTime = os_gettime_ns();
	_lastBytesSent = 0;
	broadcastUpdate("StreamStarted");
}

void WSEvents::OnStreamStopping() {
	// Implements an existing update type from bilhamil's OBS Remote
	obs_data_t *data = obs_data_create();
	obs_data_set_bool(data, "preview-only", false);

	broadcastUpdate("StreamStopping", data);

	obs_data_release(data);
}

void WSEvents::OnStreamStopped() {
	// New update type specific to OBS Studio
	_streamStartTime = 0;
	broadcastUpdate("StreamStopped");
}

void WSEvents::OnRecordingStarting() {
	// New update type specific to OBS Studio
	broadcastUpdate("RecordingStarting");
}

void WSEvents::OnRecordingStarted() {
	// New update type specific to OBS Studio
	broadcastUpdate("RecordingStarted");
}

void WSEvents::OnRecordingStopping() {
	// New update type specific to OBS Studio
	broadcastUpdate("RecordingStopping");
}

void WSEvents::OnRecordingStopped() {
	// New update type specific to OBS Studio
	broadcastUpdate("RecordingStopped");
}

void WSEvents::OnExit() {
	// New update type specific to OBS Studio
	broadcastUpdate("Exiting");
}

void WSEvents::StreamStatus() {
	bool streamingActive = obs_frontend_streaming_active();
	bool recordingActive = obs_frontend_recording_active();

	obs_output_t *streamOutput = obs_frontend_get_streaming_output();

	if (!streamOutput || !streamingActive || !recordingActive) {
		return;
	}

	uint64_t bytesSent = obs_output_get_total_bytes(streamOutput);
	uint64_t bytesSentTime = os_gettime_ns();

	if (bytesSent < _lastBytesSent) {
		bytesSent = 0;
	}
	if (bytesSent == 0) {
		_lastBytesSent = 0;
	}
	
	uint64_t bitsBetween = (bytesSent - _lastBytesSent) * 8;
	double timePassed = double(bytesSentTime - _lastBytesSentTime) / 1000000000.0;

	uint64_t bitsPerSec = bitsBetween / timePassed;
	uint64_t bytesPerSec = bitsPerSec / 8;

	_lastBytesSent = bytesSent;
	_lastBytesSentTime = bytesSentTime;

	uint64_t totalStreamTime = (os_gettime_ns() - _streamStartTime) / 1000000000;

	obs_data_t *data = obs_data_create();
	obs_data_set_bool(data, "streaming", streamingActive);
	obs_data_set_bool(data, "recording", recordingActive);
	obs_data_set_bool(data, "preview-only", false); // Retrocompat with OBSRemote
	obs_data_set_int(data, "bytes-per-sec", bytesPerSec); // BUG : Computation seems buggy
	obs_data_set_double(data, "strain", 0.0); // dafuq is strain
	obs_data_set_int(data, "total-stream-time", totalStreamTime);
	obs_data_set_int(data, "num-total-frames", obs_output_get_total_frames(streamOutput));
	obs_data_set_int(data, "num-dropped-frames", obs_output_get_frames_dropped(streamOutput));
	obs_data_set_double(data, "fps", obs_get_active_fps());

	broadcastUpdate("StreamStatus", data);

	obs_data_release(data);
	obs_output_release(streamOutput);
}