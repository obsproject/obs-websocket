/*
obs-websocket
Copyright (C) 2016-2021 Stephane Lepin <stephane.lepin@gmail.com>
Copyright (C) 2020-2021 Kyle Manning <tt2468@gmail.com>

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

#include "EventHandler.h"

EventHandler::EventHandler() :
	_obsLoaded(false),
	_inputVolumeMetersRef(0),
	_inputActiveStateChangedRef(0),
	_inputShowStateChangedRef(0),
	_sceneItemTransformChangedRef(0)
{
	blog_debug("[EventHandler::EventHandler] Setting up...");

	obs_frontend_add_event_callback(OnFrontendEvent, this);

	signal_handler_t* coreSignalHandler = obs_get_signal_handler();
	if (coreSignalHandler) {
		signal_handler_connect(coreSignalHandler, "source_create", SourceCreatedMultiHandler, this);
		signal_handler_connect(coreSignalHandler, "source_destroy", SourceDestroyedMultiHandler, this);
		signal_handler_connect(coreSignalHandler, "source_remove", SourceRemovedMultiHandler, this);
		signal_handler_connect(coreSignalHandler, "source_rename", SourceRenamedMultiHandler, this);
	} else {
		blog(LOG_ERROR, "[EventHandler::EventHandler] Unable to get libobs signal handler!");
	}

	blog_debug("[EventHandler::EventHandler] Finished.");
}

EventHandler::~EventHandler()
{
	blog_debug("[EventHandler::~EventHandler] Shutting down...");

	obs_frontend_remove_event_callback(OnFrontendEvent, this);

	signal_handler_t* coreSignalHandler = obs_get_signal_handler();
	if (coreSignalHandler) {
		signal_handler_disconnect(coreSignalHandler, "source_create", SourceCreatedMultiHandler, this);
		signal_handler_disconnect(coreSignalHandler, "source_destroy", SourceDestroyedMultiHandler, this);
		signal_handler_disconnect(coreSignalHandler, "source_remove", SourceRemovedMultiHandler, this);
		signal_handler_disconnect(coreSignalHandler, "source_rename", SourceRenamedMultiHandler, this);
	} else {
		blog(LOG_ERROR, "[EventHandler::~EventHandler] Unable to get libobs signal handler!");
	}

	blog_debug("[EventHandler::~EventHandler] Finished.");
}

void EventHandler::SetBroadcastCallback(EventHandler::BroadcastCallback cb)
{
	_broadcastCallback = cb;
}

void EventHandler::SetObsLoadedCallback(EventHandler::ObsLoadedCallback cb)
{
	_obsLoadedCallback = cb;
}

// Function to increment refcounts for high volume event subscriptions
void EventHandler::ProcessSubscription(uint64_t eventSubscriptions)
{
	if ((eventSubscriptions & EventSubscription::InputVolumeMeters) != 0) {
		if (_inputVolumeMetersRef.fetch_add(1) == 0) {
			if (_inputVolumeMetersHandler)
				blog(LOG_WARNING, "[EventHandler::ProcessSubscription] Input volume meter handler already exists!");
			else
				_inputVolumeMetersHandler = std::make_unique<Utils::Obs::VolumeMeter::Handler>(std::bind(&EventHandler::HandleInputVolumeMeters, this, std::placeholders::_1));
		}
	}
	if ((eventSubscriptions & EventSubscription::InputActiveStateChanged) != 0)
		_inputActiveStateChangedRef++;
	if ((eventSubscriptions & EventSubscription::InputShowStateChanged) != 0)
		_inputShowStateChangedRef++;
	if ((eventSubscriptions & EventSubscription::SceneItemTransformChanged) != 0)
		_sceneItemTransformChangedRef++;
}

// Function to decrement refcounts for high volume event subscriptions
void EventHandler::ProcessUnsubscription(uint64_t eventSubscriptions)
{
	if ((eventSubscriptions & EventSubscription::InputVolumeMeters) != 0) {
		if (_inputVolumeMetersRef.fetch_sub(1) == 1)
			_inputVolumeMetersHandler.reset();
	}
	if ((eventSubscriptions & EventSubscription::InputActiveStateChanged) != 0)
		_inputActiveStateChangedRef--;
	if ((eventSubscriptions & EventSubscription::InputShowStateChanged) != 0)
		_inputShowStateChangedRef--;
	if ((eventSubscriptions & EventSubscription::SceneItemTransformChanged) != 0)
		_sceneItemTransformChangedRef--;
}

// Function required in order to use default arguments
void EventHandler::BroadcastEvent(uint64_t requiredIntent, std::string eventType, json eventData, uint8_t rpcVersion)
{
	if (!_broadcastCallback)
		return;

	_broadcastCallback(requiredIntent, eventType, eventData, rpcVersion);
}

void EventHandler::ConnectSourceSignals(obs_source_t *source) // Applies to inputs and scenes
{
	if (!source || obs_source_removed(source))
		return;

	// Disconnect all existing signals from the source to prevent multiple connections
	DisconnectSourceSignals(source);

	signal_handler_t* sh = obs_source_get_signal_handler(source);

	obs_source_type sourceType = obs_source_get_type(source);

	// Inputs
	signal_handler_connect(sh, "activate", HandleInputActiveStateChanged, this);
	signal_handler_connect(sh, "deactivate", HandleInputActiveStateChanged, this);
	signal_handler_connect(sh, "show", HandleInputShowStateChanged, this);
	signal_handler_connect(sh, "hide", HandleInputShowStateChanged, this);
	signal_handler_connect(sh, "mute", HandleInputMuteStateChanged, this);
	signal_handler_connect(sh, "volume", HandleInputVolumeChanged, this);
	signal_handler_connect(sh, "audio_balance", HandleInputAudioBalanceChanged, this);
	signal_handler_connect(sh, "audio_sync", HandleInputAudioSyncOffsetChanged, this);
	signal_handler_connect(sh, "audio_mixers", HandleInputAudioTracksChanged, this);
	signal_handler_connect(sh, "audio_monitoring", HandleInputAudioMonitorTypeChanged, this);

	if (sourceType == OBS_SOURCE_TYPE_INPUT) {
		signal_handler_connect(sh, "media_started", HandleMediaInputPlaybackStarted, this);
		signal_handler_connect(sh, "media_ended", HandleMediaInputPlaybackEnded, this);
		signal_handler_connect(sh, "media_pause", SourceMediaPauseMultiHandler, this);
		signal_handler_connect(sh, "media_play", SourceMediaPlayMultiHandler, this);
		signal_handler_connect(sh, "media_restart", SourceMediaRestartMultiHandler, this);
		signal_handler_connect(sh, "media_stopped", SourceMediaStopMultiHandler, this);
		signal_handler_connect(sh, "media_next", SourceMediaNextMultiHandler, this);
		signal_handler_connect(sh, "media_previous", SourceMediaPreviousMultiHandler, this);
	}

	// Scenes
	if (sourceType == OBS_SOURCE_TYPE_SCENE) {
		signal_handler_connect(sh, "item_add", HandleSceneItemCreated, this);
		signal_handler_connect(sh, "item_remove", HandleSceneItemRemoved, this);
		signal_handler_connect(sh, "reorder", HandleSceneItemListReindexed, this);
		signal_handler_connect(sh, "item_visible", HandleSceneItemEnableStateChanged, this);
		signal_handler_connect(sh, "item_locked", HandleSceneItemLockStateChanged, this);
		signal_handler_connect(sh, "item_select", HandleSceneItemSelected, this);
		signal_handler_connect(sh, "item_transform", HandleSceneItemTransformChanged, this);
	}
}

void EventHandler::DisconnectSourceSignals(obs_source_t *source)
{
	if (!source)
		return;

	signal_handler_t* sh = obs_source_get_signal_handler(source);

	// Inputs
	signal_handler_disconnect(sh, "activate", HandleInputActiveStateChanged, this);
	signal_handler_disconnect(sh, "deactivate", HandleInputActiveStateChanged, this);
	signal_handler_disconnect(sh, "show", HandleInputShowStateChanged, this);
	signal_handler_disconnect(sh, "hide", HandleInputShowStateChanged, this);
	signal_handler_disconnect(sh, "mute", HandleInputMuteStateChanged, this);
	signal_handler_disconnect(sh, "volume", HandleInputVolumeChanged, this);
	signal_handler_disconnect(sh, "audio_balance", HandleInputAudioBalanceChanged, this);
	signal_handler_disconnect(sh, "audio_sync", HandleInputAudioSyncOffsetChanged, this);
	signal_handler_disconnect(sh, "audio_mixers", HandleInputAudioTracksChanged, this);
	signal_handler_disconnect(sh, "audio_monitoring", HandleInputAudioMonitorTypeChanged, this);
	signal_handler_disconnect(sh, "media_started", HandleMediaInputPlaybackStarted, this);
	signal_handler_disconnect(sh, "media_ended", HandleMediaInputPlaybackEnded, this);
	signal_handler_disconnect(sh, "media_pause", SourceMediaPauseMultiHandler, this);
	signal_handler_disconnect(sh, "media_play", SourceMediaPlayMultiHandler, this);
	signal_handler_disconnect(sh, "media_restart", SourceMediaRestartMultiHandler, this);
	signal_handler_disconnect(sh, "media_stopped", SourceMediaStopMultiHandler, this);
	signal_handler_disconnect(sh, "media_next", SourceMediaNextMultiHandler, this);
	signal_handler_disconnect(sh, "media_previous", SourceMediaPreviousMultiHandler, this);

	// Scenes
	signal_handler_disconnect(sh, "item_add", HandleSceneItemCreated, this);
	signal_handler_disconnect(sh, "item_remove", HandleSceneItemRemoved, this);
	signal_handler_disconnect(sh, "reorder", HandleSceneItemListReindexed, this);
	signal_handler_disconnect(sh, "item_visible", HandleSceneItemEnableStateChanged, this);
	signal_handler_disconnect(sh, "item_locked", HandleSceneItemLockStateChanged, this);
	signal_handler_disconnect(sh, "item_select", HandleSceneItemSelected, this);
	signal_handler_disconnect(sh, "item_transform", HandleSceneItemTransformChanged, this);
}

void EventHandler::OnFrontendEvent(enum obs_frontend_event event, void *private_data)
{
	auto eventHandler = static_cast<EventHandler*>(private_data);

	if (!eventHandler->_obsLoaded.load()) {
		if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
			blog_debug("[EventHandler::OnFrontendEvent] OBS has finished loading. Connecting final handlers and enabling events...");
			// Connect source signals and enable events only after OBS has fully loaded (to reduce extra logging).
			eventHandler->_obsLoaded.store(true);

			// In the case that plugins become hotloadable, this will have to go back into `EventHandler::EventHandler()`
			// Enumerate inputs and connect each one
			obs_enum_sources([](void* param, obs_source_t* source) {
				auto eventHandler = static_cast<EventHandler*>(param);
				eventHandler->ConnectSourceSignals(source);
				return true;
			}, private_data);

			// Enumerate scenes and connect each one
			obs_enum_scenes([](void* param, obs_source_t* source) {
				auto eventHandler = static_cast<EventHandler*>(param);
				eventHandler->ConnectSourceSignals(source);
				return true;
			}, private_data);

			blog_debug("[EventHandler::OnFrontendEvent] Finished.");

			if (eventHandler->_obsLoadedCallback)
				eventHandler->_obsLoadedCallback();
		} else {
			return;
		}
	}

	switch (event) {
		// General
		case OBS_FRONTEND_EVENT_EXIT:
			eventHandler->HandleExitStarted();

			blog_debug("[EventHandler::OnFrontendEvent] OBS is unloading. Disabling events...");
			// Disconnect source signals and disable events when OBS starts unloading (to reduce extra logging).
			eventHandler->_obsLoaded.store(false);

			// In the case that plugins become hotloadable, this will have to go back into `EventHandler::~EventHandler()`
			// Enumerate inputs and disconnect each one
			obs_enum_sources([](void* param, obs_source_t* source) {
				auto eventHandler = static_cast<EventHandler*>(param);
				eventHandler->DisconnectSourceSignals(source);
				return true;
			}, private_data);

			// Enumerate scenes and disconnect each one
			obs_enum_scenes([](void* param, obs_source_t* source) {
				auto eventHandler = static_cast<EventHandler*>(param);
				eventHandler->DisconnectSourceSignals(source);
				return true;
			}, private_data);

			blog_debug("[EventHandler::OnFrontendEvent] Finished.");

			break;
		case OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED:
			eventHandler->HandleStudioModeStateChanged(true);
			break;
		case OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED:
			eventHandler->HandleStudioModeStateChanged(false);
			break;

		// Config
		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGING:
			eventHandler->HandleCurrentSceneCollectionChanging();
			break;
		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED:
			eventHandler->HandleCurrentSceneCollectionChanged();
			break;
		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_LIST_CHANGED:
			eventHandler->HandleSceneCollectionListChanged();
			break;
		case OBS_FRONTEND_EVENT_PROFILE_CHANGING:
			eventHandler->HandleCurrentProfileChanging();
			break;
		case OBS_FRONTEND_EVENT_PROFILE_CHANGED:
			eventHandler->HandleCurrentProfileChanged();
			break;
		case OBS_FRONTEND_EVENT_PROFILE_LIST_CHANGED:
			eventHandler->HandleProfileListChanged();
			break;

		// Scenes
		case OBS_FRONTEND_EVENT_SCENE_CHANGED:
			eventHandler->HandleCurrentProgramSceneChanged();
			break;
		case OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED:
			eventHandler->HandleCurrentPreviewSceneChanged();
			break;
		case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED:
			eventHandler->HandleSceneListChanged();
			break;

		// Transitions
		case OBS_FRONTEND_EVENT_TRANSITION_CHANGED:
			eventHandler->HandleCurrentSceneTransitionChanged();
			break;
		case OBS_FRONTEND_EVENT_TRANSITION_LIST_CHANGED:
			break;
		case OBS_FRONTEND_EVENT_TRANSITION_DURATION_CHANGED:
			eventHandler->HandleCurrentSceneTransitionDurationChanged();
			break;

		// Outputs
		case OBS_FRONTEND_EVENT_STREAMING_STARTING:
			eventHandler->HandleStreamStateChanged(OBS_WEBSOCKET_OUTPUT_STARTING);
			break;
		case OBS_FRONTEND_EVENT_STREAMING_STARTED:
			eventHandler->HandleStreamStateChanged(OBS_WEBSOCKET_OUTPUT_STARTED);
			break;
		case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
			eventHandler->HandleStreamStateChanged(OBS_WEBSOCKET_OUTPUT_STOPPING);
			break;
		case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
			eventHandler->HandleStreamStateChanged(OBS_WEBSOCKET_OUTPUT_STOPPED);
			break;
		case OBS_FRONTEND_EVENT_RECORDING_STARTING:
			eventHandler->HandleRecordStateChanged(OBS_WEBSOCKET_OUTPUT_STARTING);
			break;
		case OBS_FRONTEND_EVENT_RECORDING_STARTED:
			eventHandler->HandleRecordStateChanged(OBS_WEBSOCKET_OUTPUT_STARTED);
			break;
		case OBS_FRONTEND_EVENT_RECORDING_STOPPING:
			eventHandler->HandleRecordStateChanged(OBS_WEBSOCKET_OUTPUT_STOPPING);
			break;
		case OBS_FRONTEND_EVENT_RECORDING_STOPPED:
			eventHandler->HandleRecordStateChanged(OBS_WEBSOCKET_OUTPUT_STOPPED);
			break;
		case OBS_FRONTEND_EVENT_RECORDING_PAUSED:
			eventHandler->HandleRecordStateChanged(OBS_WEBSOCKET_OUTPUT_PAUSED);
			break;
		case OBS_FRONTEND_EVENT_RECORDING_UNPAUSED:
			eventHandler->HandleRecordStateChanged(OBS_WEBSOCKET_OUTPUT_RESUMED);
			break;
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTING:
			eventHandler->HandleReplayBufferStateChanged(OBS_WEBSOCKET_OUTPUT_STARTING);
			break;
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED:
			eventHandler->HandleReplayBufferStateChanged(OBS_WEBSOCKET_OUTPUT_STARTED);
			break;
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPING:
			eventHandler->HandleReplayBufferStateChanged(OBS_WEBSOCKET_OUTPUT_STOPPING);
			break;
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED:
			eventHandler->HandleReplayBufferStateChanged(OBS_WEBSOCKET_OUTPUT_STOPPED);
			break;
		case OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED:
			eventHandler->HandleVirtualcamStateChanged(OBS_WEBSOCKET_OUTPUT_STARTED);
			break;
		case OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED:
			eventHandler->HandleVirtualcamStateChanged(OBS_WEBSOCKET_OUTPUT_STOPPED);
			break;
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_SAVED:
			eventHandler->HandleReplayBufferSaved();
			break;

		default:
			break;
	}
}

// Only called for creation of a public source
void EventHandler::SourceCreatedMultiHandler(void *param, calldata_t *data)
{
	auto eventHandler = static_cast<EventHandler*>(param);

	// Don't react to signals until OBS has finished loading
	if (!eventHandler->_obsLoaded.load())
		return;

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	eventHandler->ConnectSourceSignals(source);

	switch (obs_source_get_type(source)) {
		case OBS_SOURCE_TYPE_INPUT:
			eventHandler->HandleInputCreated(source);
			break;
		case OBS_SOURCE_TYPE_SCENE:
			eventHandler->HandleSceneCreated(source);
			break;
		default:
			break;
	}
}

// Only called for destruction of a public source
void EventHandler::SourceDestroyedMultiHandler(void *param, calldata_t *data)
{
	auto eventHandler = static_cast<EventHandler*>(param);

	// We can't use any smart types here because releasing the source will cause infinite recursion
	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	// Disconnect all signals from the source
	eventHandler->DisconnectSourceSignals(source);

	// Don't react to signals if OBS is unloading
	if (!eventHandler->_obsLoaded.load())
		return;

	switch (obs_source_get_type(source)) {
		case OBS_SOURCE_TYPE_INPUT:
			// We have to call `InputRemoved` with source_destroy because source_removed is not called when an input's last scene item is removed
			eventHandler->HandleInputRemoved(source);
			break;
		case OBS_SOURCE_TYPE_SCENE:
			break;
		default:
			break;
	}
}

void EventHandler::SourceRemovedMultiHandler(void *param, calldata_t *data)
{
	auto eventHandler = static_cast<EventHandler*>(param);

	if (!eventHandler->_obsLoaded.load())
		return;

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	switch (obs_source_get_type(source)) {
		case OBS_SOURCE_TYPE_INPUT:
			break;
		case OBS_SOURCE_TYPE_SCENE:
			// Scenes emit the `removed` signal when they are removed from OBS, as expected
			eventHandler->HandleSceneRemoved(source);
			break;
		default:
			break;
	}
}

void EventHandler::SourceRenamedMultiHandler(void *param, calldata_t *data)
{
	auto eventHandler = static_cast<EventHandler*>(param);

	if (!eventHandler->_obsLoaded.load())
		return;

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	std::string oldSourceName = calldata_string(data, "prev_name");
	std::string sourceName = calldata_string(data, "new_name");
	if (oldSourceName.empty() || sourceName.empty())
		return;

	switch (obs_source_get_type(source)) {
		case OBS_SOURCE_TYPE_INPUT:
			eventHandler->HandleInputNameChanged(source, oldSourceName, sourceName);
			break;
		case OBS_SOURCE_TYPE_FILTER:
			break;
		case OBS_SOURCE_TYPE_TRANSITION:
			break;
		case OBS_SOURCE_TYPE_SCENE:
			eventHandler->HandleSceneNameChanged(source, oldSourceName, sourceName);
			break;
		default:
			break;
	}
}
