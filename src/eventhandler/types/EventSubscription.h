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

#pragma once

namespace EventSubscription {
	enum EventSubscription {
		// Set subscriptions to 0 to disable all events
		None = 0,
		// Receive events in the `General` category
		General = (1 << 0),
		// Receive events in the `Config` category
		Config = (1 << 1),
		// Receive events in the `Scenes` category
		Scenes = (1 << 2),
		// Receive events in the `Inputs` category
		Inputs = (1 << 3),
		// Receive events in the `Transitions` category
		Transitions = (1 << 4),
		// Receive events in the `Filters` category
		Filters = (1 << 5),
		// Receive events in the `Outputs` category
		Outputs = (1 << 6),
		// Receive events in the `Scene Items` category
		SceneItems = (1 << 7),
		// Receive events in the `MediaInputs` category
		MediaInputs = (1 << 8),
		// Receive all event categories
		All = (General | Config | Scenes | Inputs | Transitions | Filters | Outputs | SceneItems | MediaInputs),
		// InputVolumeMeters event (high-volume)
		InputVolumeMeters = (1 << 9),
		// InputActiveStateChanged event (high-volume)
		InputActiveStateChanged = (1 << 10),
		// InputShowStateChanged event (high-volume)
		InputShowStateChanged = (1 << 11),
		// SceneItemTransformChanged event (high-volume)
		SceneItemTransformChanged = (1 << 12),
	};
};
