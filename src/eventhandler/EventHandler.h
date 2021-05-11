#pragma once

#include <obs.hpp>
#include <obs-frontend-api.h>
#include <util/platform.h>

#include "../obs-websocket.h"
#include "../WebSocketServer.h"

class EventHandler
{
	public:
		EventHandler(WebSocketServerPtr webSocketServer);
		~EventHandler();

		template <typename T> T* GetCalldataPointer(const calldata_t *data, const char* name) {
			void* ptr = nullptr;
			calldata_get_ptr(data, name, &ptr);
			return reinterpret_cast<T*>(ptr);
		}

		static std::string GetCalldataString(const calldata_t *data, const char* name);

	private:
		WebSocketServerPtr _webSocketServer;
		os_cpu_usage_info_t *_cpuUsageInfo;

		static void OnFrontendEvent(enum obs_frontend_event event, void *private_data);
};
