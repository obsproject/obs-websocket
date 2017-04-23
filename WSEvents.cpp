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

#include <util/platform.h>
#include <QTimer>
#include <QPushButton>
#include "Utils.h"
#include "WSEvents.h"
#include "obs-websocket.h"

bool transition_is_cut(obs_source_t *transition)
{
	if (!transition)
		return false;

	if (obs_source_get_type(transition) == OBS_SOURCE_TYPE_TRANSITION 
		&& strcmp(obs_source_get_id(transition), "cut_transition") == 0)
	{
		return true;
	}

	return false;
}

const char* ns_to_timestamp(uint64_t ns)
{
	uint64_t ms = ns / (1000 * 1000);
	uint64_t secs = ms / 1000;
	uint64_t minutes = secs / 60;

	uint64_t hours_part = minutes / 60;
	uint64_t minutes_part = minutes % 60;
	uint64_t secs_part = secs % 60;
	uint64_t ms_part = ms % 1000;

	char* ts = (char*)bmalloc(64);
	sprintf(ts, "%02d:%02d:%02d.%03d", hours_part, minutes_part, secs_part, ms_part);

	return ts;
}

WSEvents* WSEvents::Instance = nullptr;

WSEvents::WSEvents(WSServer *srv)
{
	_srv = srv;
	obs_frontend_add_event_callback(WSEvents::FrontendEventHandler, this);

	QSpinBox* duration_control = Utils::GetTransitionDurationControl();
	connect(duration_control, SIGNAL(valueChanged(int)), this, SLOT(TransitionDurationChanged(int)));

	QTimer *statusTimer = new QTimer();
	connect(statusTimer, SIGNAL(timeout()), this, SLOT(StreamStatus()));
	statusTimer->start(2000); // equal to frontend's constant BITRATE_UPDATE_SECONDS

	QListWidget* sceneList = Utils::GetSceneListControl();
	connect(sceneList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(SelectedSceneChanged(QListWidgetItem*, QListWidgetItem*)));

	QPushButton* modeSwitch = Utils::GetPreviewModeButtonControl();
	connect(modeSwitch, SIGNAL(clicked(bool)), this, SLOT(ModeSwitchClicked(bool)));

	transition_handler = nullptr;
	scene_handler = nullptr;

	QTimer::singleShot(1000, this, SLOT(deferredInitOperations()));

	_streaming_active = false;
	_recording_active = false;

	_stream_starttime = 0;
	_rec_starttime = 0;
}

WSEvents::~WSEvents()
{
	obs_frontend_remove_event_callback(WSEvents::FrontendEventHandler, this);
}

void WSEvents::deferredInitOperations()
{
	obs_source_t* transition = obs_frontend_get_current_transition();
	connectTransitionSignals(transition);
	obs_source_release(transition);

	obs_source_t* scene = obs_frontend_get_current_scene();
	connectSceneSignals(scene);
	obs_source_release(scene);
}

void WSEvents::FrontendEventHandler(enum obs_frontend_event event, void *private_data)
{
	WSEvents *owner = static_cast<WSEvents *>(private_data);

	if (!owner->_srv)
		return;

	// TODO : implement SourceOrderChanged and RepopulateSources

	if (event == OBS_FRONTEND_EVENT_SCENE_CHANGED)
	{
		owner->OnSceneChange();
	}
	else if (event == OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED)
	{
		owner->OnSceneListChange();
	}
	else if (event == OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED)
	{
		owner->OnSceneCollectionChange();
	}
	else if (event == OBS_FRONTEND_EVENT_SCENE_COLLECTION_LIST_CHANGED)
	{
		owner->OnSceneCollectionListChange();
	}
	else if (event == OBS_FRONTEND_EVENT_TRANSITION_CHANGED)
	{
		owner->OnTransitionChange();
	}
	else if (event == OBS_FRONTEND_EVENT_TRANSITION_LIST_CHANGED)
	{
		owner->OnTransitionListChange();
	}
	else if (event == OBS_FRONTEND_EVENT_PROFILE_CHANGED)
	{
		owner->OnProfileChange();
	}
	else if (event == OBS_FRONTEND_EVENT_PROFILE_LIST_CHANGED)
	{
		owner->OnProfileListChange();
	}
	else if (event == OBS_FRONTEND_EVENT_STREAMING_STARTING)
	{
		owner->OnStreamStarting();
	}
	else if (event == OBS_FRONTEND_EVENT_STREAMING_STARTED)
	{
		owner->_streaming_active = true;
		owner->OnStreamStarted();
	}
	else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPING)
	{
		owner->OnStreamStopping();
	}
	else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPED)
	{
		owner->_streaming_active = false;
		owner->OnStreamStopped();
	}
	else if (event == OBS_FRONTEND_EVENT_RECORDING_STARTING)
	{
		owner->OnRecordingStarting();
	}
	else if (event == OBS_FRONTEND_EVENT_RECORDING_STARTED)
	{
		owner->_recording_active = true;
		owner->OnRecordingStarted();
	}
	else if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPING)
	{
		owner->OnRecordingStopping();
	}
	else if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPED)
	{
		owner->_recording_active = false;
		owner->OnRecordingStopped();
	}
	else if (event == OBS_FRONTEND_EVENT_EXIT)
	{
		owner->OnExit();
	}
}

void WSEvents::broadcastUpdate(const char *updateType, obs_data_t *additionalFields = NULL)
{
	obs_data_t *update = obs_data_create();
	obs_data_set_string(update, "update-type", updateType);
	
	const char* ts = nullptr;
	if (_streaming_active)
	{
		ts = ns_to_timestamp(os_gettime_ns() - _stream_starttime);
		obs_data_set_string(update, "stream-timecode", ts);
		bfree((void*)ts);
	}

	if (_recording_active)
	{
		ts = ns_to_timestamp(os_gettime_ns() - _rec_starttime);
		obs_data_set_string(update, "rec-timecode", ts);
		bfree((void*)ts);
	}
	
	if (additionalFields != NULL) {
		obs_data_apply(update, additionalFields);
	}

	_srv->broadcast(obs_data_get_json(update));

	obs_data_release(update);
}

void WSEvents::connectTransitionSignals(obs_source_t* transition)
{
	if (transition_handler)
	{
		signal_handler_disconnect(transition_handler, "transition_start", OnTransitionBegin, this);
	}

	if (!transition_is_cut(transition))
	{
		transition_handler = obs_source_get_signal_handler(transition);
		signal_handler_connect(transition_handler, "transition_start", OnTransitionBegin, this);	}
	else
	{
		transition_handler = nullptr;
	}
}

void WSEvents::connectSceneSignals(obs_source_t* scene)
{
	if (scene_handler)
	{
		signal_handler_disconnect(scene_handler, "reorder", OnSceneReordered, this);
		signal_handler_disconnect(scene_handler, "item_add", OnSceneItemAdd, this);
		signal_handler_disconnect(scene_handler, "item_remove", OnSceneItemDelete, this);
		signal_handler_disconnect(scene_handler, "item_visible", OnSceneItemVisibilityChanged, this);
	}

	// TODO : connect to all scenes, not just the current one.
	scene_handler = obs_source_get_signal_handler(scene);
	signal_handler_connect(scene_handler, "reorder", OnSceneReordered, this);
	signal_handler_connect(scene_handler, "item_add", OnSceneItemAdd, this);
	signal_handler_connect(scene_handler, "item_remove", OnSceneItemDelete, this);
	signal_handler_connect(scene_handler, "item_visible", OnSceneItemVisibilityChanged, this);
}

uint64_t WSEvents::GetStreamingTime()
{
	if (_streaming_active)
		return (os_gettime_ns() - _stream_starttime);
	else
		return 0;
}

const char* WSEvents::GetStreamingTimecode()
{
	return ns_to_timestamp(GetStreamingTime());
}

uint64_t WSEvents::GetRecordingTime()
{
	if (_recording_active)
		return (os_gettime_ns() - _rec_starttime);
	else
		return 0;
}

const char* WSEvents::GetRecordingTimecode()
{
	return ns_to_timestamp(GetRecordingTime());
}

void WSEvents::OnSceneChange()
{
	// Implements an existing update type from bilhamil's OBS Remote
	obs_data_t *data = obs_data_create();

	obs_source_t* current_scene = obs_frontend_get_current_scene();
	obs_data_array_t* scene_items = Utils::GetSceneItems(current_scene);
	connectSceneSignals(current_scene);

	obs_data_set_string(data, "scene-name", obs_source_get_name(current_scene));
	obs_data_set_array(data, "sources", scene_items);

	broadcastUpdate("SwitchScenes", data);

	obs_data_array_release(scene_items);
	obs_source_release(current_scene);
	obs_data_release(data);

	// Dirty fix : OBS blocks signals when swapping scenes in Studio Mode
	// after transition end, so SelectedSceneChanged is never called...
	if (Utils::IsPreviewModeActive())
	{
		QListWidget* list = Utils::GetSceneListControl();
		SelectedSceneChanged(list->currentItem(), nullptr);
	}
}

void WSEvents::OnSceneListChange()
{
	broadcastUpdate("ScenesChanged");
}

void WSEvents::OnSceneCollectionChange()
{
	broadcastUpdate("SceneCollectionChanged");

	scene_handler = nullptr;
	transition_handler = nullptr;

	OnTransitionListChange();
	OnTransitionChange();

	OnSceneListChange();
	OnSceneChange();
}

void WSEvents::OnSceneCollectionListChange()
{
	broadcastUpdate("SceneCollectionListChanged");
}

void WSEvents::OnTransitionChange()
{
	obs_source_t* current_transition = obs_frontend_get_current_transition();
	connectTransitionSignals(current_transition);

	obs_data_t *data = obs_data_create();
	obs_data_set_string(data, "transition-name", obs_source_get_name(current_transition));
		
	broadcastUpdate("SwitchTransition", data);
	
	obs_data_release(data);
	obs_source_release(current_transition);
}

void WSEvents::OnTransitionListChange()
{
	broadcastUpdate("TransitionListChanged");
}

void WSEvents::OnProfileChange()
{
	broadcastUpdate("ProfileChanged");
}

void WSEvents::OnProfileListChange()
{
	broadcastUpdate("ProfileListChanged");
}

void WSEvents::OnStreamStarting()
{
	// Implements an existing update type from bilhamil's OBS Remote
	obs_data_t *data = obs_data_create();
	obs_data_set_bool(data, "preview-only", false);

	broadcastUpdate("StreamStarting", data);

	obs_data_release(data);
}

void WSEvents::OnStreamStarted()
{
	// New update type specific to OBS Studio
	_stream_starttime = os_gettime_ns();
	_lastBytesSent = 0;
	broadcastUpdate("StreamStarted");
}

void WSEvents::OnStreamStopping()
{
	// Implements an existing update type from bilhamil's OBS Remote
	obs_data_t *data = obs_data_create();
	obs_data_set_bool(data, "preview-only", false);

	broadcastUpdate("StreamStopping", data);

	obs_data_release(data);
}

void WSEvents::OnStreamStopped()
{
	// New update type specific to OBS Studio
	_stream_starttime = 0;
	broadcastUpdate("StreamStopped");
}

void WSEvents::OnRecordingStarting()
{
	// New update type specific to OBS Studio
	broadcastUpdate("RecordingStarting");
}

void WSEvents::OnRecordingStarted()
{
	// New update type specific to OBS Studio
	_rec_starttime = os_gettime_ns();
	broadcastUpdate("RecordingStarted");
}

void WSEvents::OnRecordingStopping()
{
	// New update type specific to OBS Studio
	broadcastUpdate("RecordingStopping");
}

void WSEvents::OnRecordingStopped()
{
	// New update type specific to OBS Studio
	_rec_starttime = 0;
	broadcastUpdate("RecordingStopped");
}

void WSEvents::OnExit()
{
	// New update type specific to OBS Studio
	broadcastUpdate("Exiting");
}

void WSEvents::StreamStatus()
{
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

	float strain = obs_output_get_congestion(stream_output);

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

void WSEvents::TransitionDurationChanged(int ms)
{
	obs_data_t* fields = obs_data_create();
	obs_data_set_int(fields, "new-duration", ms);

	broadcastUpdate("TransitionDurationChanged", fields);

	obs_data_release(fields);
}

void WSEvents::OnTransitionBegin(void* param, calldata_t* data)
{
	UNUSED_PARAMETER(data);

	WSEvents* instance = static_cast<WSEvents*>(param);
	instance->broadcastUpdate("TransitionBegin");

	blog(LOG_INFO, "transition begin");
}

void WSEvents::OnSceneReordered(void *param, calldata_t *data)
{
	WSEvents* instance = static_cast<WSEvents*>(param);

	obs_scene_t* scene = nullptr;
	calldata_get_ptr(data, "scene", &scene);

	obs_data_t *fields = obs_data_create();
	obs_data_set_string(fields, "scene-name", obs_source_get_name(obs_scene_get_source(scene)));
	
	instance->broadcastUpdate("SourceOrderChanged", fields);

	obs_data_release(fields);
}

void WSEvents::OnSceneItemAdd(void *param, calldata_t *data)
{
	WSEvents* instance = static_cast<WSEvents*>(param);

	obs_scene_t* scene = nullptr;
	calldata_get_ptr(data, "scene", &scene);

	obs_sceneitem_t* scene_item = nullptr;
	calldata_get_ptr(data, "item", &scene_item);

	const char* scene_name = obs_source_get_name(obs_scene_get_source(scene));
	const char* sceneitem_name = obs_source_get_name(obs_sceneitem_get_source(scene_item));

	obs_data_t* fields = obs_data_create();
	obs_data_set_string(fields, "scene-name", scene_name);
	obs_data_set_string(fields, "item-name", sceneitem_name);

	instance->broadcastUpdate("SceneItemAdded", fields);

	obs_data_release(fields);
}

void WSEvents::OnSceneItemDelete(void *param, calldata_t *data)
{
	WSEvents* instance = static_cast<WSEvents*>(param);

	obs_scene_t* scene = nullptr;
	calldata_get_ptr(data, "scene", &scene);

	obs_sceneitem_t* scene_item = nullptr;
	calldata_get_ptr(data, "item", &scene_item);

	const char* scene_name = obs_source_get_name(obs_scene_get_source(scene));
	const char* sceneitem_name = obs_source_get_name(obs_sceneitem_get_source(scene_item));

	obs_data_t* fields = obs_data_create();
	obs_data_set_string(fields, "scene-name", scene_name);
	obs_data_set_string(fields, "item-name", sceneitem_name);

	instance->broadcastUpdate("SceneItemRemoved", fields);

	obs_data_release(fields);
}

void WSEvents::OnSceneItemVisibilityChanged(void *param, calldata_t *data)
{
	WSEvents* instance = static_cast<WSEvents*>(param);

	obs_scene_t* scene = nullptr;
	calldata_get_ptr(data, "scene", &scene);

	obs_sceneitem_t* scene_item = nullptr;
	calldata_get_ptr(data, "item", &scene_item);

	bool visible = false;
	calldata_get_bool(data, "visible", &visible);

	const char* scene_name = obs_source_get_name(obs_scene_get_source(scene));
	const char* sceneitem_name = obs_source_get_name(obs_sceneitem_get_source(scene_item));

	obs_data_t* fields = obs_data_create();
	obs_data_set_string(fields, "scene-name", scene_name);
	obs_data_set_string(fields, "item-name", sceneitem_name);
	obs_data_set_bool(fields, "item-visible", visible);

	instance->broadcastUpdate("SceneItemVisibilityChanged", fields);

	obs_data_release(fields);
}

void WSEvents::SelectedSceneChanged(QListWidgetItem *current, QListWidgetItem *prev)
{
	if (Utils::IsPreviewModeActive())
	{
		obs_scene_t* scene = Utils::SceneListItemToScene(current);
		if (!scene) return;

		obs_source_t* scene_source = obs_scene_get_source(scene);
		obs_data_array_t* scene_items = Utils::GetSceneItems(scene_source);

		obs_data_t* data = obs_data_create();
		obs_data_set_string(data, "scene-name", obs_source_get_name(scene_source));
		obs_data_set_array(data, "sources", scene_items);

		broadcastUpdate("PreviewSceneChanged", data);

		obs_data_array_release(scene_items);
		obs_data_release(data);
	}
}

void WSEvents::ModeSwitchClicked(bool checked)
{
	obs_data_t* data = obs_data_create();
	obs_data_set_bool(data, "new-state", checked);

	broadcastUpdate("StudioModeSwitched", data);

	obs_data_release(data);
}