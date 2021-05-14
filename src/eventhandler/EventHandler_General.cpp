#include "EventHandler.h"

#include "../plugin-macros.generated.h"

void EventHandler::HandleExitStarted()
{
	_webSocketServer->BroadcastEvent(EventSubscriptions::General, "ExitStarted");
}

void EventHandler::HandleStudioModeStateChanged(bool enabled)
{
	json eventData;
	eventData["studioModeEnabled"] = enabled;
	_webSocketServer->BroadcastEvent(EventSubscriptions::General, "StudioModeStateChanged", eventData);
}
