/*
obs-websocket
Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>
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

#include "WSEvents.h"

WSEvents::WSEvents(WSServer *server) {
	_srv = server;
	obs_frontend_add_event_callback(WSEvents::FrontendEventHandler, this);

	QTimer *statusTimer = new QTimer();
	connect(statusTimer, SIGNAL(timeout()), this, SLOT(StreamStatus()));
	statusTimer->start(2000); // equal to frontend's constant BITRATE_UPDATE_SECONDS

	_stream_starttime = 0;
	_rec_starttime = 0;
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
	else if (event == OBS_FRONTEND_EVENT_TRANSITION_CHANGED) {
		owner->OnTransitionChange();
	}
	else if (event == OBS_FRONTEND_EVENT_TRANSITION_LIST_CHANGED) {
		owner->OnTransitionListChange();
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
	else if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPING) {
		owner->OnRecordingStopping();
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
	obs_data_t *update = obs_data_create();

	obs_data_set_string(update, "update-type", updateType);
	if (additionalFields != NULL) {
		obs_data_apply(update, additionalFields);
	}

	_srv->broadcast(obs_data_get_json(update));

	obs_data_release(update);
}

void WSEvents::OnSceneChange() {
	// Implements an existing update type from bilhamil's OBS Remote

	// Default behavior : get the new scene from the running transition
	obs_source_t *transition = obs_frontend_get_current_transition();
	obs_source_t *new_scene = obs_transition_get_source(transition, OBS_TRANSITION_SOURCE_B);
	if (!new_scene) {
		obs_source_release(transition);
		return;
	}
	
	const char *scene_name = obs_source_get_name(new_scene);
	if (!scene_name) {
		// Fallback behaviour : get the new scene straight from the API
		obs_source_release(new_scene);
		new_scene = obs_frontend_get_current_scene();

		if (new_scene) {
			scene_name = obs_source_get_name(new_scene);
		}
	}

	obs_data_t *data = obs_data_create();
	obs_data_set_string(data, "scene-name", scene_name);
	
	broadcastUpdate("SwitchScenes", data);

	obs_data_release(data);
	obs_source_release(new_scene);
	obs_source_release(transition);
}

void WSEvents::OnSceneListChange() {
	broadcastUpdate("ScenesChanged");
}

void WSEvents::OnTransitionChange() {
	obs_source_t *transition = obs_frontend_get_current_transition();
	const char *transition_name = obs_source_get_name(transition);

	obs_data_t *data = obs_data_create();
	obs_data_set_string(data, "transition-name", transition_name);

	broadcastUpdate("SwitchTransition", data);

	obs_data_release(data);
	obs_source_release(transition);
}

void WSEvents::OnTransitionListChange() {
	broadcastUpdate("TransitionListChanged");
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
	_stream_starttime = os_gettime_ns();
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
	_stream_starttime = 0;
	broadcastUpdate("StreamStopped");
}

void WSEvents::OnRecordingStarting() {
	// New update type specific to OBS Studio
	broadcastUpdate("RecordingStarting");
}

void WSEvents::OnRecordingStarted() {
	// New update type specific to OBS Studio
	_rec_starttime = os_gettime_ns();
	broadcastUpdate("RecordingStarted");
}

void WSEvents::OnRecordingStopping() {
	// New update type specific to OBS Studio
	broadcastUpdate("RecordingStopping");
}

void WSEvents::OnRecordingStopped() {
	// New update type specific to OBS Studio
	_rec_starttime = 0;
	broadcastUpdate("RecordingStopped");
}

void WSEvents::OnExit() {
	// New update type specific to OBS Studio
	broadcastUpdate("Exiting");
}

void WSEvents::StreamStatus() {
	bool streaming_active = obs_frontend_streaming_active();
	bool recording_active = obs_frontend_recording_active();

	obs_output_t *stream_output = obs_frontend_get_streaming_output();

	if (!stream_output || !streaming_active) {
		if (stream_output) {
			obs_output_release(stream_output);
		}
		return;
	}

	uint64_t bytes_sent = obs_output_get_total_bytes(stream_output);
	uint64_t bytes_sent_time = os_gettime_ns();

	if (bytes_sent < _lastBytesSent) {
		bytes_sent = 0;
	}
	if (bytes_sent == 0) {
		_lastBytesSent = 0;
	}
	
	uint64_t bytes_between = bytes_sent - _lastBytesSent;
	double time_passed = double(bytes_sent_time - _lastBytesSentTime) / 1000000000.0;

	uint64_t bytes_per_sec = bytes_between / time_passed;

	_lastBytesSent = bytes_sent;
	_lastBytesSentTime = bytes_sent_time;

	uint64_t totalStreamTime = (os_gettime_ns() - _stream_starttime) / 1000000000;

	int total_frames = obs_output_get_total_frames(stream_output);
	int dropped_frames = obs_output_get_frames_dropped(stream_output);

	float strain = 0.0;
	if (total_frames > 0) {
		strain = (dropped_frames / total_frames) * 100.0;
	}

	obs_data_t *data = obs_data_create();
	obs_data_set_bool(data, "streaming", streaming_active);
	obs_data_set_bool(data, "recording", recording_active);
	obs_data_set_int(data, "bytes-per-sec", bytes_per_sec);
	obs_data_set_int(data, "kbits-per-sec", (bytes_per_sec * 8) / 1024);
	obs_data_set_int(data, "total-stream-time", totalStreamTime);
	obs_data_set_int(data, "num-total-frames", total_frames);
	obs_data_set_int(data, "num-dropped-frames", dropped_frames);
	obs_data_set_double(data, "fps", obs_get_active_fps());
	obs_data_set_double(data, "strain", strain);
	obs_data_set_bool(data, "preview-only", false); // Retrocompat with OBSRemote

	broadcastUpdate("StreamStatus", data);

	obs_data_release(data);
	obs_output_release(stream_output);
}
