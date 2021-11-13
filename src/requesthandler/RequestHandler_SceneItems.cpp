#include "RequestHandler.h"

RequestResult RequestHandler::GetSceneItemList(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease scene = request.ValidateScene("sceneName", statusCode, comment);
	if (!scene)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["sceneItems"] = Utils::Obs::ListHelper::GetSceneItemList(obs_scene_from_source(scene));

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::GetGroupSceneItemList(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease scene = request.ValidateScene("sceneName", statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_GROUP_ONLY);
	if (!scene)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["sceneItems"] = Utils::Obs::ListHelper::GetSceneItemList(obs_group_from_source(scene));

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::GetSceneItemId(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease sceneSource = request.ValidateScene("sceneName", statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!(sceneSource && request.ValidateString("sourceName", statusCode, comment)))
		return RequestResult::Error(statusCode, comment);

	OBSScene scene = obs_scene_from_source(sceneSource);
	if (!scene) {
		scene = obs_group_from_source(sceneSource);
		if (!scene) // This should never happen
			return RequestResult::Error(RequestStatus::GenericError, "Somehow the scene was found but the scene object could not be fetched. Please report this to the obs-websocket developers.");
	}

	std::string sourceName = request.RequestData["sourceName"];

	OBSSceneItemAutoRelease item = Utils::Obs::SearchHelper::GetSceneItemByName(scene, sourceName);
	if (!item)
		return RequestResult::Error(RequestStatus::ResourceNotFound, "No scene items were found in the specified scene by that name.");

	json responseData;
	responseData["sceneItemId"] = obs_sceneitem_get_id(item);

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::CreateSceneItem(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease sceneSource = request.ValidateScene("sceneName", statusCode, comment);
	if (!sceneSource)
		return RequestResult::Error(statusCode, comment);

	OBSScene scene = obs_scene_from_source(sceneSource);

	OBSSourceAutoRelease source = request.ValidateSource("sourceName", statusCode, comment);
	if (!source)
		return RequestResult::Error(statusCode, comment);

	if (request.RequestData["sceneName"] == request.RequestData["sourceName"])
		return RequestResult::Error(RequestStatus::CannotAct, "You cannot create scene item of a scene within itself.");

	bool sceneItemEnabled = true;
	if (request.Contains("sceneItemEnabled")) {
		if (!request.ValidateOptionalBoolean("sceneItemEnabled", statusCode, comment))
			return RequestResult::Error(statusCode, comment);
		sceneItemEnabled = request.RequestData["sceneItemEnabled"];
	}

	OBSSceneItemAutoRelease sceneItem = Utils::Obs::ActionHelper::CreateSceneItem(source, scene, sceneItemEnabled);
	if (!sceneItem)
		return RequestResult::Error(RequestStatus::ResourceCreationFailed, "Failed to create the scene item.");

	json responseData;
	responseData["sceneItemId"] = obs_sceneitem_get_id(sceneItem);

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::RemoveSceneItem(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	obs_sceneitem_remove(sceneItem);

	return RequestResult::Success();
}

RequestResult RequestHandler::DuplicateSceneItem(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	// Get destination scene
	obs_scene_t *destinationScene;
	if (request.Contains("destinationSceneName")) {
		destinationScene = request.ValidateScene2("destinationSceneName", statusCode, comment);
		if (!destinationScene)
			return RequestResult::Error(statusCode, comment);
	} else {
		destinationScene = obs_sceneitem_get_scene(sceneItem);
		obs_scene_addref(destinationScene);
	}

	if (obs_sceneitem_is_group(sceneItem) && obs_sceneitem_get_scene(sceneItem) == destinationScene) {
		obs_scene_release(destinationScene);
		return RequestResult::Error(RequestStatus::ResourceCreationFailed, "Scenes may only have one instance of a group.");
	}

	// Get scene item details
	OBSSource sceneItemSource = obs_sceneitem_get_source(sceneItem);
	bool sceneItemEnabled = obs_sceneitem_visible(sceneItem);
	obs_transform_info sceneItemTransform;
	obs_sceneitem_crop sceneItemCrop;
	obs_sceneitem_get_info(sceneItem, &sceneItemTransform);
	obs_sceneitem_get_crop(sceneItem, &sceneItemCrop);

	// Create the new item
	OBSSceneItemAutoRelease newSceneItem = Utils::Obs::ActionHelper::CreateSceneItem(sceneItemSource, destinationScene, sceneItemEnabled, &sceneItemTransform, &sceneItemCrop);
	obs_scene_release(destinationScene);
	if (!newSceneItem)
		return RequestResult::Error(RequestStatus::ResourceCreationFailed, "Failed to create the scene item.");

	json responseData;
	responseData["sceneItemId"] = obs_sceneitem_get_id(newSceneItem);

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::GetSceneItemTransform(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["sceneItemTransform"] = Utils::Obs::DataHelper::GetSceneItemTransform(sceneItem);

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::GetSceneItemEnabled(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["sceneItemEnabled"] = obs_sceneitem_visible(sceneItem);

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetSceneItemEnabled(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!(sceneItem && request.ValidateBoolean("sceneItemEnabled", statusCode, comment)))
		return RequestResult::Error(statusCode, comment);

	bool sceneItemEnabled = request.RequestData["sceneItemEnabled"];

	obs_sceneitem_set_visible(sceneItem, sceneItemEnabled);

	return RequestResult::Success();
}

RequestResult RequestHandler::GetSceneItemLocked(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["sceneItemLocked"] = obs_sceneitem_locked(sceneItem);

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetSceneItemLocked(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!(sceneItem && request.ValidateBoolean("sceneItemLocked", statusCode, comment)))
		return RequestResult::Error(statusCode, comment);

	bool sceneItemLocked = request.RequestData["sceneItemLocked"];

	obs_sceneitem_set_locked(sceneItem, sceneItemLocked);

	return RequestResult::Success();
}

RequestResult RequestHandler::GetSceneItemIndex(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["sceneItemIndex"] = obs_sceneitem_get_order_position(sceneItem);

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetSceneItemIndex(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!(sceneItem && request.ValidateNumber("sceneItemIndex", statusCode, comment, 0, 8192)))
		return RequestResult::Error(statusCode, comment);

	int sceneItemIndex = request.RequestData["sceneItemIndex"];

	obs_sceneitem_set_order_position(sceneItem, sceneItemIndex);

	return RequestResult::Success();
}
