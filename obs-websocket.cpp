#include <obs-module.h>
#include <obs-frontend-api.h>
#include "WSServer.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-websocket", "en-US")

WSServer *server;

void obs_frontend_callback(enum obs_frontend_event event, void *) 
{
	bool sendMessage = false;
	obs_data_t *announce = obs_data_create();

	if (event == OBS_FRONTEND_EVENT_SCENE_CHANGED) {
		obs_source_t *source = obs_frontend_get_current_scene();
		const char *name = obs_source_get_name(source);

		obs_data_set_string(announce, "type", "scene_changed");
		obs_data_set_string(announce, "name", name);
		sendMessage = true;
	}
	else if (event == OBS_FRONTEND_EVENT_STREAMING_STARTED) {
		obs_data_set_string(announce, "type", "streaming_started");
		sendMessage = true;
	}
	else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPED) {
		obs_data_set_string(announce, "type", "streaming_stopped");
		sendMessage = true;
	}
	else if (event == OBS_FRONTEND_EVENT_RECORDING_STARTED) {
		obs_data_set_string(announce, "type", "recording_started");
		sendMessage = true;
	}
	else if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPED) {
		obs_data_set_string(announce, "type", "recording_stopped");
		sendMessage = true;
	}
	else if (event == OBS_FRONTEND_EVENT_EXIT) {
		obs_data_set_string(announce, "type", "exiting");
		sendMessage = true;
	}

	if (sendMessage && server) {
		const char *message = obs_data_get_json(announce);
		server->broadcast(message);
	}

	obs_data_release(announce);
}

bool obs_module_load(void) 
{
	blog(LOG_INFO, "[obs-websockets] you can haz websockets");
	
	server = new WSServer(8080);
	obs_frontend_add_event_callback(obs_frontend_callback, nullptr);

	return true;
}

void obs_module_unload()
{
	
}

