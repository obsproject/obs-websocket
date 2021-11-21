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

void EventHandler::HandleTransitionCreated(obs_source_t *source)
{
	json eventData;
	eventData["transitionName"] = obs_source_get_name(source);
	eventData["transitionKind"] = obs_source_get_id(source);
	eventData["transitionFixed"] = obs_transition_fixed(source);
	BroadcastEvent(EventSubscription::Transitions, "TransitionCreated", eventData);
}

void EventHandler::HandleTransitionRemoved(obs_source_t *source)
{
	json eventData;
	eventData["transitionName"] = obs_source_get_name(source);
	BroadcastEvent(EventSubscription::Transitions, "TransitionRemoved", eventData);
}

void EventHandler::HandleTransitionNameChanged(obs_source_t *, std::string oldTransitionName, std::string transitionName)
{
	json eventData;
	eventData["oldTransitionName"] = oldTransitionName;
	eventData["transitionName"] = transitionName;
	BroadcastEvent(EventSubscription::Transitions, "TransitionNameChanged", eventData);
}
