#include "EventHandler.h"

EventHandler::EventHandler(WebSocketServerPtr webSocketServer) :
	_webSocketServer(webSocketServer)
{
	_cpuUsageInfo = os_cpu_usage_info_start();

	obs_frontend_add_event_callback(EventHandler::OnFrontendEvent, this);
}

EventHandler::~EventHandler()
{
	os_cpu_usage_info_destroy(_cpuUsageInfo);

	obs_frontend_remove_event_callback(EventHandler::OnFrontendEvent, this);
}

void EventHandler::OnFrontendEvent(enum obs_frontend_event event, void *private_data) {
	auto owner = reinterpret_cast<EventHandler*>(private_data);
}
