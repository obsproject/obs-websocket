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

/**
 * The current scene transition has changed.
 *
 * @dataField transitionName | String | Name of the new transition
 *
 * @eventType CurrentSceneTransitionChanged
 * @eventSubscription Transitions
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api events
 * @category transitions
 */
void EventHandler::HandleCurrentSceneTransitionChanged()
{
	OBSSourceAutoRelease transition = obs_frontend_get_current_transition();

	json eventData;
	eventData["transitionName"] = obs_source_get_name(transition);
	BroadcastEvent(EventSubscription::Transitions, "CurrentSceneTransitionChanged", eventData);
}

/**
 * The current scene transition duration has changed.
 *
 * @dataField transitionDuration | Number | Transition duration in milliseconds
 *
 * @eventType CurrentSceneTransitionDurationChanged
 * @eventSubscription Transitions
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api events
 * @category transitions
 */
void EventHandler::HandleCurrentSceneTransitionDurationChanged()
{
	json eventData;
	eventData["transitionDuration"] = obs_frontend_get_transition_duration();
	BroadcastEvent(EventSubscription::Transitions, "CurrentSceneTransitionDurationChanged", eventData);
}
