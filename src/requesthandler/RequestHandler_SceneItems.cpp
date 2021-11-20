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

RequestResult RequestHandler::SetSceneItemTransform(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.ValidateSceneItem("sceneName", "sceneItemId", statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!(sceneItem && request.ValidateObject("sceneItemTransform", statusCode, comment)))
		return RequestResult::Error(statusCode, comment);

	// Create a fake request to use checks on the sub object
	Request r("", request.RequestData["sceneItemTransform"]);

	bool transformChanged = false;
	bool cropChanged = false;
	obs_transform_info sceneItemTransform;
	obs_sceneitem_crop sceneItemCrop;
	obs_sceneitem_get_info(sceneItem, &sceneItemTransform);
	obs_sceneitem_get_crop(sceneItem, &sceneItemCrop);

	OBSSource source = obs_sceneitem_get_source(sceneItem);
	float sourceWidth = float(obs_source_get_width(source));
	float sourceHeight = float(obs_source_get_height(source));

	if (r.Contains("positionX")) {
		if (!r.ValidateOptionalNumber("positionX", statusCode, comment, -90001.0, 90001.0))
			return RequestResult::Error(statusCode, comment);
		sceneItemTransform.pos.x = r.RequestData["positionX"];
		transformChanged = true;
	}
	if (r.Contains("positionY")) {
		if (!r.ValidateOptionalNumber("positionY", statusCode, comment, -90001.0, 90001.0))
			return RequestResult::Error(statusCode, comment);
		sceneItemTransform.pos.y = r.RequestData["positionY"];
		transformChanged = true;
	}

	if (r.Contains("rotation")) {
		if (!r.ValidateOptionalNumber("rotation", statusCode, comment, -360.0, 360.0))
			return RequestResult::Error(statusCode, comment);
		sceneItemTransform.rot = r.RequestData["rotation"];
		transformChanged = true;
	}

	if (r.Contains("scaleX")) {
		if (!r.ValidateOptionalNumber("scaleX", statusCode, comment))
			return RequestResult::Error(statusCode, comment);
		float scaleX = r.RequestData["scaleX"];
		if (!(-90001.0 < (scaleX * sourceWidth) < 90001.0))
			return RequestResult::Error(RequestStatus::RequestParameterOutOfRange, "The parameter scaleX is too small or large for the current source resolution.");
		sceneItemTransform.scale.x = scaleX;
		transformChanged = true;
	}
	if (r.Contains("scaleY")) {
		if (!r.ValidateOptionalNumber("scaleY", statusCode, comment, -90001.0, 90001.0))
			return RequestResult::Error(statusCode, comment);
		float scaleY = r.RequestData["scaleY"];
		if (!(-90001.0 < (scaleY * sourceHeight) < 90001.0))
			return RequestResult::Error(RequestStatus::RequestParameterOutOfRange, "The parameter scaleY is too small or large for the current source resolution.");
		sceneItemTransform.scale.y = scaleY;
		transformChanged = true;
	}

	if (r.Contains("alignment")) {
		if (!r.ValidateOptionalNumber("alignment", statusCode, comment, 0, std::numeric_limits<uint32_t>::max()))
			return RequestResult::Error(statusCode, comment);
		sceneItemTransform.alignment = r.RequestData["alignment"];
		transformChanged = true;
	}

	if (r.Contains("boundsType")) {
		if (!r.ValidateOptionalString("boundsType", statusCode, comment))
			return RequestResult::Error(statusCode, comment);
		std::string boundsTypeString = r.RequestData["boundsType"];
		enum obs_bounds_type boundsType = Utils::Obs::EnumHelper::GetSceneItemBoundsType(boundsTypeString);
		if (boundsType == OBS_BOUNDS_NONE && boundsTypeString != "OBS_BOUNDS_NONE")
			return RequestResult::Error(RequestStatus::InvalidRequestParameter, "The parameter boundsType has an invalid value.");
		sceneItemTransform.bounds_type = boundsType;
		transformChanged = true;
	}

	if (r.Contains("boundsAlignment")) {
		if (!r.ValidateOptionalNumber("boundsAlignment", statusCode, comment, 0, std::numeric_limits<uint32_t>::max()))
			return RequestResult::Error(statusCode, comment);
		sceneItemTransform.bounds_alignment = r.RequestData["boundsAlignment"];
		transformChanged = true;
	}

	if (r.Contains("boundsWidth")) {
		if (!r.ValidateOptionalNumber("boundsWidth", statusCode, comment, 1.0, 90001.0))
			return RequestResult::Error(statusCode, comment);
		sceneItemTransform.bounds.x = r.RequestData["boundsWidth"];
		transformChanged = true;
	}
	if (r.Contains("boundsHeight")) {
		if (!r.ValidateOptionalNumber("boundsHeight", statusCode, comment, 1.0, 90001.0))
			return RequestResult::Error(statusCode, comment);
		sceneItemTransform.bounds.y = r.RequestData["boundsHeight"];
		transformChanged = true;
	}

	if (r.Contains("cropLeft")) {
		if (!r.ValidateOptionalNumber("cropLeft", statusCode, comment, 0.0, 100000.0))
			return RequestResult::Error(statusCode, comment);
		sceneItemCrop.left = r.RequestData["cropLeft"];
		cropChanged = true;
	}
	if (r.Contains("cropRight")) {
		if (!r.ValidateOptionalNumber("cropRight", statusCode, comment, 0.0, 100000.0))
			return RequestResult::Error(statusCode, comment);
		sceneItemCrop.right = r.RequestData["cropRight"];
		cropChanged = true;
	}
	if (r.Contains("cropTop")) {
		if (!r.ValidateOptionalNumber("cropTop", statusCode, comment, 0.0, 100000.0))
			return RequestResult::Error(statusCode, comment);
		sceneItemCrop.top = r.RequestData["cropTop"];
		cropChanged = true;
	}
	if (r.Contains("cropBottom")) {
		if (!r.ValidateOptionalNumber("cropBottom", statusCode, comment, 0.0, 100000.0))
			return RequestResult::Error(statusCode, comment);
		sceneItemCrop.bottom = r.RequestData["cropBottom"];
		cropChanged = true;
	}

	if (!transformChanged && !cropChanged)
		return RequestResult::Error(RequestStatus::CannotAct, "You have not provided any valid transform changes.");

	if (transformChanged)
		obs_sceneitem_set_info(sceneItem, &sceneItemTransform);

	if (cropChanged)
		obs_sceneitem_set_crop(sceneItem, &sceneItemCrop);

	return RequestResult::Success();
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
