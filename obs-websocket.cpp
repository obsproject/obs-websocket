#include <obs-module.h>
#include <obs-frontend-api.h>
#include "WSEvents.h"
#include "WSServer.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-websocket", "en-US")

WSEvents *eventHandler;
WSServer *server;

bool obs_module_load(void) 
{
	blog(LOG_INFO, "[obs-websockets] you can haz websockets");
	
	server = new WSServer(8080);
	eventHandler = new WSEvents(server);
	
	return true;
}

void obs_module_unload()
{
	blog(LOG_INFO, "[obs-websockets] goodbye !");
}

