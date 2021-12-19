/*
obs-websocket
Copyright (C) 2016-2021 Stephane Lepin <stephane.lepin@gmail.com>
Copyright (C) 2020-2021 Kyle Manning <tt2468@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include "EventHandler.h"

static bool GetOutputStateActive(ObsOutputState state) {
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
	eventData["outputState"] = Utils::Obs::StringHelper::GetOutputState(state);
	BroadcastEvent(EventSubscription::Outputs, "StreamStateChanged", eventData);
}

void EventHandler::HandleRecordStateChanged(ObsOutputState state)
{
	json eventData;
	eventData["outputActive"] = GetOutputStateActive(state);
	eventData["outputState"] = Utils::Obs::StringHelper::GetOutputState(state);
	BroadcastEvent(EventSubscription::Outputs, "RecordStateChanged", eventData);
}

void EventHandler::HandleReplayBufferStateChanged(ObsOutputState state)
{
	json eventData;
	eventData["outputActive"] = GetOutputStateActive(state);
	eventData["outputState"] = Utils::Obs::StringHelper::GetOutputState(state);
	BroadcastEvent(EventSubscription::Outputs, "ReplayBufferStateChanged", eventData);
}

void EventHandler::HandleVirtualcamStateChanged(ObsOutputState state)
{
	json eventData;
	eventData["outputActive"] = GetOutputStateActive(state);
	eventData["outputState"] = Utils::Obs::StringHelper::GetOutputState(state);
	BroadcastEvent(EventSubscription::Outputs, "VirtualcamStateChanged", eventData);
}

void EventHandler::HandleReplayBufferSaved()
{
	json eventData;
	eventData["savedReplayPath"] = Utils::Obs::StringHelper::GetLastReplayBufferFilePath();
	BroadcastEvent(EventSubscription::Outputs, "ReplayBufferSaved", eventData);
}
