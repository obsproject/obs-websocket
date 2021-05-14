#pragma once

#include <obs.hpp>
#include <obs-frontend-api.h>
#include <util/platform.h>

#include "../obs-websocket.h"
#include "../WebSocketServer.h"
#include "types/EventSubscriptions.h"

template <typename T> T* GetCalldataPointer(const calldata_t *data, const char* name) {
	void* ptr = nullptr;
	calldata_get_ptr(data, name, &ptr);
	return reinterpret_cast<T*>(ptr);
}

std::string GetCalldataString(const calldata_t *data, const char* name);

class EventHandler
{
	public:
		EventHandler(WebSocketServerPtr webSocketServer);
		~EventHandler();

	private:
		WebSocketServerPtr _webSocketServer;
		os_cpu_usage_info_t *_cpuUsageInfo;

		void ConnectSourceSignals(obs_source_t *source);
		void DisconnectSourceSignals(obs_source_t *source);

		// Signal handler: frontend
		static void OnFrontendEvent(enum obs_frontend_event event, void *private_data);

		// Signal handler: libobs
		static void SourceCreatedMultiHandler(void *param, calldata_t *data);
		static void SourceRemovedMultiHandler(void *param, calldata_t *data);

		// Signal handler: source
		static void SourceRenamedMultiHandler(void *param, calldata_t *data);

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
		void HandleSceneListReindexed();
};
