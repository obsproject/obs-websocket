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

	private:
		WebSocketServerPtr _webSocketServer;
		os_cpu_usage_info_t *_cpuUsageInfo;

		static void OnFrontendEvent(enum obs_frontend_event event, void *private_data);
};
