#include "EventHandler.h"

#define CASE(x) case x: return #x;

std::string GetOutputStateString(ObsOutputState state) {
	switch (state) {
		default:
		CASE(OBS_WEBSOCKET_OUTPUT_STARTING)
		CASE(OBS_WEBSOCKET_OUTPUT_STARTED)
		CASE(OBS_WEBSOCKET_OUTPUT_STOPPING)
		CASE(OBS_WEBSOCKET_OUTPUT_STOPPED)
		CASE(OBS_WEBSOCKET_OUTPUT_PAUSED)
		CASE(OBS_WEBSOCKET_OUTPUT_RESUMED)
	}
}

bool GetOutputStateActive(ObsOutputState state) {
	switch(state) {
		case OBS_WEBSOCKET_OUTPUT_STARTED:
		case OBS_WEBSOCKET_OUTPUT_RESUMED:
			return true;
		case OBS_WEBSOCKET_OUTPUT_STARTING:
		case OBS_WEBSOCKET_OUTPUT_STOPPING:
		case OBS_WEBSOCKET_OUTPUT_STOPPED:
		case OBS_WEBSOCKET_OUTPUT_PAUSED:
			return false;
		default:
			return false;
	}
}

void EventHandler::HandleStreamStateChanged(ObsOutputState state)
{
	json eventData;
	eventData["outputActive"] = GetOutputStateActive(state);
	eventData["outputState"] = GetOutputStateString(state);
	BroadcastEvent(EventSubscription::Outputs, "StreamStateChanged", eventData);
}

void EventHandler::HandleRecordStateChanged(ObsOutputState state)
{
	json eventData;
	eventData["outputActive"] = GetOutputStateActive(state);
	eventData["outputState"] = GetOutputStateString(state);
	BroadcastEvent(EventSubscription::Outputs, "RecordStateChanged", eventData);
}

void EventHandler::HandleReplayBufferStateChanged(ObsOutputState state)
{
	json eventData;
	eventData["outputActive"] = GetOutputStateActive(state);
	eventData["outputState"] = GetOutputStateString(state);
	BroadcastEvent(EventSubscription::Outputs, "ReplayBufferStateChanged", eventData);
}

void EventHandler::HandleVirtualcamStateChanged(ObsOutputState state)
{
	json eventData;
	eventData["outputActive"] = GetOutputStateActive(state);
	eventData["outputState"] = GetOutputStateString(state);
	BroadcastEvent(EventSubscription::Outputs, "VirtualcamStateChanged", eventData);
}

void EventHandler::HandleReplayBufferSaved()
{
	json eventData;
	eventData["savedReplayPath"] = Utils::Obs::StringHelper::GetLastReplayBufferFilePath();
	BroadcastEvent(EventSubscription::Outputs, "ReplayBufferSaved", eventData);
}
