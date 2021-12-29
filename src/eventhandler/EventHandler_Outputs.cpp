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

/**
 * The state of the stream output has changed.
 *
 * @dataField outputActive | Boolean | Whether the output is active
 * @dataField outputState  | String  | The specific state of the output
 *
 * @eventType StreamStateChanged
 * @eventSubscription Outputs
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api events
 * @category outputs
 */
void EventHandler::HandleStreamStateChanged(ObsOutputState state)
{
	json eventData;
	eventData["outputActive"] = GetOutputStateActive(state);
	eventData["outputState"] = Utils::Obs::StringHelper::GetOutputState(state);
	BroadcastEvent(EventSubscription::Outputs, "StreamStateChanged", eventData);
}

/**
 * The state of the record output has changed.
 *
 * @dataField outputActive | Boolean | Whether the output is active
 * @dataField outputState  | String  | The specific state of the output
 *
 * @eventType RecordStateChanged
 * @eventSubscription Outputs
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api events
 * @category outputs
 */
void EventHandler::HandleRecordStateChanged(ObsOutputState state)
{
	json eventData;
	eventData["outputActive"] = GetOutputStateActive(state);
	eventData["outputState"] = Utils::Obs::StringHelper::GetOutputState(state);
	BroadcastEvent(EventSubscription::Outputs, "RecordStateChanged", eventData);
}

/**
 * The state of the replay buffer output has changed.
 *
 * @dataField outputActive | Boolean | Whether the output is active
 * @dataField outputState  | String  | The specific state of the output
 *
 * @eventType ReplayBufferStateChanged
 * @eventSubscription Outputs
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api events
 * @category outputs
 */
void EventHandler::HandleReplayBufferStateChanged(ObsOutputState state)
{
	json eventData;
	eventData["outputActive"] = GetOutputStateActive(state);
	eventData["outputState"] = Utils::Obs::StringHelper::GetOutputState(state);
	BroadcastEvent(EventSubscription::Outputs, "ReplayBufferStateChanged", eventData);
}

/**
 * The state of the virtualcam output has changed.
 *
 * @dataField outputActive | Boolean | Whether the output is active
 * @dataField outputState  | String  | The specific state of the output
 *
 * @eventType VirtualcamStateChanged
 * @eventSubscription Outputs
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api events
 * @category outputs
 */
void EventHandler::HandleVirtualcamStateChanged(ObsOutputState state)
{
	json eventData;
	eventData["outputActive"] = GetOutputStateActive(state);
	eventData["outputState"] = Utils::Obs::StringHelper::GetOutputState(state);
	BroadcastEvent(EventSubscription::Outputs, "VirtualcamStateChanged", eventData);
}

/**
 * The replay buffer has been saved.
 *
 * @dataField savedReplayPath | String | Path of the saved replay file
 *
 * @eventType ReplayBufferSaved
 * @eventSubscription Outputs
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api events
 * @category outputs
 */
void EventHandler::HandleReplayBufferSaved()
{
	json eventData;
	eventData["savedReplayPath"] = Utils::Obs::StringHelper::GetLastReplayBufferFilePath();
	BroadcastEvent(EventSubscription::Outputs, "ReplayBufferSaved", eventData);
}
