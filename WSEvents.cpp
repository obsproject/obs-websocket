#include "WSEvents.h"

WSEvents::WSEvents(WSServer *server) {
	_srv = server;
	obs_frontend_add_event_callback(WSEvents::FrontendEventHandler, this);

	QTimer *statusTimer = new QTimer();
	connect(statusTimer, SIGNAL(timeout()), this, SLOT(StreamStatus));
	statusTimer->start(1000);
}

WSEvents::~WSEvents() {
	obs_frontend_remove_event_callback(WSEvents::FrontendEventHandler, this);
}

void WSEvents::FrontendEventHandler(enum obs_frontend_event event, void *private_data)
{
	WSEvents *owner = static_cast<WSEvents *>(private_data);

	if (event == OBS_FRONTEND_EVENT_SCENE_CHANGED) {
		owner->OnSceneChange();
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
}

void WSEvents::OnSceneChange() {
	// Implements an existing update type from bilhamil's OBS Remote
	obs_source_t *source = obs_frontend_get_current_scene();
	const char *name = obs_source_get_name(source);

	obs_data_t *data = obs_data_create();
	obs_data_set_string(data, "scene-name", name);

	broadcastUpdate("SwitchScenes", data);

	obs_data_release(data);
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

// TODO : Add a timer to trigger StreamStatus
void WSEvents::StreamStatus() {
	blog(LOG_INFO, "top StreamStatus");

	bool streamingActive = obs_frontend_streaming_active();
	bool recordingActive = obs_frontend_recording_active();

	obs_output_t *streamOutput = obs_frontend_get_streaming_output();

	if (!streamOutput) {
		blog(LOG_INFO, "not this time. no stream output running.");
		return;
	}

	uint64_t bytesPerSec = 0;

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
	bytesPerSec = bitsPerSec / 8;

	uint64_t totalStreamTime = (os_gettime_ns() - _streamStartTime); // TODO : convert to seconds
	
	uint64_t droppedFrames = obs_output_get_frames_dropped(streamOutput);
	uint64_t totalFrames = obs_output_get_total_frames(streamOutput);

	obs_data_t *data = obs_data_create();
	obs_data_set_bool(data, "streaming", streamingActive);
	obs_data_set_bool(data, "recording", recordingActive); // New in OBS Studio
	obs_data_set_bool(data, "preview-only", false); // Retrocompat with OBSRemote
	obs_data_set_int(data, "bytes-per-sec", bytesPerSec);
	obs_data_set_double(data, "strain", 0.0); // TODO
	obs_data_set_int(data, "total-stream-time", totalStreamTime);
	obs_data_set_int(data, "num-total-frames", totalFrames);
	obs_data_set_int(data, "num-dropped-frames", droppedFrames);
	obs_data_set_double(data, "fps", obs_get_active_fps());

	broadcastUpdate("StreamStatus", data);

	obs_data_release(data);
}