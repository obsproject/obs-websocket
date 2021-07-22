#include "EventHandler.h"
#include "../plugin-macros.generated.h"

void EventHandler::HandleSceneCreated(obs_source_t *source)
{
	json eventData;
	eventData["sceneName"] = obs_source_get_name(source);
	eventData["isGroup"] = obs_source_is_group(source);
	_webSocketServer->BroadcastEvent(EventSubscription::Scenes, "SceneCreated", eventData);
}

void EventHandler::HandleSceneRemoved(obs_source_t *source)
{
	json eventData;
	eventData["sceneName"] = obs_source_get_name(source);
	eventData["isGroup"] = obs_source_is_group(source);
	_webSocketServer->BroadcastEvent(EventSubscription::Scenes, "SceneRemoved", eventData);
}

void EventHandler::HandleSceneNameChanged(obs_source_t *source, std::string oldSceneName, std::string sceneName)
{
	json eventData;
	eventData["oldSceneName"] = oldSceneName;
	eventData["sceneName"] = sceneName;
	_webSocketServer->BroadcastEvent(EventSubscription::Scenes, "SceneNameChanged", eventData);
}

void EventHandler::HandleCurrentSceneChanged()
{
	OBSSourceAutoRelease currentScene = obs_frontend_get_current_scene();

	json eventData;
	eventData["sceneName"] = obs_source_get_name(currentScene);
	_webSocketServer->BroadcastEvent(EventSubscription::Scenes, "CurrentSceneChanged", eventData);
}

void EventHandler::HandleCurrentPreviewSceneChanged()
{
	OBSSourceAutoRelease currentPreviewScene = obs_frontend_get_current_preview_scene();

	// This event may be called when OBS is not in studio mode, however retreiving the source while not in studio mode will return null. 
	if (!currentPreviewScene)
		return;

	json eventData;
	eventData["sceneName"] = obs_source_get_name(currentPreviewScene);
	_webSocketServer->BroadcastEvent(EventSubscription::Scenes, "CurrentPreviewSceneChanged", eventData);
}

void EventHandler::HandleSceneListChanged()
{
	json eventData;
	eventData["scenes"] = Utils::Obs::ListHelper::GetSceneList();
	_webSocketServer->BroadcastEvent(EventSubscription::Scenes, "SceneListChanged", eventData);
}
