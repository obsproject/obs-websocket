#include "EventHandler.h"

#include "../plugin-macros.generated.h"

void EventHandler::HandleSceneItemCreated(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_scene_t *scene = GetCalldataPointer<obs_scene_t>(data, "scene");
	if (!scene)
		return;

	obs_sceneitem_t *sceneItem = GetCalldataPointer<obs_sceneitem_t>(data, "item");
	if (!sceneItem)
		return;

	json eventData;
	eventData["sceneName"] = obs_source_get_name(obs_scene_get_source(scene));
	eventData["inputName"] = obs_source_get_name(obs_sceneitem_get_source(sceneItem));
	eventData["sceneItemId"] = obs_sceneitem_get_id(sceneItem);
	eventHandler->_webSocketServer->BroadcastEvent(EventSubscription::SceneItems, "SceneItemCreated", eventData);
}

void EventHandler::HandleSceneItemRemoved(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	json eventData;
	eventHandler->_webSocketServer->BroadcastEvent(EventSubscription::SceneItems, "SceneItemRemoved", eventData);
}

void EventHandler::HandleSceneItemListReindexed(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	json eventData;
	eventHandler->_webSocketServer->BroadcastEvent(EventSubscription::SceneItems, "SceneItemReindexed", eventData);
}

void EventHandler::HandleSceneItemEnableStateChanged(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	json eventData;
	eventHandler->_webSocketServer->BroadcastEvent(EventSubscription::SceneItems, "SceneItemEnableStateChanged", eventData);
}

void EventHandler::HandleSceneItemLockStateChanged(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	json eventData;
	eventHandler->_webSocketServer->BroadcastEvent(EventSubscription::SceneItems, "SceneItemLockStateChanged", eventData);
}

void EventHandler::HandleSceneItemTransformChanged(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	json eventData;
	eventHandler->_webSocketServer->BroadcastEvent(EventSubscription::SceneItems, "SceneItemTransformChanged", eventData);
}
