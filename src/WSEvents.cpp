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

#include <inttypes.h>
#include <util/platform.h>
#include <media-io/video-io.h>

#include <QtWidgets/QPushButton>

#include "WSEvents.h"

#include "obs-websocket.h"
#include "Config.h"
#include "Utils.h"
#include "rpc/RpcEvent.h"

#define STATUS_INTERVAL 2000

const char* sourceTypeToString(obs_source_type type) {
	switch (type) {
		case OBS_SOURCE_TYPE_INPUT:
			return "input";
		case OBS_SOURCE_TYPE_SCENE:
			return "scene";
		case OBS_SOURCE_TYPE_TRANSITION:
			return "transition";
		case OBS_SOURCE_TYPE_FILTER:
			return "filter";
		default:
			return "unknown";
	}
}

template <typename T> T* calldata_get_pointer(const calldata_t* data, const char* name) {
	void* ptr = nullptr;
	calldata_get_ptr(data, name, &ptr);
	return reinterpret_cast<T*>(ptr);
}

const char* calldata_get_string(const calldata_t* data, const char* name) {
	const char* value = nullptr;
	calldata_get_string(data, name, &value);
	return value;
}

WSEvents::WSEvents(WSServerPtr srv) :
	_srv(srv),
	_streamStarttime(0),
	_lastBytesSent(0),
	_lastBytesSentTime(0),
	HeartbeatIsActive(false),
	pulse(false)
{
	cpuUsageInfo = os_cpu_usage_info_start();
	obs_frontend_add_event_callback(WSEvents::FrontendEventHandler, this);

	QSpinBox* durationControl = Utils::GetTransitionDurationControl();
	connect(durationControl, SIGNAL(valueChanged(int)),
		this, SLOT(TransitionDurationChanged(int)));

	connect(&streamStatusTimer, SIGNAL(timeout()),
		this, SLOT(StreamStatus()));
	connect(&heartbeatTimer, SIGNAL(timeout()),
		this, SLOT(Heartbeat()));

	heartbeatTimer.start(STATUS_INTERVAL);

	// Connect to signals of all existing sources
	obs_enum_sources([](void* param, obs_source_t* source) {
		auto self = reinterpret_cast<WSEvents*>(param);
		self->connectSourceSignals(source);
		return true;
	}, this);

	signal_handler_t* coreSignalHandler = obs_get_signal_handler();
	if (coreSignalHandler) {
		signal_handler_connect(coreSignalHandler, "source_create", OnSourceCreate, this);
		signal_handler_connect(coreSignalHandler, "source_destroy", OnSourceDestroy, this);
	}
}

WSEvents::~WSEvents() {
	signal_handler_t* coreSignalHandler = obs_get_signal_handler();
	if (coreSignalHandler) {
		signal_handler_disconnect(coreSignalHandler, "source_destroy", OnSourceDestroy, this);
		signal_handler_disconnect(coreSignalHandler, "source_create", OnSourceCreate, this);
	}

	// Disconnect from signals of all existing sources
	obs_enum_sources([](void* param, obs_source_t* source) {
		auto self = reinterpret_cast<WSEvents*>(param);
		self->disconnectSourceSignals(source);
		return true;
	}, this);

	obs_frontend_remove_event_callback(WSEvents::FrontendEventHandler, this);
	os_cpu_usage_info_destroy(cpuUsageInfo);
}

void WSEvents::FrontendEventHandler(enum obs_frontend_event event, void* private_data) {
	auto owner = reinterpret_cast<WSEvents*>(private_data);

	if (!owner->_srv) {
		return;
	}

	switch (event) {
		case OBS_FRONTEND_EVENT_FINISHED_LOADING:
			owner->hookTransitionPlaybackEvents();
			break;
	
		case OBS_FRONTEND_EVENT_SCENE_CHANGED:
			owner->OnSceneChange();
			break;

		case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED:
			owner->OnSceneListChange();
			break;

		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED:
			owner->hookTransitionPlaybackEvents();
			owner->OnSceneCollectionChange();
			break;

		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_LIST_CHANGED:
			owner->OnSceneCollectionListChange();
			break;

		case OBS_FRONTEND_EVENT_TRANSITION_CHANGED:
			owner->OnTransitionChange();
			break;

		case OBS_FRONTEND_EVENT_TRANSITION_LIST_CHANGED:
			owner->hookTransitionPlaybackEvents();
			owner->OnTransitionListChange();
			break;

		case OBS_FRONTEND_EVENT_PROFILE_CHANGED:
			owner->OnProfileChange();
			break;

		case OBS_FRONTEND_EVENT_PROFILE_LIST_CHANGED:
			owner->OnProfileListChange();
			break;

		case OBS_FRONTEND_EVENT_STREAMING_STARTING:
			owner->OnStreamStarting();
			break;

		case OBS_FRONTEND_EVENT_STREAMING_STARTED:
			owner->streamStatusTimer.start(STATUS_INTERVAL);
			owner->StreamStatus();
			owner->OnStreamStarted();
			break;

		case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
			owner->streamStatusTimer.stop();
			owner->OnStreamStopping();
			break;

		case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
			owner->OnStreamStopped();
			break;

		case OBS_FRONTEND_EVENT_RECORDING_STARTING:
			owner->OnRecordingStarting();
			break;

		case OBS_FRONTEND_EVENT_RECORDING_STARTED:
			owner->OnRecordingStarted();
			break;

		case OBS_FRONTEND_EVENT_RECORDING_STOPPING:
			owner->OnRecordingStopping();
			break;

		case OBS_FRONTEND_EVENT_RECORDING_STOPPED:
			owner->OnRecordingStopped();
			break;

		case OBS_FRONTEND_EVENT_RECORDING_PAUSED:
			owner->OnRecordingPaused();
			break;

		case OBS_FRONTEND_EVENT_RECORDING_UNPAUSED:
			owner->OnRecordingResumed();
			break;

		case OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED:
			owner->OnVirtualCamStarted();
			break;

		case OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED:
			owner->OnVirtualCamStopped();
			break;

		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTING:
			owner->OnReplayStarting();
			break;

		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED:
			owner->OnReplayStarted();
			break;

		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPING:
			owner->OnReplayStopping();
			break;

		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED:
			owner->OnReplayStopped();
			break;

		case OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED:
			owner->OnStudioModeSwitched(true);
			break;

		case OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED:
			owner->OnStudioModeSwitched(false);
			break;

		case OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED:
			owner->OnPreviewSceneChanged();
			break;

		case OBS_FRONTEND_EVENT_EXIT:
			owner->unhookTransitionPlaybackEvents();
			owner->OnExit();
			owner->_srv->stop();
			break;
	}
}

void WSEvents::broadcastUpdate(const char* updateType,
	obs_data_t* additionalFields = nullptr)
{
	std::optional<uint64_t> streamTime;
	if (obs_frontend_streaming_active()) {
		streamTime = std::make_optional(getStreamingTime());
	}

	std::optional<uint64_t> recordingTime;
	if (obs_frontend_recording_active()) {
		recordingTime = std::make_optional(getRecordingTime());
	}

	RpcEvent event(QString(updateType), streamTime, recordingTime, additionalFields);
	_srv->broadcast(event);
}

void WSEvents::connectSourceSignals(obs_source_t* source) {
	if (!source) {
		return;
	}

	// Disconnect everything first to avoid double-binding
	disconnectSourceSignals(source);

	obs_source_type sourceType = obs_source_get_type(source);
	signal_handler_t* sh = obs_source_get_signal_handler(source);

	signal_handler_connect(sh, "rename", OnSourceRename, this);

	signal_handler_connect(sh, "mute", OnSourceMuteStateChange, this);
	signal_handler_connect(sh, "volume", OnSourceVolumeChange, this);
	signal_handler_connect(sh, "audio_sync", OnSourceAudioSyncOffsetChanged, this);
	signal_handler_connect(sh, "audio_mixers", OnSourceAudioMixersChanged, this);
	signal_handler_connect(sh, "audio_activate", OnSourceAudioActivated, this);
	signal_handler_connect(sh, "audio_deactivate", OnSourceAudioDeactivated, this);

	signal_handler_connect(sh, "filter_add", OnSourceFilterAdded, this);
	signal_handler_connect(sh, "filter_remove", OnSourceFilterRemoved, this);
	signal_handler_connect(sh, "reorder_filters", OnSourceFilterOrderChanged, this);
	
	signal_handler_connect(sh, "media_play", OnMediaPlaying, this);
	signal_handler_connect(sh, "media_pause", OnMediaPaused, this);
	signal_handler_connect(sh, "media_restart", OnMediaRestarted, this);
	signal_handler_connect(sh, "media_stopped", OnMediaStopped, this);
	signal_handler_connect(sh, "media_next", OnMediaNext, this);
	signal_handler_connect(sh, "media_previous", OnMediaPrevious, this);
	signal_handler_connect(sh, "media_started", OnMediaStarted, this);
	signal_handler_connect(sh, "media_ended", OnMediaEnded, this);

	if (sourceType == OBS_SOURCE_TYPE_SCENE) {
		signal_handler_connect(sh, "reorder", OnSceneReordered, this);
		signal_handler_connect(sh, "item_add", OnSceneItemAdd, this);
		signal_handler_connect(sh, "item_remove", OnSceneItemDelete, this);
		signal_handler_connect(sh,
			"item_visible", OnSceneItemVisibilityChanged, this);
		signal_handler_connect(sh,
			"item_locked", OnSceneItemLockChanged, this);
		signal_handler_connect(sh, "item_transform", OnSceneItemTransform, this);
		signal_handler_connect(sh, "item_select", OnSceneItemSelected, this);
		signal_handler_connect(sh, "item_deselect", OnSceneItemDeselected, this);
	}
}

void WSEvents::disconnectSourceSignals(obs_source_t* source) {
	if (!source) {
		return;
	}

	signal_handler_t* sh = obs_source_get_signal_handler(source);

	signal_handler_disconnect(sh, "rename", OnSourceRename, this);

	signal_handler_disconnect(sh, "mute", OnSourceMuteStateChange, this);
	signal_handler_disconnect(sh, "volume", OnSourceVolumeChange, this);
	signal_handler_disconnect(sh, "audio_sync", OnSourceAudioSyncOffsetChanged, this);
	signal_handler_disconnect(sh, "audio_mixers", OnSourceAudioMixersChanged, this);
	signal_handler_disconnect(sh, "audio_activate", OnSourceAudioActivated, this);
	signal_handler_disconnect(sh, "audio_deactivate", OnSourceAudioDeactivated, this);

	signal_handler_disconnect(sh, "filter_add", OnSourceFilterAdded, this);
	signal_handler_disconnect(sh, "filter_remove", OnSourceFilterRemoved, this);
	signal_handler_disconnect(sh, "reorder_filters", OnSourceFilterOrderChanged, this);

	signal_handler_disconnect(sh, "reorder", OnSceneReordered, this);
	signal_handler_disconnect(sh, "item_add", OnSceneItemAdd, this);
	signal_handler_disconnect(sh, "item_remove", OnSceneItemDelete, this);
	signal_handler_disconnect(sh,
		"item_visible", OnSceneItemVisibilityChanged, this);
	signal_handler_disconnect(sh,
		"item_locked", OnSceneItemLockChanged, this);
	signal_handler_disconnect(sh, "item_transform", OnSceneItemTransform, this);
	signal_handler_disconnect(sh, "item_select", OnSceneItemSelected, this);
	signal_handler_disconnect(sh, "item_deselect", OnSceneItemDeselected, this);

	signal_handler_disconnect(sh, "transition_start", OnTransitionBegin, this);
	signal_handler_disconnect(sh, "transition_stop", OnTransitionEnd, this);
	signal_handler_disconnect(sh, "transition_video_stop", OnTransitionVideoEnd, this);
	
	signal_handler_disconnect(sh, "media_play", OnMediaPlaying, this);
	signal_handler_disconnect(sh, "media_pause", OnMediaPaused, this);
	signal_handler_disconnect(sh, "media_restart", OnMediaRestarted, this);
	signal_handler_disconnect(sh, "media_stopped", OnMediaStopped, this);
	signal_handler_disconnect(sh, "media_next", OnMediaNext, this);
	signal_handler_disconnect(sh, "media_previous", OnMediaPrevious, this);
	signal_handler_disconnect(sh, "media_started", OnMediaStarted, this);
	signal_handler_disconnect(sh, "media_ended", OnMediaEnded, this);
}

void WSEvents::connectFilterSignals(obs_source_t* filter) {
	if (!filter) {
		return;
	}

	signal_handler_t* sh = obs_source_get_signal_handler(filter);

	signal_handler_connect(sh, "enable", OnSourceFilterVisibilityChanged, this);
}

void WSEvents::disconnectFilterSignals(obs_source_t* filter) {
	if (!filter) {
		return;
	}

	signal_handler_t* sh = obs_source_get_signal_handler(filter);

	signal_handler_disconnect(sh, "enable", OnSourceFilterVisibilityChanged, this);
}

void WSEvents::hookTransitionPlaybackEvents() {
	obs_frontend_source_list transitions = {};
	obs_frontend_get_transitions(&transitions);

	for (uint i = 0; i < transitions.sources.num; i++) {
		obs_source_t* transition = transitions.sources.array[i];
		signal_handler_t* sh = obs_source_get_signal_handler(transition);
		signal_handler_disconnect(sh, "transition_start", OnTransitionBegin, this);
		signal_handler_connect(sh, "transition_start", OnTransitionBegin, this);
		signal_handler_disconnect(sh, "transition_stop", OnTransitionEnd, this);
		signal_handler_connect(sh, "transition_stop", OnTransitionEnd, this);
		signal_handler_disconnect(sh, "transition_video_stop", OnTransitionVideoEnd, this);
		signal_handler_connect(sh, "transition_video_stop", OnTransitionVideoEnd, this);
	}

	obs_frontend_source_list_free(&transitions);
}

void WSEvents::unhookTransitionPlaybackEvents() {
	obs_frontend_source_list transitions = {};
	obs_frontend_get_transitions(&transitions);

	for (uint i = 0; i < transitions.sources.num; i++) {
		obs_source_t* transition = transitions.sources.array[i];
		signal_handler_t* sh = obs_source_get_signal_handler(transition);
		signal_handler_disconnect(sh, "transition_start", OnTransitionBegin, this);
		signal_handler_disconnect(sh, "transition_stop", OnTransitionEnd, this);
		signal_handler_disconnect(sh, "transition_video_stop", OnTransitionVideoEnd, this);
	}

	obs_frontend_source_list_free(&transitions);
}

uint64_t getOutputRunningTime(obs_output_t* output) {
	if (!output || !obs_output_active(output)) {
		return 0;
	}

	video_t* video = obs_output_video(output);
	uint64_t frameTimeNs = video_output_get_frame_time(video);
	int totalFrames = obs_output_get_total_frames(output);

	return (((uint64_t)totalFrames) * frameTimeNs);
}

uint64_t WSEvents::getStreamingTime() {
	OBSOutputAutoRelease streamingOutput = obs_frontend_get_streaming_output();
	return getOutputRunningTime(streamingOutput);
}

uint64_t WSEvents::getRecordingTime() {
	OBSOutputAutoRelease recordingOutput = obs_frontend_get_recording_output();
	return getOutputRunningTime(recordingOutput);
}

uint64_t WSEvents::getVirtualCamTime() {
	OBSOutputAutoRelease virtualCamOutput = obs_frontend_get_virtualcam_output();
	return getOutputRunningTime(virtualCamOutput);
}

QString WSEvents::getStreamingTimecode() {
	return Utils::nsToTimestamp(getStreamingTime());
}

QString WSEvents::getRecordingTimecode() {
	return Utils::nsToTimestamp(getRecordingTime());
}

QString WSEvents::getVirtualCamTimecode() {
	return Utils::nsToTimestamp(getVirtualCamTime());
}

OBSDataAutoRelease getMediaSourceData(calldata_t* data) {
	OBSDataAutoRelease fields = obs_data_create();
	OBSSource source = calldata_get_pointer<obs_source_t>(data, "source");

	obs_data_set_string(fields, "sourceName", obs_source_get_name(source));
	obs_data_set_string(fields, "sourceKind", obs_source_get_id(source));

	return fields;
}

 /**
 * Indicates a scene change.
 *
 * @return {String} `scene-name` The new scene.
 * @return {Array<SceneItem>} `sources` List of scene items in the new scene. Same specification as [`GetCurrentScene`](#getcurrentscene).
 *
 * @api events
 * @name SwitchScenes
 * @category scenes
 * @since 0.3
 */
void WSEvents::OnSceneChange() {
	OBSSourceAutoRelease currentScene = obs_frontend_get_current_scene();
	OBSDataArrayAutoRelease sceneItems = Utils::GetSceneItems(currentScene);

	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_string(data, "scene-name", obs_source_get_name(currentScene));
	obs_data_set_array(data, "sources", sceneItems);

	broadcastUpdate("SwitchScenes", data);
}

/**
 * The scene list has been modified.
 * Scenes have been added, removed, or renamed.
 *
 * Note: This event is not fired when the scenes are reordered.
 *
 * @return {Array<Scene>} `scenes` Scenes list.
 * 
 * @api events
 * @name ScenesChanged
 * @category scenes
 * @since 0.3
 */
void WSEvents::OnSceneListChange() {
	OBSDataArrayAutoRelease scenes = Utils::GetScenes();

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_array(fields, "scenes", scenes);
	broadcastUpdate("ScenesChanged", fields);
}

/**
 * Triggered when switching to another scene collection or when renaming the current scene collection.
 *
 * @return {String} `sceneCollection` Name of the new current scene collection.
 * 
 * @api events
 * @name SceneCollectionChanged
 * @category scenes
 * @since 4.0.0
 */
void WSEvents::OnSceneCollectionChange() {
	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "sceneCollection", obs_frontend_get_current_scene_collection());
	broadcastUpdate("SceneCollectionChanged", fields);

	OnTransitionListChange();
	OnTransitionChange();

	OnSceneListChange();
	OnSceneChange();
}

/**
 * Triggered when a scene collection is created, added, renamed, or removed.
 *
 * @return {Array<Object>} `sceneCollections` Scene collections list.
 * @return {String} `sceneCollections.*.name` Scene collection name.
 * 
 * @api events
 * @name SceneCollectionListChanged
 * @category scenes
 * @since 4.0.0
 */
void WSEvents::OnSceneCollectionListChange() {
	char** sceneCollections = obs_frontend_get_scene_collections();
	OBSDataArrayAutoRelease sceneCollectionsList =
		Utils::StringListToArray(sceneCollections, "name");
	bfree(sceneCollections);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_array(fields, "sceneCollections", sceneCollectionsList);
	broadcastUpdate("SceneCollectionListChanged", fields);
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
	OBSSourceAutoRelease currentTransition = obs_frontend_get_current_transition();

	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_string(data, "transition-name",
		obs_source_get_name(currentTransition));

	broadcastUpdate("SwitchTransition", data);
}

/**
 * The list of available transitions has been modified.
 * Transitions have been added, removed, or renamed.
 *
 * @return {Array<Object>} `transitions` Transitions list.
 * @return {String} `transitions.*.name` Transition name.
 * 
 * @api events
 * @name TransitionListChanged
 * @category transitions
 * @since 4.0.0
 */
void WSEvents::OnTransitionListChange() {
	obs_frontend_source_list transitionList = {};
	obs_frontend_get_transitions(&transitionList);

	OBSDataArrayAutoRelease transitions = obs_data_array_create();
	for (size_t i = 0; i < transitionList.sources.num; i++) {
		OBSSource transition = transitionList.sources.array[i];

		OBSDataAutoRelease obj = obs_data_create();
		obs_data_set_string(obj, "name", obs_source_get_name(transition));
		obs_data_array_push_back(transitions, obj);
	}
	obs_frontend_source_list_free(&transitionList);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_array(fields, "transitions", transitions);
	broadcastUpdate("TransitionListChanged", fields);
}

/**
 * Triggered when switching to another profile or when renaming the current profile.
 *
 * @return {String} `profile` Name of the new current profile.
 * 
 * @api events
 * @name ProfileChanged
 * @category profiles
 * @since 4.0.0
 */
void WSEvents::OnProfileChange() {
	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "profile", obs_frontend_get_current_profile());
	broadcastUpdate("ProfileChanged", fields);
}

/**
 * Triggered when a profile is created, added, renamed, or removed.
 *
 * @return {Array<Object>} `profiles` Profiles list.
 * @return {String} `profiles.*.name` Profile name.
 * 
 * @api events
 * @name ProfileListChanged
 * @category profiles
 * @since 4.0.0
 */
void WSEvents::OnProfileListChange() {
	char** profiles = obs_frontend_get_profiles();
	OBSDataArrayAutoRelease profilesList = Utils::StringListToArray(profiles, "name");
	bfree(profiles);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_array(fields, "profiles", profilesList);
	broadcastUpdate("ProfileListChanged", fields);
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
	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_bool(data, "preview-only", false);

	broadcastUpdate("StreamStarting", data);
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
	_streamStarttime = os_gettime_ns();
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
	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_bool(data, "preview-only", false);
	broadcastUpdate("StreamStopping", data);
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
	_streamStarttime = 0;

	broadcastUpdate("StreamStopped");
}

/**
 * A request to start recording has been issued.
 * 
 * Note: `recordingFilename` is not provided in this event because this information
 * is not available at the time this event is emitted.
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
 * @return {String} `recordingFilename` Absolute path to the file of the current recording.
 * 
 * @api events
 * @name RecordingStarted
 * @category recording
 * @since 0.3
 */
void WSEvents::OnRecordingStarted() {
	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_string(data, "recordingFilename", Utils::GetCurrentRecordingFilename());
	broadcastUpdate("RecordingStarted", data);
}

/**
 * A request to stop recording has been issued.
 *
 * @return {String} `recordingFilename` Absolute path to the file of the current recording.
 * 
 * @api events
 * @name RecordingStopping
 * @category recording
 * @since 0.3
 */
void WSEvents::OnRecordingStopping() {
	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_string(data, "recordingFilename", Utils::GetCurrentRecordingFilename());
	broadcastUpdate("RecordingStopping", data);
}

/**
 * Recording stopped successfully.
 *
 * @return {String} `recordingFilename` Absolute path to the file of the current recording.
 * 
 * @api events
 * @name RecordingStopped
 * @category recording
 * @since 0.3
 */
void WSEvents::OnRecordingStopped() {
	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_string(data, "recordingFilename", Utils::GetCurrentRecordingFilename());
	broadcastUpdate("RecordingStopped", data);
}

/**
 * Current recording paused
 *
 * @api events
 * @name RecordingPaused
 * @category recording
 * @since 4.7.0
 */
void WSEvents::OnRecordingPaused() {
	broadcastUpdate("RecordingPaused");
}

/**
 * Current recording resumed
 *
 * @api events
 * @name RecordingResumed
 * @category recording
 * @since 4.7.0
 */
void WSEvents::OnRecordingResumed() {
	broadcastUpdate("RecordingResumed");
}

/**
 * Virtual cam started successfully.
 *
 * @api events
 * @name VirtualCamStarted
 * @category virtual cam
 * @since 4.9.1
 */
void WSEvents::OnVirtualCamStarted() {
	broadcastUpdate("VirtualCamStarted");
}

/**
 * Virtual cam stopped successfully.
 *
 * @api events
 * @name VirtualCamStopped
 * @category virtual cam
 * @since 4.9.1
 */
void WSEvents::OnVirtualCamStopped() {
	broadcastUpdate("VirtualCamStopped");
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
* A request to stop the replay buffer has been issued.
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
 * Emitted every 2 seconds when stream is active.
 *
 * @return {boolean} `streaming` Current streaming state.
 * @return {boolean} `recording` Current recording state.
 * @return {boolean} `replay-buffer-active` Replay Buffer status
 * @return {int} `bytes-per-sec` Amount of data per second (in bytes) transmitted by the stream encoder.
 * @return {int} `kbits-per-sec` Amount of data per second (in kilobits) transmitted by the stream encoder.
 * @return {double} `strain` Percentage of dropped frames.
 * @return {int} `total-stream-time` Total time (in seconds) since the stream started.
 * @return {int} `num-total-frames` Total number of frames transmitted since the stream started.
 * @return {int} `num-dropped-frames` Number of frames dropped by the encoder since the stream started.
 * @return {double} `fps` Current framerate.
 * @return {int} `render-total-frames` Number of frames rendered
 * @return {int} `render-missed-frames` Number of frames missed due to rendering lag
 * @return {int} `output-total-frames` Number of frames outputted
 * @return {int} `output-skipped-frames` Number of frames skipped due to encoding lag
 * @return {double} `average-frame-time` Average frame time (in milliseconds)
 * @return {double} `cpu-usage` Current CPU usage (percentage)
 * @return {double} `memory-usage` Current RAM usage (in megabytes)
 * @return {double} `free-disk-space` Free recording disk space (in megabytes)
 * @return {boolean} `preview-only` Always false (retrocompatibility).
 *
 * @api events
 * @name StreamStatus
 * @category streaming
 * @since 0.3
 */
void WSEvents::StreamStatus() {
	bool streamingActive = obs_frontend_streaming_active();
	bool recordingActive = obs_frontend_recording_active();
	bool recordingPaused = obs_frontend_recording_paused();
	bool replayBufferActive = obs_frontend_replay_buffer_active();

	OBSOutputAutoRelease streamOutput = obs_frontend_get_streaming_output();

	if (!streamOutput || !streamingActive) {
		return;
	}

	uint64_t bytesSent = obs_output_get_total_bytes(streamOutput);
	uint64_t bytesSentTime = os_gettime_ns();

	if (bytesSent < _lastBytesSent)
		bytesSent = 0;

	if (bytesSent == 0)
		_lastBytesSent = 0;

	uint64_t bytesBetween = bytesSent - _lastBytesSent;
	double timePassed =
		double(bytesSentTime - _lastBytesSentTime) / 1000000000.0;

	uint64_t bytesPerSec = bytesBetween / timePassed;

	_lastBytesSent = bytesSent;
	_lastBytesSentTime = bytesSentTime;

	uint64_t totalStreamTime = (getStreamingTime() / 1000000000ULL);
	int totalFrames = obs_output_get_total_frames(streamOutput);
	int droppedFrames = obs_output_get_frames_dropped(streamOutput);

	float strain = obs_output_get_congestion(streamOutput);

	OBSDataAutoRelease data = obs_data_create();

	obs_data_set_bool(data, "streaming", streamingActive);
	obs_data_set_bool(data, "recording", recordingActive);
	obs_data_set_bool(data, "recording-paused", recordingPaused);
	obs_data_set_bool(data, "replay-buffer-active", replayBufferActive);

	obs_data_set_int(data, "bytes-per-sec", bytesPerSec);
	obs_data_set_int(data, "kbits-per-sec", (bytesPerSec * 8) / 1024);

	obs_data_set_int(data, "total-stream-time", totalStreamTime);

	obs_data_set_int(data, "num-total-frames", totalFrames);
	obs_data_set_int(data, "num-dropped-frames", droppedFrames);
	obs_data_set_double(data, "strain", strain);

	// `stats` contains fps, cpu usage, memory usage, render missed frames, ...
	OBSDataAutoRelease stats = GetStats();
	obs_data_apply(data, stats);

	obs_data_set_bool(data, "preview-only", false); // Retrocompat with OBSRemote

	broadcastUpdate("StreamStatus", data);
}

/**
 * Emitted every 2 seconds after enabling it by calling SetHeartbeat.
 *
 * @return {boolean} `pulse` Toggles between every JSON message as an "I am alive" indicator.
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
 * @return {OBSStats} `stats` OBS Stats
 *
 * @api events
 * @name Heartbeat
 * @category general
 * @since v0.3
 */
void WSEvents::Heartbeat() {

	if (!HeartbeatIsActive) return;

	bool streamingActive = obs_frontend_streaming_active();
	bool recordingActive = obs_frontend_recording_active();
	bool recordingPaused = obs_frontend_recording_paused();

	OBSDataAutoRelease data = obs_data_create();
	OBSOutputAutoRelease recordOutput = obs_frontend_get_recording_output();
	OBSOutputAutoRelease streamOutput = obs_frontend_get_streaming_output();

	pulse = !pulse;
	obs_data_set_bool(data, "pulse", pulse);

	char* currentProfile = obs_frontend_get_current_profile();
	obs_data_set_string(data, "current-profile", currentProfile);
	bfree(currentProfile);

	OBSSourceAutoRelease currentScene = obs_frontend_get_current_scene();
	obs_data_set_string(data, "current-scene", obs_source_get_name(currentScene));

	obs_data_set_bool(data, "streaming", streamingActive);
	if (streamingActive) {
		obs_data_set_int(data, "total-stream-time", (getStreamingTime() / 1000000000ULL));
		obs_data_set_int(data, "total-stream-bytes", (uint64_t)obs_output_get_total_bytes(streamOutput));
		obs_data_set_int(data, "total-stream-frames", obs_output_get_total_frames(streamOutput));
	}

	obs_data_set_bool(data, "recording", recordingActive);
	obs_data_set_bool(data, "recording-paused", recordingPaused);
	if (recordingActive) {
		obs_data_set_int(data, "total-record-time", (getRecordingTime() / 1000000000ULL));
		obs_data_set_int(data, "total-record-bytes", (uint64_t)obs_output_get_total_bytes(recordOutput));
		obs_data_set_int(data, "total-record-frames", obs_output_get_total_frames(recordOutput));
	}

	OBSDataAutoRelease stats = GetStats();
	obs_data_set_obj(data, "stats", stats);

	broadcastUpdate("Heartbeat", data);
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
	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_int(fields, "new-duration", ms);

	broadcastUpdate("TransitionDurationChanged", fields);
}

/**
 * A transition (other than "cut") has begun.
 *
 * @return {String} `name` Transition name.
 * @return {String} `type` Transition type.
 * @return {int} `duration` Transition duration (in milliseconds). 
 * Will be -1 for any transition with a fixed duration, 
 * such as a Stinger, due to limitations of the OBS API.
 * @return {String} `from-scene` Source scene of the transition
 * @return {String} `to-scene` Destination scene of the transition
 *
 * @api events
 * @name TransitionBegin
 * @category transitions
 * @since 4.0.0
 */
void WSEvents::OnTransitionBegin(void* param, calldata_t* data) {
	auto instance = reinterpret_cast<WSEvents*>(param);

	OBSSource transition = calldata_get_pointer<obs_source_t>(data, "source");
	if (!transition) {
		return;
	}

	OBSDataAutoRelease fields = Utils::GetTransitionData(transition);
	instance->broadcastUpdate("TransitionBegin", fields);
}

/**
* A transition (other than "cut") has ended.
* Note: The `from-scene` field is not available in TransitionEnd.
*
* @return {String} `name` Transition name.
* @return {String} `type` Transition type.
* @return {int} `duration` Transition duration (in milliseconds).
* @return {String} `to-scene` Destination scene of the transition
*
* @api events
* @name TransitionEnd
* @category transitions
* @since 4.8.0
*/
void WSEvents::OnTransitionEnd(void* param, calldata_t* data) {
	auto instance = reinterpret_cast<WSEvents*>(param);

	OBSSource transition = calldata_get_pointer<obs_source_t>(data, "source");
	if (!transition) {
		return;
	}

	OBSDataAutoRelease fields = Utils::GetTransitionData(transition);
	instance->broadcastUpdate("TransitionEnd", fields);
}

/**
* A stinger transition has finished playing its video.
*
* @return {String} `name` Transition name.
* @return {String} `type` Transition type.
* @return {int} `duration` Transition duration (in milliseconds).
* @return {String} `from-scene` Source scene of the transition
* @return {String} `to-scene` Destination scene of the transition
*
* @api events
* @name TransitionVideoEnd
* @category transitions
* @since 4.8.0
*/
void WSEvents::OnTransitionVideoEnd(void* param, calldata_t* data) {
	auto instance = reinterpret_cast<WSEvents*>(param);

	OBSSource transition = calldata_get_pointer<obs_source_t>(data, "source");
	if (!transition) {
		return;
	}

	OBSDataAutoRelease fields = Utils::GetTransitionData(transition);
	instance->broadcastUpdate("TransitionVideoEnd", fields);
}

/**
 * A source has been created. A source can be an input, a scene or a transition.
 *
 * @return {String} `sourceName` Source name
 * @return {String} `sourceType` Source type. Can be "input", "scene", "transition" or "filter".
 * @return {String} `sourceKind` Source kind.
 * @return {Object} `sourceSettings` Source settings
 *
 * @api events
 * @name SourceCreated
 * @category sources
 * @since 4.6.0
 */
void WSEvents::OnSourceCreate(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSSource source = calldata_get_pointer<obs_source_t>(data, "source");
	if (!source) {
		return;
	}

	self->connectSourceSignals(source);

	OBSDataAutoRelease sourceSettings = obs_source_get_settings(source);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "sourceName", obs_source_get_name(source));
	obs_data_set_string(fields, "sourceType",
		sourceTypeToString(obs_source_get_type(source))
	);
	obs_data_set_string(fields, "sourceKind", obs_source_get_id(source));
	obs_data_set_obj(fields, "sourceSettings", sourceSettings);
	self->broadcastUpdate("SourceCreated", fields);
}

/**
 * A source has been destroyed/removed. A source can be an input, a scene or a transition.
 *
 * @return {String} `sourceName` Source name
 * @return {String} `sourceType` Source type. Can be "input", "scene", "transition" or "filter".
 * @return {String} `sourceKind` Source kind.
 *
 * @api events
 * @name SourceDestroyed
 * @category sources
 * @since 4.6.0
 */
void WSEvents::OnSourceDestroy(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	obs_source_t* source = calldata_get_pointer<obs_source_t>(data, "source");
	if (!source) {
		return;
	}

	self->disconnectSourceSignals(source);

	obs_source_type sourceType = obs_source_get_type(source);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "sourceName", obs_source_get_name(source));
	obs_data_set_string(fields, "sourceType", sourceTypeToString(sourceType));
	obs_data_set_string(fields, "sourceKind", obs_source_get_id(source));
	self->broadcastUpdate("SourceDestroyed", fields);
}

/**
 * The volume of a source has changed.
 *
 * @return {String} `sourceName` Source name
 * @return {float} `volume` Source volume
 *
 * @api events
 * @name SourceVolumeChanged
 * @category sources
 * @since 4.6.0
 */
void WSEvents::OnSourceVolumeChange(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSSource source = calldata_get_pointer<obs_source_t>(data, "source");
	if (!source) {
		return;
	}

	double volume = 0;
	if (!calldata_get_float(data, "volume", &volume)) {
		return;
	}

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "sourceName", obs_source_get_name(source));
	obs_data_set_double(fields, "volume", volume);
	self->broadcastUpdate("SourceVolumeChanged", fields);
}

/**
 * A source has been muted or unmuted.
 *
 * @return {String} `sourceName` Source name
 * @return {boolean} `muted` Mute status of the source
 *
 * @api events
 * @name SourceMuteStateChanged
 * @category sources
 * @since 4.6.0
 */
void WSEvents::OnSourceMuteStateChange(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSSource source = calldata_get_pointer<obs_source_t>(data, "source");
	if (!source) {
		return;
	}

	bool muted = false;
	if (!calldata_get_bool(data, "muted", &muted)) {
		return;
	}

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "sourceName", obs_source_get_name(source));
	obs_data_set_bool(fields, "muted", muted);
	self->broadcastUpdate("SourceMuteStateChanged", fields);
}

/**
 * A source has removed audio.
 *
 * @return {String} `sourceName` Source name
 *
 * @api events
 * @name SourceAudioDeactivated
 * @category sources
 * @since 4.9.0
 */
void WSEvents::OnSourceAudioDeactivated(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSSource source = calldata_get_pointer<obs_source_t>(data, "source");
	if (!source) {
		return;
	}

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "sourceName", obs_source_get_name(source));
	self->broadcastUpdate("SourceAudioDeactivated", fields);
}

/**
 * A source has added audio.
 *
 * @return {String} `sourceName` Source name
 *
 * @api events
 * @name SourceAudioActivated
 * @category sources
 * @since 4.9.0
 */
void WSEvents::OnSourceAudioActivated(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSSource source = calldata_get_pointer<obs_source_t>(data, "source");
	if (!source) {
		return;
	}

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "sourceName", obs_source_get_name(source));
	self->broadcastUpdate("SourceAudioActivated", fields);
}

/**
 * The audio sync offset of a source has changed.
 *
 * @return {String} `sourceName` Source name
 * @return {int} `syncOffset` Audio sync offset of the source (in nanoseconds)
 *
 * @api events
 * @name SourceAudioSyncOffsetChanged
 * @category sources
 * @since 4.6.0
 */
void WSEvents::OnSourceAudioSyncOffsetChanged(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSSource source = calldata_get_pointer<obs_source_t>(data, "source");
	if (!source) {
		return;
	}

	long long syncOffset;
	if (!calldata_get_int(data, "offset", &syncOffset)) {
		return;
	}

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "sourceName", obs_source_get_name(source));
	obs_data_set_int(fields, "syncOffset", (int)syncOffset);
	self->broadcastUpdate("SourceAudioSyncOffsetChanged", fields);
}

/**
 * Audio mixer routing changed on a source.
 *
 * @return {String} `sourceName` Source name
 * @return {Array<Object>} `mixers` Routing status of the source for each audio mixer (array of 6 values)
 * @return {int} `mixers.*.id` Mixer number
 * @return {boolean} `mixers.*.enabled` Routing status
 * @return {String} `hexMixersValue` Raw mixer flags (little-endian, one bit per mixer) as an hexadecimal value
 *
 * @api events
 * @name SourceAudioMixersChanged
 * @category sources
 * @since 4.6.0
 */
void WSEvents::OnSourceAudioMixersChanged(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSSource source = calldata_get_pointer<obs_source_t>(data, "source");
	if (!source) {
		return;
	}

	long long audioMixers;
	if (!calldata_get_int(data, "mixers", &audioMixers)) {
		return;
	}

	OBSDataArrayAutoRelease mixers = obs_data_array_create();
	for (size_t i = 0; i < MAX_AUDIO_MIXES; i++) {
		OBSDataAutoRelease item = obs_data_create();
		obs_data_set_int(item, "id", i + 1);
		obs_data_set_bool(item, "enabled", (1 << i) & audioMixers);
		obs_data_array_push_back(mixers, item);
	}

	const QString hexValue = QString::number(audioMixers, 16).toUpper().prepend("0x");

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "sourceName", obs_source_get_name(source));
	obs_data_set_array(fields, "mixers", mixers);
	obs_data_set_string(fields, "hexMixersValue", hexValue.toUtf8());
	self->broadcastUpdate("SourceAudioMixersChanged", fields);
}

/**
 * A source has been renamed.
 *
 * @return {String} `previousName` Previous source name
 * @return {String} `newName` New source name
 * @return {String} `sourceType` Type of source (input, scene, filter, transition)
 *
 * @api events
 * @name SourceRenamed
 * @category sources
 * @since 4.6.0
 */
void WSEvents::OnSourceRename(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSSource source = calldata_get_pointer<obs_source_t>(data, "source");
	if (!source) {
		return;
	}

	const char* newName = calldata_get_string(data, "new_name");
	if (!newName) {
		return;
	}

	const char* previousName = calldata_get_string(data, "prev_name");

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "previousName", previousName);
	obs_data_set_string(fields, "newName", newName);
	obs_data_set_string(fields, "sourceType",
						sourceTypeToString(obs_source_get_type(source))); // TODO: Split into dedicated events for source/scene. Only doing it this way for backwards compatability until 5.0
	self->broadcastUpdate("SourceRenamed", fields);
}

/**
 * A filter was added to a source.
 *
 * @return {String} `sourceName` Source name
 * @return {String} `filterName` Filter name
 * @return {String} `filterType` Filter type
 * @return {Object} `filterSettings` Filter settings
 *
 * @api events
 * @name SourceFilterAdded
 * @category sources
 * @since 4.6.0
 */
void WSEvents::OnSourceFilterAdded(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSSource source = calldata_get_pointer<obs_source_t>(data, "source");
	if (!source) {
		return;
	}

	OBSSource filter = calldata_get_pointer<obs_source_t>(data, "filter");
	if (!filter) {
		return;
	}
	
	self->connectFilterSignals(filter);

	OBSDataAutoRelease filterSettings = obs_source_get_settings(filter);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "sourceName", obs_source_get_name(source));
	obs_data_set_string(fields, "filterName", obs_source_get_name(filter));
	obs_data_set_string(fields, "filterType", obs_source_get_id(filter));
	obs_data_set_obj(fields, "filterSettings", filterSettings);
	self->broadcastUpdate("SourceFilterAdded", fields);
}

/**
 * A filter was removed from a source.
 *
 * @return {String} `sourceName` Source name
 * @return {String} `filterName` Filter name
 * @return {String} `filterType` Filter type
 *
 * @api events
 * @name SourceFilterRemoved
 * @category sources
 * @since 4.6.0
 */
void WSEvents::OnSourceFilterRemoved(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	obs_source_t* source = calldata_get_pointer<obs_source_t>(data, "source");
	if (!source) {
		return;
	}

	obs_source_t* filter = calldata_get_pointer<obs_source_t>(data, "filter");
	if (!filter) {
		return;
	}

	self->disconnectFilterSignals(filter);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "sourceName", obs_source_get_name(source));
	obs_data_set_string(fields, "filterName", obs_source_get_name(filter));
	obs_data_set_string(fields, "filterType", obs_source_get_id(filter));
	self->broadcastUpdate("SourceFilterRemoved", fields);
}

/**
 * The visibility/enabled state of a filter changed
 *
 * @return {String} `sourceName` Source name
 * @return {String} `filterName` Filter name
 * @return {Boolean} `filterEnabled` New filter state
 *
 * @api events
 * @name SourceFilterVisibilityChanged
 * @category sources
 * @since 4.7.0
 */
void WSEvents::OnSourceFilterVisibilityChanged(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSSource source = calldata_get_pointer<obs_source_t>(data, "source");
	if (!source) {
		return;
	}

	OBSSource parent = obs_filter_get_parent(source);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "sourceName", obs_source_get_name(parent));
	obs_data_set_string(fields, "filterName", obs_source_get_name(source));
	obs_data_set_bool(fields, "filterEnabled", obs_source_enabled(source));

	self->broadcastUpdate("SourceFilterVisibilityChanged", fields);
}

/**
 * Filters in a source have been reordered.
 *
 * @return {String} `sourceName` Source name
 * @return {Array<Object>} `filters` Ordered Filters list
 * @return {String} `filters.*.name` Filter name
 * @return {String} `filters.*.type` Filter type
 * @return {boolean} `filters.*.enabled` Filter visibility status
 *
 * @api events
 * @name SourceFiltersReordered
 * @category sources
 * @since 4.6.0
 */
void WSEvents::OnSourceFilterOrderChanged(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSSource source = calldata_get_pointer<obs_source_t>(data, "source");
	if (!source) {
		return;
	}

	OBSDataArrayAutoRelease filters = Utils::GetSourceFiltersList(source, false);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "sourceName", obs_source_get_name(source));
	obs_data_set_array(fields, "filters", filters);
	self->broadcastUpdate("SourceFiltersReordered", fields);
}

/**
 * A media source has started playing.
 *
 * Note: This event is only emitted when something actively controls the media/VLC source. In other words, the source will never emit this on its own naturally.
 *
 * @return {String} `sourceName` Source name
 * @return {String} `sourceKind` The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`)
 *
 * @api events
 * @name MediaPlaying
 * @category media
 * @since 4.9.0
 */
void WSEvents::OnMediaPlaying(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSDataAutoRelease fields = getMediaSourceData(data);

	self->broadcastUpdate("MediaPlaying", fields);
}

/**
 * A media source has been paused.
 *
 * Note: This event is only emitted when something actively controls the media/VLC source. In other words, the source will never emit this on its own naturally.
 *
 * @return {String} `sourceName` Source name
 * @return {String} `sourceKind` The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`)
 *
 * @api events
 * @name MediaPaused
 * @category media
 * @since 4.9.0
 */
void WSEvents::OnMediaPaused(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSDataAutoRelease fields = getMediaSourceData(data);

	self->broadcastUpdate("MediaPaused", fields);
}

/**
 * A media source has been restarted.
 *
 * Note: This event is only emitted when something actively controls the media/VLC source. In other words, the source will never emit this on its own naturally.
 *
 * @return {String} `sourceName` Source name
 * @return {String} `sourceKind` The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`)
 *
 * @api events
 * @name MediaRestarted
 * @category media
 * @since 4.9.0
 */
void WSEvents::OnMediaRestarted(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSDataAutoRelease fields = getMediaSourceData(data);

	self->broadcastUpdate("MediaRestarted", fields);
}

/**
 * A media source has been stopped.
 *
 * Note: This event is only emitted when something actively controls the media/VLC source. In other words, the source will never emit this on its own naturally.
 *
 * @return {String} `sourceName` Source name
 * @return {String} `sourceKind` The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`)
 *
 * @api events
 * @name MediaStopped
 * @category media
 * @since 4.9.0
 */
void WSEvents::OnMediaStopped(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSDataAutoRelease fields = getMediaSourceData(data);

	self->broadcastUpdate("MediaStopped", fields);
}

/**
 * A media source has gone to the next item in the playlist.
 *
 * Note: This event is only emitted when something actively controls the media/VLC source. In other words, the source will never emit this on its own naturally.
 *
 * @return {String} `sourceName` Source name
 * @return {String} `sourceKind` The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`)
 *
 * @api events
 * @name MediaNext
 * @category media
 * @since 4.9.0
 */
void WSEvents::OnMediaNext(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSDataAutoRelease fields = getMediaSourceData(data);

	self->broadcastUpdate("MediaNext", fields);
}

/**
 * A media source has gone to the previous item in the playlist.
 *
 * Note: This event is only emitted when something actively controls the media/VLC source. In other words, the source will never emit this on its own naturally.
 *
 * @return {String} `sourceName` Source name
 * @return {String} `sourceKind` The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`)
 *
 * @api events
 * @name MediaPrevious
 * @category media
 * @since 4.9.0
 */
void WSEvents::OnMediaPrevious(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSDataAutoRelease fields = getMediaSourceData(data);

	self->broadcastUpdate("MediaPrevious", fields);
}

/**
 * A media source has been started.
 *
 * Note: These events are emitted by the OBS sources themselves. For example when the media file starts playing. The behavior depends on the type of media source being used.
 *
 * @return {String} `sourceName` Source name
 * @return {String} `sourceKind` The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`)
 *
 * @api events
 * @name MediaStarted
 * @category media
 * @since 4.9.0
 */
void WSEvents::OnMediaStarted(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSDataAutoRelease fields = getMediaSourceData(data);

	self->broadcastUpdate("MediaStarted", fields);
}

/**
 * A media source has ended.
 *
 * Note: These events are emitted by the OBS sources themselves. For example when the media file ends. The behavior depends on the type of media source being used.
 *
 * @return {String} `sourceName` Source name
 * @return {String} `sourceKind` The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`)
 *
 * @api events
 * @name MediaEnded
 * @category media
 * @since 4.9.0
 */
void WSEvents::OnMediaEnded(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSDataAutoRelease fields = getMediaSourceData(data);

	self->broadcastUpdate("MediaEnded", fields);
}

/**
 * Scene items within a scene have been reordered.
 *
 * @return {String} `scene-name` Name of the scene where items have been reordered.
 * @return {Array<Object>} `scene-items` Ordered list of scene items
 * @return {String} `scene-items.*.source-name` Item source name
 * @return {int} `scene-items.*.item-id` Scene item unique ID
 *
 * @api events
 * @name SourceOrderChanged
 * @category scene items
 * @since 4.0.0
 */
void WSEvents::OnSceneReordered(void* param, calldata_t* data) {
	auto instance = reinterpret_cast<WSEvents*>(param);

	OBSScene scene = calldata_get_pointer<obs_scene_t>(data, "scene");
	if (!scene) {
		return;
	}

	OBSDataArrayAutoRelease sceneItems = obs_data_array_create();
	obs_scene_enum_items(scene, [](obs_scene_t* scene, obs_sceneitem_t* sceneItem, void* param) {
		obs_data_array_t* sceneItems = reinterpret_cast<obs_data_array_t*>(param);

		OBSSource itemSource = obs_sceneitem_get_source(sceneItem);

		OBSDataAutoRelease item = obs_data_create();
		obs_data_set_string(item, "source-name", obs_source_get_name(itemSource));
		obs_data_set_int(item, "item-id", obs_sceneitem_get_id(sceneItem));
		obs_data_array_push_back(sceneItems, item);

		return true;
	}, sceneItems);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "scene-name",
		obs_source_get_name(obs_scene_get_source(scene)));
	obs_data_set_array(fields, "scene-items", sceneItems);

	instance->broadcastUpdate("SourceOrderChanged", fields);
}

/**
 * A scene item has been added to a scene.
 *
 * @return {String} `scene-name` Name of the scene.
 * @return {String} `item-name` Name of the item added to the scene.
 * @return {int} `item-id` Scene item ID
 *
 * @api events
 * @name SceneItemAdded
 * @category scene items
 * @since 4.0.0
 */
void WSEvents::OnSceneItemAdd(void* param, calldata_t* data) {
	auto instance = reinterpret_cast<WSEvents*>(param);

	obs_scene_t* scene = nullptr;
	calldata_get_ptr(data, "scene", &scene);

	obs_sceneitem_t* sceneItem = nullptr;
	calldata_get_ptr(data, "item", &sceneItem);

	const char* sceneName =
		obs_source_get_name(obs_scene_get_source(scene));
	const char* sceneItemName =
		obs_source_get_name(obs_sceneitem_get_source(sceneItem));

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "scene-name", sceneName);
	obs_data_set_string(fields, "item-name", sceneItemName);
	obs_data_set_int(fields, "item-id", obs_sceneitem_get_id(sceneItem));
	instance->broadcastUpdate("SceneItemAdded", fields);
}

/**
 * A scene item has been removed from a scene.
 *
 * @return {String} `scene-name` Name of the scene.
 * @return {String} `item-name` Name of the item removed from the scene.
 * @return {int} `item-id` Scene item ID
 *
 * @api events
 * @name SceneItemRemoved
 * @category scene items
 * @since 4.0.0
 */
void WSEvents::OnSceneItemDelete(void* param, calldata_t* data) {
	auto instance = reinterpret_cast<WSEvents*>(param);

	obs_scene_t* scene = nullptr;
	calldata_get_ptr(data, "scene", &scene);

	obs_sceneitem_t* sceneItem = nullptr;
	calldata_get_ptr(data, "item", &sceneItem);

	const char* sceneName =
		obs_source_get_name(obs_scene_get_source(scene));
	const char* sceneItemName =
		obs_source_get_name(obs_sceneitem_get_source(sceneItem));

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "scene-name", sceneName);
	obs_data_set_string(fields, "item-name", sceneItemName);
	obs_data_set_int(fields, "item-id", obs_sceneitem_get_id(sceneItem));
	instance->broadcastUpdate("SceneItemRemoved", fields);
}

/**
 * A scene item's visibility has been toggled.
 *
 * @return {String} `scene-name` Name of the scene.
 * @return {String} `item-name` Name of the item in the scene.
 * @return {int} `item-id` Scene item ID
 * @return {boolean} `item-visible` New visibility state of the item.
 *
 * @api events
 * @name SceneItemVisibilityChanged
 * @category scene items
 * @since 4.0.0
 */
void WSEvents::OnSceneItemVisibilityChanged(void* param, calldata_t* data) {
	auto instance = reinterpret_cast<WSEvents*>(param);

	obs_scene_t* scene = nullptr;
	calldata_get_ptr(data, "scene", &scene);

	obs_sceneitem_t* sceneItem = nullptr;
	calldata_get_ptr(data, "item", &sceneItem);

	bool visible = false;
	calldata_get_bool(data, "visible", &visible);

	const char* sceneName =
		obs_source_get_name(obs_scene_get_source(scene));
	const char* sceneItemName =
		obs_source_get_name(obs_sceneitem_get_source(sceneItem));

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "scene-name", sceneName);
	obs_data_set_string(fields, "item-name", sceneItemName);
	obs_data_set_int(fields, "item-id", obs_sceneitem_get_id(sceneItem));
	obs_data_set_bool(fields, "item-visible", visible);
	instance->broadcastUpdate("SceneItemVisibilityChanged", fields);
}

/**
 * A scene item's locked status has been toggled.
 *
 * @return {String} `scene-name` Name of the scene.
 * @return {String} `item-name` Name of the item in the scene.
 * @return {int} `item-id` Scene item ID
 * @return {boolean} `item-locked` New locked state of the item.
 *
 * @api events
 * @name SceneItemLockChanged
 * @category scene items
 * @since 4.8.0
 */
void WSEvents::OnSceneItemLockChanged(void* param, calldata_t* data) {
	auto instance = reinterpret_cast<WSEvents*>(param);

	obs_scene_t* scene = nullptr;
	calldata_get_ptr(data, "scene", &scene);

	obs_sceneitem_t* sceneItem = nullptr;
	calldata_get_ptr(data, "item", &sceneItem);

	bool locked = false;
	calldata_get_bool(data, "locked", &locked);

	const char* sceneName =
		obs_source_get_name(obs_scene_get_source(scene));
	const char* sceneItemName =
		obs_source_get_name(obs_sceneitem_get_source(sceneItem));

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "scene-name", sceneName);
	obs_data_set_string(fields, "item-name", sceneItemName);
	obs_data_set_int(fields, "item-id", obs_sceneitem_get_id(sceneItem));
	obs_data_set_bool(fields, "item-locked", locked);
	instance->broadcastUpdate("SceneItemLockChanged", fields);
}

/**
 * A scene item's transform has been changed.
 *
 * @return {String} `scene-name` Name of the scene.
 * @return {String} `item-name` Name of the item in the scene.
 * @return {int} `item-id` Scene item ID
 * @return {SceneItemTransform} `transform` Scene item transform properties
 *
 * @api events
 * @name SceneItemTransformChanged
 * @category scene items
 * @since 4.6.0
 */
void WSEvents::OnSceneItemTransform(void* param, calldata_t* data) {
	auto instance = reinterpret_cast<WSEvents*>(param);

	obs_scene_t* scene = nullptr;
	calldata_get_ptr(data, "scene", &scene);

	obs_sceneitem_t* sceneItem = nullptr;
	calldata_get_ptr(data, "item", &sceneItem);

	const char* sceneName =
		obs_source_get_name(obs_scene_get_source(scene));
	const char* sceneItemName =
		obs_source_get_name(obs_sceneitem_get_source(sceneItem));

	OBSDataAutoRelease transform = Utils::GetSceneItemPropertiesData(sceneItem);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "scene-name", sceneName);
	obs_data_set_string(fields, "item-name", sceneItemName);
	obs_data_set_int(fields, "item-id", obs_sceneitem_get_id(sceneItem));
	obs_data_set_obj(fields, "transform", transform);
	instance->broadcastUpdate("SceneItemTransformChanged", fields);
}

/**
 * A scene item is selected.
 *
 * @return {String} `scene-name` Name of the scene.
 * @return {String} `item-name` Name of the item in the scene.
 * @return {int} `item-id` Name of the item in the scene.
 *
 * @api events
 * @name SceneItemSelected
 * @category scene items
 * @since 4.6.0
 */
void WSEvents::OnSceneItemSelected(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSScene scene = calldata_get_pointer<obs_scene_t>(data, "scene");
	if (!scene) {
		return;
	}

	OBSSceneItem item = calldata_get_pointer<obs_sceneitem_t>(data, "item");
	if (!item) {
		return;
	}

	OBSSource sceneSource = obs_scene_get_source(scene);
	OBSSource itemSource = obs_sceneitem_get_source(item);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "scene-name", obs_source_get_name(sceneSource));
	obs_data_set_string(fields, "item-name", obs_source_get_name(itemSource));
	obs_data_set_int(fields, "item-id", obs_sceneitem_get_id(item));
	self->broadcastUpdate("SceneItemSelected", fields);
}

/**
 * A scene item is deselected.
 *
 * @return {String} `scene-name` Name of the scene.
 * @return {String} `item-name` Name of the item in the scene.
 * @return {int} `item-id` Name of the item in the scene.
 *
 * @api events
 * @name SceneItemDeselected
 * @category scene items
 * @since 4.6.0
 */
void WSEvents::OnSceneItemDeselected(void* param, calldata_t* data) {
	auto self = reinterpret_cast<WSEvents*>(param);

	OBSScene scene = calldata_get_pointer<obs_scene_t>(data, "scene");
	if (!scene) {
		return;
	}

	OBSSceneItem item = calldata_get_pointer<obs_sceneitem_t>(data, "item");
	if (!item) {
		return;
	}

	OBSSource sceneSource = obs_scene_get_source(scene);
	OBSSource itemSource = obs_sceneitem_get_source(item);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "scene-name", obs_source_get_name(sceneSource));
	obs_data_set_string(fields, "item-name", obs_source_get_name(itemSource));
	obs_data_set_int(fields, "item-id", obs_sceneitem_get_id(item));
	self->broadcastUpdate("SceneItemDeselected", fields);
}

/**
 * The selected preview scene has changed (only available in Studio Mode).
 *
 * @return {String} `scene-name` Name of the scene being previewed.
 * @return {Array<SceneItem>} `sources` List of sources composing the scene. Same specification as [`GetCurrentScene`](#getcurrentscene).
 *
 * @api events
 * @name PreviewSceneChanged
 * @category studio mode
 * @since 4.1.0
 */
void WSEvents::OnPreviewSceneChanged() {
	if (obs_frontend_preview_program_mode_active()) {
		OBSSourceAutoRelease scene = obs_frontend_get_current_preview_scene();
		if (!scene)
			return;

		OBSDataArrayAutoRelease sceneItems = Utils::GetSceneItems(scene);

		OBSDataAutoRelease data = obs_data_create();
		obs_data_set_string(data, "scene-name", obs_source_get_name(scene));
		obs_data_set_array(data, "sources", sceneItems);

		broadcastUpdate("PreviewSceneChanged", data);
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
	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_bool(data, "new-state", checked);

	broadcastUpdate("StudioModeSwitched", data);
}

/**
 * A custom broadcast message, sent by the server, requested by one of the websocket clients.
 *
 * @return {String} `realm` Identifier provided by the sender
 * @return {Object} `data` User-defined data
 *
 * @api events
 * @name BroadcastCustomMessage
 * @category general
 * @since 4.7.0
 */
void WSEvents::OnBroadcastCustomMessage(QString realm, obs_data_t* data) {
	OBSDataAutoRelease broadcastData = obs_data_create();
	obs_data_set_string(broadcastData, "realm", realm.toUtf8().constData());
	obs_data_set_obj(broadcastData, "data", data);

	broadcastUpdate("BroadcastCustomMessage", broadcastData);
}

/**
 * @typedef {Object} `OBSStats`
 * @property {double} `fps` Current framerate.
 * @property {int} `render-total-frames` Number of frames rendered
 * @property {int} `render-missed-frames` Number of frames missed due to rendering lag
 * @property {int} `output-total-frames` Number of frames outputted
 * @property {int} `output-skipped-frames` Number of frames skipped due to encoding lag
 * @property {double} `average-frame-time` Average frame render time (in milliseconds)
 * @property {double} `cpu-usage` Current CPU usage (percentage)
 * @property {double} `memory-usage` Current RAM usage (in megabytes)
 * @property {double} `free-disk-space` Free recording disk space (in megabytes)
 */
obs_data_t* WSEvents::GetStats() {
	obs_data_t* stats = obs_data_create();

	double cpuUsage = os_cpu_usage_info_query(cpuUsageInfo);
	double memoryUsage = (double)os_get_proc_resident_size() / (1024.0 * 1024.0);

	video_t* mainVideo = obs_get_video();
	uint32_t outputTotalFrames = video_output_get_total_frames(mainVideo);
	uint32_t outputSkippedFrames = video_output_get_skipped_frames(mainVideo);

	double averageFrameTime = (double)obs_get_average_frame_time_ns() / 1000000.0;

	config_t* currentProfile = obs_frontend_get_profile_config();
	const char* outputMode = config_get_string(currentProfile, "Output", "Mode");
	const char* path = strcmp(outputMode, "Advanced") ?
		config_get_string(currentProfile, "SimpleOutput", "FilePath") :
		config_get_string(currentProfile, "AdvOut", "RecFilePath");

	double freeDiskSpace = (double)os_get_free_disk_space(path) / (1024.0 * 1024.0);

	obs_data_set_double(stats, "fps", obs_get_active_fps());
	obs_data_set_int(stats, "render-total-frames", obs_get_total_frames());
	obs_data_set_int(stats, "render-missed-frames", obs_get_lagged_frames());
	obs_data_set_int(stats, "output-total-frames", outputTotalFrames);
	obs_data_set_int(stats, "output-skipped-frames", outputSkippedFrames);
	obs_data_set_double(stats, "average-frame-time", averageFrameTime);
	obs_data_set_double(stats, "cpu-usage", cpuUsage);
	obs_data_set_double(stats, "memory-usage", memoryUsage);
	obs_data_set_double(stats, "free-disk-space", freeDiskSpace);

	return stats;
}
