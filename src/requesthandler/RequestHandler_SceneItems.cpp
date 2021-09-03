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
