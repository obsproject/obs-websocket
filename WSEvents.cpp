/**
 * obs-websocket
 * Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>
 * Copyright (C) 2017	Brendan Hagan <https://github.com/haganbmj>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>
 */

#include <util/platform.h>

#include <QTimer>
#include <QPushButton>

#include "Config.h"
#include "Utils.h"
#include "WSEvents.h"

#include "obs-websocket.h"

bool transition_is_cut(obs_source_t* transition) {
    if (!transition)
        return false;

    if (obs_source_get_type(transition) == OBS_SOURCE_TYPE_TRANSITION
        && strcmp(obs_source_get_id(transition), "cut_transition") == 0) {
        return true;
    }
    return false;
}

const char* ns_to_timestamp(uint64_t ns) {
    uint64_t ms = ns / (1000 * 1000);
    uint64_t secs = ms / 1000;
    uint64_t minutes = secs / 60;

    uint64_t hours_part = minutes / 60;
    uint64_t minutes_part = minutes % 60;
    uint64_t secs_part = secs % 60;
    uint64_t ms_part = ms % 1000;

    char* ts = (char*)bmalloc(64);
    sprintf(ts, "%02d:%02d:%02d.%03d",
        hours_part, minutes_part, secs_part, ms_part);

    return ts;
}

WSEvents* WSEvents::Instance = nullptr;

WSEvents::WSEvents(WSServer* srv) {
    _srv = srv;
    obs_frontend_add_event_callback(WSEvents::FrontendEventHandler, this);

    QSpinBox* duration_control = Utils::GetTransitionDurationControl();
    connect(duration_control, SIGNAL(valueChanged(int)),
        this, SLOT(TransitionDurationChanged(int)));

    QTimer* statusTimer = new QTimer();
    connect(statusTimer, SIGNAL(timeout()),
        this, SLOT(StreamStatus()));
    pulse = false;
    connect(statusTimer, SIGNAL(timeout()),
        this, SLOT(Heartbeat()));
    statusTimer->start(2000); // equal to frontend's constant BITRATE_UPDATE_SECONDS

    QListWidget* sceneList = Utils::GetSceneListControl();
    connect(sceneList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
        this, SLOT(SelectedSceneChanged(QListWidgetItem*, QListWidgetItem*)));

    transition_handler = nullptr;
    scene_handler = nullptr;

    QTimer::singleShot(1000, this, SLOT(deferredInitOperations()));

    Heartbeat_active = false;

    _streaming_active = false;
    _recording_active = false;

    _stream_starttime = 0;
    _rec_starttime = 0;
}

WSEvents::~WSEvents() {
    obs_frontend_remove_event_callback(WSEvents::FrontendEventHandler, this);
}

void WSEvents::deferredInitOperations() {
    obs_source_t* transition = obs_frontend_get_current_transition();
    connectTransitionSignals(transition);
    obs_source_release(transition);

    obs_source_t* scene = obs_frontend_get_current_scene();
    connectSceneSignals(scene);
    obs_source_release(scene);
}

void WSEvents::FrontendEventHandler(enum obs_frontend_event event, void* private_data) {
    WSEvents* owner = static_cast<WSEvents*>(private_data);

    if (!owner->_srv)
        return;

    if (event == OBS_FRONTEND_EVENT_SCENE_CHANGED) {
        owner->OnSceneChange();
    }
    else if (event == OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED) {
        owner->OnSceneListChange();
    }
    else if (event == OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED) {
        owner->OnSceneCollectionChange();
    }
    else if (event == OBS_FRONTEND_EVENT_SCENE_COLLECTION_LIST_CHANGED) {
        owner->OnSceneCollectionListChange();
    }
    else if (event == OBS_FRONTEND_EVENT_TRANSITION_CHANGED) {
        owner->OnTransitionChange();
    }
    else if (event == OBS_FRONTEND_EVENT_TRANSITION_LIST_CHANGED) {
        owner->OnTransitionListChange();
    }
    else if (event == OBS_FRONTEND_EVENT_PROFILE_CHANGED) {
        owner->OnProfileChange();
    }
    else if (event == OBS_FRONTEND_EVENT_PROFILE_LIST_CHANGED) {
        owner->OnProfileListChange();
    }
    else if (event == OBS_FRONTEND_EVENT_STREAMING_STARTING) {
        owner->OnStreamStarting();
    }
    else if (event == OBS_FRONTEND_EVENT_STREAMING_STARTED) {
        owner->_streaming_active = true;
        owner->OnStreamStarted();
    }
    else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPING) {
        owner->OnStreamStopping();
    }
    else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPED) {
        owner->_streaming_active = false;
        owner->OnStreamStopped();
    }
    else if (event == OBS_FRONTEND_EVENT_RECORDING_STARTING) {
        owner->OnRecordingStarting();
    }
    else if (event == OBS_FRONTEND_EVENT_RECORDING_STARTED) {
        owner->_recording_active = true;
        owner->OnRecordingStarted();
    }
    else if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPING) {
        owner->OnRecordingStopping();
    }
    else if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPED) {
        owner->_recording_active = false;
        owner->OnRecordingStopped();
    }
    else if (event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTING) {
        owner->OnReplayStarting();
    }
    else if (event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED) {
        owner->OnReplayStarted();
    }
    else if (event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPING) {
        owner->OnReplayStopping();
    }
    else if (event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED) {
        owner->OnReplayStopped();
    }
    else if (event == OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED) {
        owner->OnStudioModeSwitched(true);
    }
    else if (event == OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED) {
        owner->OnStudioModeSwitched(false);
    }
    else if (event == OBS_FRONTEND_EVENT_EXIT) {
        owner->OnExit();
    }
}

void WSEvents::broadcastUpdate(const char* updateType,
    obs_data_t* additionalFields = NULL) {
    obs_data_t* update = obs_data_create();
    obs_data_set_string(update, "update-type", updateType);

    const char* ts = nullptr;
    if (_streaming_active) {
        ts = ns_to_timestamp(os_gettime_ns() - _stream_starttime);
        obs_data_set_string(update, "stream-timecode", ts);
        bfree((void*)ts);
    }

    if (_recording_active) {
        ts = ns_to_timestamp(os_gettime_ns() - _rec_starttime);
        obs_data_set_string(update, "rec-timecode", ts);
        bfree((void*)ts);
    }

    if (additionalFields != NULL)
        obs_data_apply(update, additionalFields);

    const char *json = obs_data_get_json(update);
    _srv->broadcast(json);
    if (Config::Current()->DebugEnabled)
        blog(LOG_DEBUG, "Update << '%s'", json);

    obs_data_release(update);
}

void WSEvents::connectTransitionSignals(obs_source_t* transition) {
    if (transition_handler) {
        signal_handler_disconnect(transition_handler,
            "transition_start", OnTransitionBegin, this);
    }

    if (!transition_is_cut(transition)) {
        transition_handler = obs_source_get_signal_handler(transition);
        signal_handler_connect(transition_handler,
            "transition_start", OnTransitionBegin, this);
    } else {
        transition_handler = nullptr;
    }
}

void WSEvents::connectSceneSignals(obs_source_t* scene) {
    if (scene_handler) {
        signal_handler_disconnect(scene_handler,
            "reorder", OnSceneReordered, this);
        signal_handler_disconnect(scene_handler,
            "item_add", OnSceneItemAdd, this);
        signal_handler_disconnect(scene_handler,
            "item_remove", OnSceneItemDelete, this);
        signal_handler_disconnect(scene_handler,
            "item_visible", OnSceneItemVisibilityChanged, this);
    }

    // TODO : connect to all scenes, not just the current one.
    scene_handler = obs_source_get_signal_handler(scene);
    signal_handler_connect(scene_handler,
        "reorder", OnSceneReordered, this);
    signal_handler_connect(scene_handler,
        "item_add", OnSceneItemAdd, this);
    signal_handler_connect(scene_handler,
        "item_remove", OnSceneItemDelete, this);
    signal_handler_connect(scene_handler,
        "item_visible", OnSceneItemVisibilityChanged, this);
}

uint64_t WSEvents::GetStreamingTime() {
    if (_streaming_active)
        return (os_gettime_ns() - _stream_starttime);
    else
        return 0;
}

const char* WSEvents::GetStreamingTimecode() {
    return ns_to_timestamp(GetStreamingTime());
}

uint64_t WSEvents::GetRecordingTime() {
    if (_recording_active)
        return (os_gettime_ns() - _rec_starttime);
    else
        return 0;
}

const char* WSEvents::GetRecordingTimecode() {
    return ns_to_timestamp(GetRecordingTime());
}

 /**
 * Indicates a scene change.
 * 
 * @return {String} `scene-name` The new scene.
 * @return {Array} `sources` List of sources in the new scene.
 *
 * @api events
 * @name SwitchScenes
 * @category scenes
 * @since 0.3
 */
void WSEvents::OnSceneChange() {
    obs_data_t* data = obs_data_create();

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
    if (obs_frontend_preview_program_mode_active()) {
        QListWidget* list = Utils::GetSceneListControl();
        SelectedSceneChanged(list->currentItem(), nullptr);
    }
}

/**
 * The scene list has been modified.
 * Scenes have been added, removed, or renamed.
 * 
 * @api events
 * @name ScenesChanged
 * @category scenes
 * @since 0.3
 */
void WSEvents::OnSceneListChange() {
    broadcastUpdate("ScenesChanged");
}

/**
 * Triggered when switching to another scene collection or when renaming the current scene collection.
 * 
 * @api events
 * @name SceneCollectionChanged
 * @category scenes
 * @since 4.0.0
 */
void WSEvents::OnSceneCollectionChange() {
    broadcastUpdate("SceneCollectionChanged");

    scene_handler = nullptr;
    transition_handler = nullptr;

    OnTransitionListChange();
    OnTransitionChange();

    OnSceneListChange();
    OnSceneChange();
}

/**
 * Triggered when a scene collection is created, added, renamed, or removed.
 * 
 * @api events
 * @name SceneCollectionListChanged
 * @category scenes
 * @since 4.0.0
 */
void WSEvents::OnSceneCollectionListChange() {
    broadcastUpdate("SceneCollectionListChanged");
}

/**
 * The active transition has been changed.
 *
 * @return {String} `transition-name` The name of the new active transition.
 *
 * @api events
 * @name SwitchTransition
 * @category transitions
 * @since 4.0.0
 */
void WSEvents::OnTransitionChange() {
    obs_source_t* current_transition = obs_frontend_get_current_transition();
    connectTransitionSignals(current_transition);

    obs_data_t* data = obs_data_create();
    obs_data_set_string(data, "transition-name",
        obs_source_get_name(current_transition));

    broadcastUpdate("SwitchTransition", data);

    obs_data_release(data);
    obs_source_release(current_transition);
}

/**
 * The list of available transitions has been modified.
 * Transitions have been added, removed, or renamed.
 *
 * @api events
 * @name TransitionListChanged
 * @category transitions
 * @since 4.0.0
 */
void WSEvents::OnTransitionListChange() {
    broadcastUpdate("TransitionListChanged");
}

/**
 * Triggered when switching to another profile or when renaming the current profile.
 *
 * @api events
 * @name ProfileChanged
 * @category profiles
 * @since 4.0.0
 */
void WSEvents::OnProfileChange() {
    broadcastUpdate("ProfileChanged");
}

/**
 * Triggered when a profile is created, added, renamed, or removed.
 *
 * @api events
 * @name ProfileListChanged
 * @category profiles
 * @since 4.0.0
 */
void WSEvents::OnProfileListChange() {
    broadcastUpdate("ProfileListChanged");
}

/**
 * A request to start streaming has been issued.
 *
 * @return {boolean} `preview-only` Always false (retrocompatibility).
 *
 * @api events
 * @name StreamStarting
 * @category streaming
 * @since 0.3
 */
void WSEvents::OnStreamStarting() {
    obs_data_t* data = obs_data_create();
    obs_data_set_bool(data, "preview-only", false);

    broadcastUpdate("StreamStarting", data);

    obs_data_release(data);
}

/**
 * Streaming started successfully.
 *
 * @api events
 * @name StreamStarted
 * @category streaming
 * @since 0.3
 */
void WSEvents::OnStreamStarted() {
    _stream_starttime = os_gettime_ns();
    _lastBytesSent = 0;
    broadcastUpdate("StreamStarted");
}

/**
 * A request to stop streaming has been issued.
 * 
 * @return {boolean} `preview-only` Always false (retrocompatibility).
 *
 * @api events
 * @name StreamStopping
 * @category streaming
 * @since 0.3
 */
void WSEvents::OnStreamStopping() {
    obs_data_t* data = obs_data_create();
    obs_data_set_bool(data, "preview-only", false);

    broadcastUpdate("StreamStopping", data);

    obs_data_release(data);
}

/**
 * Streaming stopped successfully.
 *
 * @api events
 * @name StreamStopped
 * @category streaming
 * @since 0.3
 */
void WSEvents::OnStreamStopped() {
    _stream_starttime = 0;
    broadcastUpdate("StreamStopped");
}

/**
 * A request to start recording has been issued.
 *
 * @api events
 * @name RecordingStarting
 * @category recording
 * @since 0.3
 */
void WSEvents::OnRecordingStarting() {
    broadcastUpdate("RecordingStarting");
}

/**
 * Recording started successfully.
 *
 * @api events
 * @name RecordingStarted
 * @category recording
 * @since 0.3
 */
void WSEvents::OnRecordingStarted() {
    _rec_starttime = os_gettime_ns();
    broadcastUpdate("RecordingStarted");
}

/**
 * A request to stop recording has been issued.
 *
 * @api events
 * @name RecordingStopping
 * @category recording
 * @since 0.3
 */
void WSEvents::OnRecordingStopping() {
    broadcastUpdate("RecordingStopping");
}

/**
 * Recording stopped successfully.
 *
 * @api events
 * @name RecordingStopped
 * @category recording
 * @since 0.3
 */
void WSEvents::OnRecordingStopped() {
    _rec_starttime = 0;
    broadcastUpdate("RecordingStopped");
}

/**
* A request to start the replay buffer has been issued.
*
* @api events
* @name ReplayStarting
* @category replay buffer
* @since 4.2.0
*/
void WSEvents::OnReplayStarting() {
    broadcastUpdate("ReplayStarting");
}

/**
* Replay Buffer started successfully
*
* @api events
* @name ReplayStarted
* @category replay buffer
* @since 4.2.0
*/
void WSEvents::OnReplayStarted() {
    broadcastUpdate("ReplayStarted");
}

/**
* A request to start the replay buffer has been issued.
*
* @api events
* @name ReplayStopping
* @category replay buffer
* @since 4.2.0
*/
void WSEvents::OnReplayStopping() {
    broadcastUpdate("ReplayStopping");
}

/**
* Replay Buffer stopped successfully
*
* @api events
* @name ReplayStopped
* @category replay buffer
* @since 4.2.0
*/
void WSEvents::OnReplayStopped() {
    broadcastUpdate("ReplayStopped");
}

/**
 * OBS is exiting.
 *
 * @api events
 * @name Exiting
 * @category other
 * @since 0.3
 */
void WSEvents::OnExit() {
    broadcastUpdate("Exiting");
}

/**
 * Emit every 2 seconds.
 *
 * @return {boolean} `streaming` Current streaming state.
 * @return {boolean} `recording` Current recording state.
 * @return {boolean} `preview-only` Always false (retrocompatibility).
 * @return {int} `bytes-per-sec` Amount of data per second (in bytes) transmitted by the stream encoder.
 * @return {int} `kbits-per-sec` Amount of data per second (in kilobits) transmitted by the stream encoder.
 * @return {double} `strain` Percentage of dropped frames.
 * @return {int} `total-stream-time` Total time (in seconds) since the stream started.
 * @return {int} `num-total-frames` Total number of frames transmitted since the stream started.
 * @return {int} `num-dropped-frames` Number of frames dropped by the encoder since the stream started.
 * @return {double} `fps` Current framerate.
 *
 * @api events
 * @name StreamStatus
 * @category streaming
 * @since 0.3
 */
void WSEvents::StreamStatus() {
    bool streaming_active = obs_frontend_streaming_active();
    bool recording_active = obs_frontend_recording_active();

    obs_output_t* stream_output = obs_frontend_get_streaming_output();

    if (!stream_output || !streaming_active) {
        if (stream_output) {
            obs_output_release(stream_output);
        }
        return;
    }

    uint64_t bytes_sent = obs_output_get_total_bytes(stream_output);
    uint64_t bytes_sent_time = os_gettime_ns();

    if (bytes_sent < _lastBytesSent)
        bytes_sent = 0;

    if (bytes_sent == 0)
        _lastBytesSent = 0;

    uint64_t bytes_between = bytes_sent - _lastBytesSent;
    double time_passed =
        double(bytes_sent_time - _lastBytesSentTime) / 1000000000.0;

    uint64_t bytes_per_sec = bytes_between / time_passed;

    _lastBytesSent = bytes_sent;
    _lastBytesSentTime = bytes_sent_time;

    uint64_t totalStreamTime =
        (os_gettime_ns() - _stream_starttime) / 1000000000;

    int total_frames = obs_output_get_total_frames(stream_output);
    int dropped_frames = obs_output_get_frames_dropped(stream_output);

    float strain = obs_output_get_congestion(stream_output);

    obs_data_t* data = obs_data_create();
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

/**
 * Emitted every 2 seconds after enabling it by calling SetHeartbeat.
 * 
 * @return {boolean} `pulse` Toggles between every JSON meassage as an "I am alive" indicator.
 * @return {string (optional)} `current-profile` Current active profile.
 * @return {string (optional)} `current-scene` Current active scene.
 * @return {boolean (optional)} `streaming` Current streaming state.
 * @return {int (optional)} `total-stream-time` Total time (in seconds) since the stream started.
 * @return {int (optional)} `total-stream-bytes` Total bytes sent since the stream started.
 * @return {int (optional)} `total-stream-frames` Total frames streamed since the stream started.
 * @return {boolean (optional)} `recording` Current recording state.
 * @return {int (optional)} `total-record-time` Total time (in seconds) since recording started.
 * @return {int (optional)} `total-record-bytes` Total bytes recorded since the recording started.
 * @return {int (optional)} `total-record-frames` Total frames recorded since the recording started.
 * 
 * @api events
 * @name Heartbeat
 * @category general
 */
void WSEvents::Heartbeat() {

    if (!Heartbeat_active) return;

    bool streaming_active = obs_frontend_streaming_active();
    bool recording_active = obs_frontend_recording_active();
    obs_data_t* data = obs_data_create();
    obs_output_t* record_output = obs_frontend_get_recording_output();
    obs_output_t* stream_output = obs_frontend_get_streaming_output();

    pulse = !pulse;
    obs_data_set_bool(data, "pulse", pulse);

    obs_data_set_string(data, "current-profile", obs_frontend_get_current_profile());

    obs_source_t* current_scene = obs_frontend_get_current_scene();
    const char* name = obs_source_get_name(current_scene);
    obs_source_release(current_scene);
    obs_data_set_string(data, "current-scene", name);

    obs_data_set_bool(data, "streaming", streaming_active);
    if (streaming_active) {
        uint64_t totalStreamTime = (os_gettime_ns() - _stream_starttime) / 1000000000;
        obs_data_set_int(data, "total-stream-time", totalStreamTime);
        obs_data_set_int(data, "total-stream-bytes", (uint64_t)obs_output_get_total_bytes(stream_output));
        obs_data_set_int(data, "total-stream-frames", obs_output_get_total_frames(stream_output));
    }

    obs_data_set_bool(data, "recording", recording_active);
    if (recording_active) {
        uint64_t totalRecordTime = (os_gettime_ns() - _rec_starttime) / 1000000000;
        obs_data_set_int(data, "total-record-time", totalRecordTime);
        obs_data_set_int(data, "total-record-bytes", (uint64_t)obs_output_get_total_bytes(record_output));
        obs_data_set_int(data, "total-record-frames", obs_output_get_total_frames(record_output));
    }

    broadcastUpdate("Heartbeat", data);
    obs_data_release(data);
    obs_output_release(record_output);
    obs_output_release(stream_output);
}

/**
 * The active transition duration has been changed.
 *
 * @return {int} `new-duration` New transition duration.
 *
 * @api events
 * @name TransitionDurationChanged
 * @category transitions
 * @since 4.0.0
 */
void WSEvents::TransitionDurationChanged(int ms) {
    obs_data_t* fields = obs_data_create();
    obs_data_set_int(fields, "new-duration", ms);

    broadcastUpdate("TransitionDurationChanged", fields);
    obs_data_release(fields);
}

/**
 * A transition (other than "cut") has begun.
 *
 * @return {String} `name` Transition name.
 * @return {int} `duration` Transition duration (in milliseconds).
 *
 * @api events
 * @name TransitionBegin
 * @category transitions
 * @since 4.0.0
 */
void WSEvents::OnTransitionBegin(void* param, calldata_t* data) {
    UNUSED_PARAMETER(data);
    WSEvents* instance = static_cast<WSEvents*>(param);

    obs_source_t* current_transition = obs_frontend_get_current_transition();
    const char* name = obs_source_get_name(current_transition);
    int duration = Utils::GetTransitionDuration();

    obs_data_t* fields = obs_data_create();
    obs_data_set_string(fields, "name", name);
    obs_data_set_int(fields, "duration", duration);

    instance->broadcastUpdate("TransitionBegin", fields);
    obs_data_release(fields);
    obs_source_release(current_transition);
}

/**
 * Scene items have been reordered.
 *
 * @return {String} `scene-name` Name of the scene where items have been reordered.
 *
 * @api events
 * @name SourceOrderChanged
 * @category sources
 * @since 4.0.0
 */
void WSEvents::OnSceneReordered(void* param, calldata_t* data) {
    WSEvents* instance = static_cast<WSEvents*>(param);

    obs_scene_t* scene = nullptr;
    calldata_get_ptr(data, "scene", &scene);

    obs_data_t* fields = obs_data_create();
    obs_data_set_string(fields, "scene-name",
        obs_source_get_name(obs_scene_get_source(scene)));

    instance->broadcastUpdate("SourceOrderChanged", fields);
    obs_data_release(fields);
}

/**
 * An item has been added to the current scene.
 *
 * @return {String} `scene-name` Name of the scene.
 * @return {String} `item-name` Name of the item added to the scene.
 *
 * @api events
 * @name SceneItemAdded
 * @category sources
 * @since 4.0.0
 */
void WSEvents::OnSceneItemAdd(void* param, calldata_t* data) {
    WSEvents* instance = static_cast<WSEvents*>(param);

    obs_scene_t* scene = nullptr;
    calldata_get_ptr(data, "scene", &scene);

    obs_sceneitem_t* scene_item = nullptr;
    calldata_get_ptr(data, "item", &scene_item);

    const char* scene_name =
        obs_source_get_name(obs_scene_get_source(scene));
    const char* sceneitem_name =
        obs_source_get_name(obs_sceneitem_get_source(scene_item));

    obs_data_t* fields = obs_data_create();
    obs_data_set_string(fields, "scene-name", scene_name);
    obs_data_set_string(fields, "item-name", sceneitem_name);

    instance->broadcastUpdate("SceneItemAdded", fields);
    obs_data_release(fields);
}

/**
 * An item has been removed from the current scene.
 *
 * @return {String} `scene-name` Name of the scene.
 * @return {String} `item-name` Name of the item removed from the scene.
 *
 * @api events
 * @name SceneItemRemoved
 * @category sources
 * @since 4.0.0
 */
void WSEvents::OnSceneItemDelete(void* param, calldata_t* data) {
    WSEvents* instance = static_cast<WSEvents*>(param);

    obs_scene_t* scene = nullptr;
    calldata_get_ptr(data, "scene", &scene);

    obs_sceneitem_t* scene_item = nullptr;
    calldata_get_ptr(data, "item", &scene_item);

    const char* scene_name =
        obs_source_get_name(obs_scene_get_source(scene));
    const char* sceneitem_name =
        obs_source_get_name(obs_sceneitem_get_source(scene_item));

    obs_data_t* fields = obs_data_create();
    obs_data_set_string(fields, "scene-name", scene_name);
    obs_data_set_string(fields, "item-name", sceneitem_name);

    instance->broadcastUpdate("SceneItemRemoved", fields);
    obs_data_release(fields);
}

/**
 * An item's visibility has been toggled.
 *
 * @return {String} `scene-name` Name of the scene.
 * @return {String} `item-name` Name of the item in the scene.
 * @return {boolean} `item-visible` New visibility state of the item.
 *
 * @api events
 * @name SceneItemVisibilityChanged
 * @category sources
 * @since 4.0.0
 */
void WSEvents::OnSceneItemVisibilityChanged(void* param, calldata_t* data) {
    WSEvents* instance = static_cast<WSEvents*>(param);

    obs_scene_t* scene = nullptr;
    calldata_get_ptr(data, "scene", &scene);

    obs_sceneitem_t* scene_item = nullptr;
    calldata_get_ptr(data, "item", &scene_item);

    bool visible = false;
    calldata_get_bool(data, "visible", &visible);

    const char* scene_name =
        obs_source_get_name(obs_scene_get_source(scene));
    const char* sceneitem_name =
        obs_source_get_name(obs_sceneitem_get_source(scene_item));

    obs_data_t* fields = obs_data_create();
    obs_data_set_string(fields, "scene-name", scene_name);
    obs_data_set_string(fields, "item-name", sceneitem_name);
    obs_data_set_bool(fields, "item-visible", visible);

    instance->broadcastUpdate("SceneItemVisibilityChanged", fields);
    obs_data_release(fields);
}

/**
 * The selected preview scene has changed (only available in Studio Mode).
 *
 * @return {String} `scene-name` Name of the scene being previewed.
 * @return {Source|Array} `sources` List of sources composing the scene. Same specification as [`GetCurrentScene`](#getcurrentscene).
 *
 * @api events
 * @name PreviewSceneChanged
 * @category studio mode
 * @since 4.1.0
 */
void WSEvents::SelectedSceneChanged(QListWidgetItem* current, QListWidgetItem* prev) {
    if (obs_frontend_preview_program_mode_active()) {
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

/**
 * Studio Mode has been enabled or disabled.
 *
 * @return {boolean} `new-state` The new enabled state of Studio Mode.
 *
 * @api events
 * @name StudioModeSwitched
 * @category studio mode
 * @since 4.1.0
 */
void WSEvents::OnStudioModeSwitched(bool checked) {
    obs_data_t* data = obs_data_create();
    obs_data_set_bool(data, "new-state", checked);

    broadcastUpdate("StudioModeSwitched", data);
    obs_data_release(data);
}
