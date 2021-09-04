#include "EventHandler.h"

void EventHandler::HandleExitStarted()
{
	BroadcastEvent(EventSubscription::General, "ExitStarted");
}

void EventHandler::HandleStudioModeStateChanged(bool enabled)
{
	json eventData;
	eventData["studioModeEnabled"] = enabled;
	BroadcastEvent(EventSubscription::General, "StudioModeStateChanged", eventData);
}
