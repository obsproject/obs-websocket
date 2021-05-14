#pragma once

namespace EventSubscriptions {
	enum EventSubscriptions {
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
	};
};
