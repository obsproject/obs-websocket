#pragma once

#include <obs.hpp>
#include <obs-frontend-api.h>
#include <util/platform.h>

#include "../obs-websocket.h"
#include "../WebSocketServer.h"
#include "types/EventSubscription.h"

enum ObsOutputState {
	OBS_WEBSOCKET_OUTPUT_STARTING,
	OBS_WEBSOCKET_OUTPUT_STARTED,
	OBS_WEBSOCKET_OUTPUT_STOPPING,
	OBS_WEBSOCKET_OUTPUT_STOPPED,
	OBS_WEBSOCKET_OUTPUT_PAUSED,
	OBS_WEBSOCKET_OUTPUT_RESUMED
};

enum ObsMediaInputAction {
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PAUSE,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PLAY,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_RESTART,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_STOP,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_NEXT,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PREVIOUS
};

template <typename T> T* GetCalldataPointer(const calldata_t *data, const char* name) {
	void *ptr = nullptr;
	calldata_get_ptr(data, name, &ptr);
	return reinterpret_cast<T*>(ptr);
}

class EventHandler
{
	public:
		EventHandler(WebSocketServerPtr webSocketServer);
		~EventHandler();

	private:
		WebSocketServerPtr _webSocketServer;

		std::atomic<bool> _obsLoaded;

		void ConnectSourceSignals(obs_source_t *source);
		void DisconnectSourceSignals(obs_source_t *source);

		// Signal handler: frontend
		static void OnFrontendEvent(enum obs_frontend_event event, void *private_data);

		// Signal handler: libobs
		static void SourceCreatedMultiHandler(void *param, calldata_t *data);
		static void SourceDestroyedMultiHandler(void *param, calldata_t *data);
		static void SourceRemovedMultiHandler(void *param, calldata_t *data);

		// Signal handler: source
		static void SourceRenamedMultiHandler(void *param, calldata_t *data);
		static void SourceMediaPauseMultiHandler(void *param, calldata_t *data);
		static void SourceMediaPlayMultiHandler(void *param, calldata_t *data);
		static void SourceMediaRestartMultiHandler(void *param, calldata_t *data);
		static void SourceMediaStopMultiHandler(void *param, calldata_t *data);
		static void SourceMediaNextMultiHandler(void *param, calldata_t *data);
		static void SourceMediaPreviousMultiHandler(void *param, calldata_t *data);


		// General
		void HandleExitStarted();
		void HandleStudioModeStateChanged(bool enabled);

		// Config
		void HandleCurrentSceneCollectionChanged();
		void HandleSceneCollectionListChanged();
		void HandleCurrentProfileChanged();
		void HandleProfileListChanged();

		// Scenes
		void HandleSceneCreated(obs_source_t *source);
		void HandleSceneRemoved(obs_source_t *source);
		void HandleSceneNameChanged(obs_source_t *source, std::string oldSceneName, std::string sceneName);
		void HandleCurrentSceneChanged();
		void HandleCurrentPreviewSceneChanged();
		void HandleSceneListChanged();

		// Inputs
		void HandleInputCreated(obs_source_t *source);
		void HandleInputRemoved(obs_source_t *source);
		void HandleInputNameChanged(obs_source_t *source, std::string oldInputName, std::string inputName);
		static void HandleInputActiveStateChanged(void *param, calldata_t *data); // Direct callback
		static void HandleInputShowStateChanged(void *param, calldata_t *data); // Direct callback
		static void HandleInputMuteStateChanged(void *param, calldata_t *data); // Direct callback
		static void HandleInputVolumeChanged(void *param, calldata_t *data); // Direct callback
		static void HandleInputAudioSyncOffsetChanged(void *param, calldata_t *data); // Direct callback
		static void HandleInputAudioTracksChanged(void *param, calldata_t *data); // Direct callback

		// Transitions
		void HandleTransitionCreated(obs_source_t *source);
		void HandleTransitionRemoved(obs_source_t *source);
		void HandleTransitionNameChanged(obs_source_t *source, std::string oldTransitionName, std::string transitionName);

		// Outputs
		void HandleStreamStateChanged(ObsOutputState state);
		void HandleRecordStateChanged(ObsOutputState state);
		void HandleReplayBufferStateChanged(ObsOutputState state);
		void HandleVirtualcamStateChanged(ObsOutputState state);
		void HandleReplayBufferSaved();

		// Scene Items
		static void HandleSceneItemCreated(void *param, calldata_t *data); // Direct callback
		static void HandleSceneItemRemoved(void *param, calldata_t *data); // Direct callback
		static void HandleSceneItemListReindexed(void *param, calldata_t *data); // Direct callback
		static void HandleSceneItemEnableStateChanged(void *param, calldata_t *data); // Direct callback
		static void HandleSceneItemLockStateChanged(void *param, calldata_t *data); // Direct callback
		static void HandleSceneItemTransformChanged(void *param, calldata_t *data); // Direct callback

		// Media Inputs
		static void HandleMediaInputPlaybackStarted(void *param, calldata_t *data); // Direct callback
		static void HandleMediaInputPlaybackEnded(void *param, calldata_t *data); // Direct callback
		void HandleMediaInputActionTriggered(obs_source_t *source, ObsMediaInputAction action);
};
