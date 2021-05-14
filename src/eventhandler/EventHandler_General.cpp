#include "EventHandler.h"

#include "../plugin-macros.generated.h"

void EventHandler::HandleExitStarted()
{
	_webSocketServer->BroadcastEvent(EventSubscription::General, "ExitStarted");
}

void EventHandler::HandleStudioModeStateChanged(bool enabled)
{
	json eventData;
	eventData["studioModeEnabled"] = enabled;
	_webSocketServer->BroadcastEvent(EventSubscription::General, "StudioModeStateChanged", eventData);
}
