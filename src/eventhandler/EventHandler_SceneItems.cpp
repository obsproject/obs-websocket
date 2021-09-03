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
	eventData["sceneItemIndex"] = obs_sceneitem_get_order_position(sceneItem);
	eventHandler->_webSocketServer->BroadcastEvent(EventSubscription::SceneItems, "SceneItemCreated", eventData);
}

// Will not be emitted if an item is removed due to the parent scene being removed.
void EventHandler::HandleSceneItemRemoved(void *param, calldata_t *data)
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
	eventData["sceneItemIndex"] = obs_sceneitem_get_order_position(sceneItem);
	eventHandler->_webSocketServer->BroadcastEvent(EventSubscription::SceneItems, "SceneItemRemoved", eventData);
}

void EventHandler::HandleSceneItemListReindexed(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_scene_t *scene = GetCalldataPointer<obs_scene_t>(data, "scene");
	if (!scene)
		return;

	json eventData;
	eventData["sceneName"] = obs_source_get_name(obs_scene_get_source(scene));
	eventData["sceneItems"] = Utils::Obs::ListHelper::GetSceneItemList(scene, true);
	eventHandler->_webSocketServer->BroadcastEvent(EventSubscription::SceneItems, "SceneItemListReindexed", eventData);
}

void EventHandler::HandleSceneItemEnableStateChanged(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_scene_t *scene = GetCalldataPointer<obs_scene_t>(data, "scene");
	if (!scene)
		return;

	obs_sceneitem_t *sceneItem = GetCalldataPointer<obs_sceneitem_t>(data, "item");
	if (!sceneItem)
		return;

	bool sceneItemEnabled = calldata_bool(data, "visible");

	json eventData;
	eventData["sceneName"] = obs_source_get_name(obs_scene_get_source(scene));
	eventData["sceneItemId"] = obs_sceneitem_get_id(sceneItem);
	eventData["sceneItemEnabled"] = sceneItemEnabled;
	eventHandler->_webSocketServer->BroadcastEvent(EventSubscription::SceneItems, "SceneItemEnableStateChanged", eventData);
}

void EventHandler::HandleSceneItemLockStateChanged(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_scene_t *scene = GetCalldataPointer<obs_scene_t>(data, "scene");
	if (!scene)
		return;

	obs_sceneitem_t *sceneItem = GetCalldataPointer<obs_sceneitem_t>(data, "item");
	if (!sceneItem)
		return;

	bool sceneItemLocked = calldata_bool(data, "locked");

	json eventData;
	eventData["sceneName"] = obs_source_get_name(obs_scene_get_source(scene));
	eventData["sceneItemId"] = obs_sceneitem_get_id(sceneItem);
	eventData["sceneItemLocked"] = sceneItemLocked;
	eventHandler->_webSocketServer->BroadcastEvent(EventSubscription::SceneItems, "SceneItemLockStateChanged", eventData);
}

void EventHandler::HandleSceneItemTransformChanged(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	json eventData;
	eventHandler->_webSocketServer->BroadcastEvent(EventSubscription::SceneItems, "SceneItemTransformChanged", eventData);
}
