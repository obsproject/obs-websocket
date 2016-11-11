#include <obs-module.h>
#include <obs-frontend-api.h>

#include "obs-websocket.h"
#include "WSEvents.h"
#include "WSServer.h"
#include "Config.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-websocket", "en-US")

WSEvents *eventHandler;
WSServer *server;

bool obs_module_load(void) 
{
	blog(LOG_INFO, "[obs-websockets] you can haz websockets (version %.2f)", OBS_WEBSOCKET_VERSION);
	
	server = new WSServer(4444);
	eventHandler = new WSEvents(server);
	
	obs_frontend_add_save_callback(Config::OBSSaveCallback, Config::Current());

	return true;
}

void obs_module_unload()
{
	blog(LOG_INFO, "[obs-websockets] goodbye !");
}

