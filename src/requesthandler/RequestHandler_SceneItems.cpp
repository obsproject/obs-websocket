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
	if (request.RequestData.contains("sceneItemEnabled") && request.RequestData["sceneItemEnabled"].is_boolean())
		sceneItemEnabled = request.RequestData["sceneItemEnabled"];

	obs_sceneitem_t *sceneItem = Utils::Obs::ActionHelper::CreateSceneItem(source, scene, sceneItemEnabled);
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

RequestResult RequestHandler::GetSceneItemTransform(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment);
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
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment);
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
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment);
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
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment);
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
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment);
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
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment);
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
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment);
	if (!(sceneItem && request.ValidateNumber("sceneItemIndex", statusCode, comment, 0, 8192)))
		return RequestResult::Error(statusCode, comment);

	int sceneItemIndex = request.RequestData["sceneItemIndex"];

	obs_sceneitem_set_order_position(sceneItem, sceneItemIndex);

	return RequestResult::Success();
}
