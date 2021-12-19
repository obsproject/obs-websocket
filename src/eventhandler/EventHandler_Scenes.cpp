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

void EventHandler::HandleSceneCreated(obs_source_t *source)
{
	json eventData;
	eventData["sceneName"] = obs_source_get_name(source);
	eventData["isGroup"] = obs_source_is_group(source);
	BroadcastEvent(EventSubscription::Scenes, "SceneCreated", eventData);
}

void EventHandler::HandleSceneRemoved(obs_source_t *source)
{
	json eventData;
	eventData["sceneName"] = obs_source_get_name(source);
	eventData["isGroup"] = obs_source_is_group(source);
	BroadcastEvent(EventSubscription::Scenes, "SceneRemoved", eventData);
}

void EventHandler::HandleSceneNameChanged(obs_source_t *, std::string oldSceneName, std::string sceneName)
{
	json eventData;
	eventData["oldSceneName"] = oldSceneName;
	eventData["sceneName"] = sceneName;
	BroadcastEvent(EventSubscription::Scenes, "SceneNameChanged", eventData);
}

void EventHandler::HandleCurrentSceneChanged()
{
	OBSSourceAutoRelease currentScene = obs_frontend_get_current_scene();

	json eventData;
	eventData["sceneName"] = obs_source_get_name(currentScene);
	BroadcastEvent(EventSubscription::Scenes, "CurrentSceneChanged", eventData);
}

void EventHandler::HandleCurrentPreviewSceneChanged()
{
	OBSSourceAutoRelease currentPreviewScene = obs_frontend_get_current_preview_scene();

	// This event may be called when OBS is not in studio mode, however retreiving the source while not in studio mode will return null. 
	if (!currentPreviewScene)
		return;

	json eventData;
	eventData["sceneName"] = obs_source_get_name(currentPreviewScene);
	BroadcastEvent(EventSubscription::Scenes, "CurrentPreviewSceneChanged", eventData);
}

void EventHandler::HandleSceneListChanged()
{
	json eventData;
	eventData["scenes"] = Utils::Obs::ArrayHelper::GetSceneList();
	BroadcastEvent(EventSubscription::Scenes, "SceneListChanged", eventData);
}
