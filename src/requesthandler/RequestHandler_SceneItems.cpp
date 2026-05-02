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
#include <optional>
#include "RequestHandler.h"

/**
 * Gets a list of all scene items in a scene.
 *
 * Scenes only
 *
 * @requestField ?canvasUuid | String | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName  | String | Name of the scene to get the items of
 * @requestField ?sceneUuid  | String | UUID of the scene to get the items of
 *
 * @responseField sceneItems | Array<Object> | Array of scene items in the scene
 *
 * @requestType GetSceneItemList
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::GetSceneItemList(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease scene = request.AcquireScene(statusCode, comment);
	if (!scene)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["sceneItems"] = Utils::Obs::ArrayHelper::GetSceneItemList(obs_scene_from_source(scene));

	return RequestResult::Success(responseData);
}

/**
 * Basically GetSceneItemList, but for groups.
 *
 * Using groups at all in OBS is discouraged, as they are very broken under the hood. Please use nested scenes instead.
 *
 * Groups only
 *
 * @requestField ?canvasUuid | String | UUID of the canvas the group is in, if using the sceneName field
 * @requestField ?sceneName  | String | Name of the group to get the items of
 * @requestField ?sceneUuid  | String | UUID of the group to get the items of
 *
 * @responseField sceneItems | Array<Object> | Array of scene items in the group
 *
 * @requestType GetGroupSceneItemList
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::GetGroupSceneItemList(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease scene = request.AcquireScene(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_GROUP_ONLY);
	if (!scene)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["sceneItems"] = Utils::Obs::ArrayHelper::GetSceneItemList(obs_group_from_source(scene));

	return RequestResult::Success(responseData);
}

/**
 * Searches a scene for a source, and returns its id.
 *
 * Scenes and Groups
 *
 * @requestField ?canvasUuid   | String | UUID of the canvas the scene or group is in, if using the sceneName field
 * @requestField ?sceneName    | String | Name of the scene or group to search in
 * @requestField ?sceneUuid    | String | UUID of the scene or group to search in
 * @requestField sourceName    | String | Name of the source to find
 * @requestField ?searchOffset | Number | Number of matches to skip during search. >= 0 means first forward. -1 means last (top) item | >= -1 | 0
 *
 * @responseField sceneItemId | Number | Numeric ID of the scene item
 *
 * @requestType GetSceneItemId
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::GetSceneItemId(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneAutoRelease scene = request.AcquireScene2(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!(scene && request.ValidateString("sourceName", statusCode, comment))) // TODO: Source UUID support
		return RequestResult::Error(statusCode, comment);

	std::string sourceName = request.RequestData["sourceName"];

	int offset = 0;
	if (request.Contains("searchOffset")) {
		if (!request.ValidateOptionalNumber("searchOffset", statusCode, comment, -1))
			return RequestResult::Error(statusCode, comment);
		offset = request.RequestData["searchOffset"];
	}

	OBSSceneItemAutoRelease item = Utils::Obs::SearchHelper::GetSceneItemByName(scene, sourceName, offset);
	if (!item)
		return RequestResult::Error(RequestStatus::ResourceNotFound,
					    "No scene items were found in the specified scene by that name or offset.");

	json responseData;
	responseData["sceneItemId"] = obs_sceneitem_get_id(item);

	return RequestResult::Success(responseData);
}

/**
 * Gets the source associated with a scene item.
 *
 * @requestField ?canvasUuid | String | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName  | String | Name of the scene the item is in
 * @requestField ?sceneUuid  | String | UUID of the scene the item is in
 * @requestField sceneItemId | Number | Numeric ID of the scene item | >= 0
 *
 * @responseField sourceName | String | Name of the source associated with the scene item
 * @responseField sourceUuid | String | UUID of the source associated with the scene item
 *
 * @requestType GetSceneItemSource
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.4.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::GetSceneItemSource(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	OBSSource source = obs_sceneitem_get_source(sceneItem);

	json responseData;
	responseData["sourceName"] = obs_source_get_name(source);
	responseData["sourceUuid"] = obs_source_get_uuid(source);

	return RequestResult::Success(responseData);
}

/**
 * Creates a new scene item using a source.
 *
 * Scenes only
 *
 * @requestField ?canvasUuid       | String  | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName        | String  | Name of the scene to create the new item in
 * @requestField ?sceneUuid        | String  | UUID of the scene to create the new item in
 * @requestField ?sourceName       | String  | Name of the source to add to the scene
 * @requestField ?sourceUuid       | String  | UUID of the source to add to the scene
 * @requestField ?sceneItemEnabled | Boolean | Enable state to apply to the scene item on creation | True
 *
 * @responseField sceneItemId | Number | Numeric ID of the scene item
 *
 * @requestType CreateSceneItem
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::CreateSceneItem(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease sceneSource = request.AcquireScene(statusCode, comment);
	if (!sceneSource)
		return RequestResult::Error(statusCode, comment);

	OBSScene scene = obs_scene_from_source(sceneSource);

	OBSSourceAutoRelease source = request.AcquireSource("canvasUuid", "sourceName", "sourceUuid", statusCode, comment);
	if (!source)
		return RequestResult::Error(statusCode, comment);

	if (sceneSource == source)
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

/**
 * Removes a scene item from a scene.
 *
 * Scenes only
 *
 * @requestField ?canvasUuid | String | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName  | String | Name of the scene the item is in
 * @requestField ?sceneUuid  | String | UUID of the scene the item is in
 * @requestField sceneItemId | Number | Numeric ID of the scene item | >= 0
 *
 * @requestType RemoveSceneItem
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::RemoveSceneItem(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	// Makes the UI log `User Removed source '[source]' from scene '(null)'`. This is not a problem, just a side effect.
	obs_sceneitem_remove(sceneItem);

	return RequestResult::Success();
}

/**
 * Duplicates a scene item, copying all transform and crop info.
 *
 * Scenes only
 *
 * @requestField ?canvasUuid           | String | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName            | String | Name of the scene the item is in
 * @requestField ?sceneUuid            | String | UUID of the scene the item is in
 * @requestField sceneItemId           | Number | Numeric ID of the scene item | >= 0
 * @requestField ?destinationSceneName | String | Name of the scene to create the duplicated item in | From scene is assumed
 * @requestField ?destinationSceneUuid | String | UUID of the scene to create the duplicated item in | From scene is assumed
 *
 * @responseField sceneItemId | Number | Numeric ID of the duplicated scene item
 *
 * @requestType DuplicateSceneItem
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::DuplicateSceneItem(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	// Get destination scene
	obs_scene_t *destinationScene;
	if (request.Contains("destinationSceneName")) {
		OBSSourceAutoRelease destinationSceneSource = request.AcquireSource("destinationCanvasUuid", "destinationSceneName",
										"destinationSceneUuid", statusCode, comment);
		if (!destinationSceneSource)
			return RequestResult::Error(statusCode, comment);

		// Reimplementation of GetScene2
		if (obs_source_get_type(destinationSceneSource) != OBS_SOURCE_TYPE_SCENE)
			return RequestResult::Error(RequestStatus::InvalidResourceType, "The specified source is not a scene.");
		if (obs_source_is_group(destinationSceneSource))
			return RequestResult::Error(RequestStatus::InvalidResourceType,
						    "The specified source is not a scene. (Is group)");

		destinationScene = obs_scene_get_ref(obs_scene_from_source(destinationSceneSource));
	} else {
		destinationScene = obs_scene_get_ref(obs_sceneitem_get_scene(sceneItem));
		if (!destinationScene)
			return RequestResult::Error(RequestStatus::RequestProcessingFailed,
						    "Internal error: Failed to get ref for scene of scene item.");
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
	obs_sceneitem_get_info2(sceneItem, &sceneItemTransform);
	obs_sceneitem_get_crop(sceneItem, &sceneItemCrop);

	// Create the new item
	OBSSceneItemAutoRelease newSceneItem = Utils::Obs::ActionHelper::CreateSceneItem(
		sceneItemSource, destinationScene, sceneItemEnabled, &sceneItemTransform, &sceneItemCrop);
	obs_scene_release(destinationScene);
	if (!newSceneItem)
		return RequestResult::Error(RequestStatus::ResourceCreationFailed, "Failed to create the scene item.");

	json responseData;
	responseData["sceneItemId"] = obs_sceneitem_get_id(newSceneItem);

	return RequestResult::Success(responseData);
}

/**
 * Gets the transform and crop info of a scene item.
 *
 * Scenes and Groups
 *
 * @requestField ?canvasUuid | String | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName  | String | Name of the scene the item is in
 * @requestField ?sceneUuid  | String | UUID of the scene the item is in
 * @requestField sceneItemId | Number | Numeric ID of the scene item | >= 0
 *
 * @responseField sceneItemTransform | Object | Object containing scene item transform info
 *
 * @requestType GetSceneItemTransform
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::GetSceneItemTransform(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["sceneItemTransform"] = Utils::Obs::ObjectHelper::GetSceneItemTransform(sceneItem);

	return RequestResult::Success(responseData);
}

/**
 * Sets the transform and crop info of a scene item.
 *
 * @requestField ?canvasUuid        | String | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName         | String | Name of the scene the item is in
 * @requestField ?sceneUuid         | String | UUID of the scene the item is in
 * @requestField sceneItemId        | Number | Numeric ID of the scene item | >= 0
 * @requestField sceneItemTransform | Object | Object containing scene item transform info to update
 *
 * @requestType SetSceneItemTransform
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::SetSceneItemTransform(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!(sceneItem && request.ValidateObject("sceneItemTransform", statusCode, comment)))
		return RequestResult::Error(statusCode, comment);

	// Create a fake request to use checks on the sub object
	Request r("", request.RequestData["sceneItemTransform"]);

	bool transformChanged = false;
	bool cropChanged = false;
	obs_transform_info sceneItemTransform;
	obs_sceneitem_crop sceneItemCrop;
	obs_sceneitem_get_info2(sceneItem, &sceneItemTransform);
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
		float finalWidth = scaleX * sourceWidth;
		if (!(finalWidth > -90001.0 && finalWidth < 90001.0))
			return RequestResult::Error(RequestStatus::RequestFieldOutOfRange,
						    "The field `scaleX` is too small or large for the current source resolution.");
		sceneItemTransform.scale.x = scaleX;
		transformChanged = true;
	}
	if (r.Contains("scaleY")) {
		if (!r.ValidateOptionalNumber("scaleY", statusCode, comment, -90001.0, 90001.0))
			return RequestResult::Error(statusCode, comment);
		float scaleY = r.RequestData["scaleY"];
		float finalHeight = scaleY * sourceHeight;
		if (!(finalHeight > -90001.0 && finalHeight < 90001.0))
			return RequestResult::Error(RequestStatus::RequestFieldOutOfRange,
						    "The field `scaleY` is too small or large for the current source resolution.");
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
		enum obs_bounds_type boundsType = r.RequestData["boundsType"];
		if (boundsType == OBS_BOUNDS_NONE && r.RequestData["boundsType"] != "OBS_BOUNDS_NONE")
			return RequestResult::Error(RequestStatus::InvalidRequestField,
						    "The field `boundsType` has an invalid value.");
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

	if (r.Contains("cropToBounds")) {
		if (!r.ValidateOptionalBoolean("cropToBounds", statusCode, comment))
			return RequestResult::Error(statusCode, comment);
		sceneItemTransform.crop_to_bounds = r.RequestData["cropToBounds"];
		transformChanged = true;
	}

	if (!transformChanged && !cropChanged)
		return RequestResult::Error(RequestStatus::CannotAct, "You have not provided any valid transform changes.");

	if (transformChanged)
		obs_sceneitem_set_info2(sceneItem, &sceneItemTransform);

	if (cropChanged)
		obs_sceneitem_set_crop(sceneItem, &sceneItemCrop);

	return RequestResult::Success();
}

/**
 * Gets the enable state of a scene item.
 *
 * Scenes and Groups
 *
 * @requestField ?canvasUuid | String | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName  | String | Name of the scene the item is in
 * @requestField ?sceneUuid  | String | UUID of the scene the item is in
 * @requestField sceneItemId | Number | Numeric ID of the scene item | >= 0
 *
 * @responseField sceneItemEnabled | Boolean | Whether the scene item is enabled. `true` for enabled, `false` for disabled
 *
 * @requestType GetSceneItemEnabled
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::GetSceneItemEnabled(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["sceneItemEnabled"] = obs_sceneitem_visible(sceneItem);

	return RequestResult::Success(responseData);
}

/**
 * Sets the enable state of a scene item.
 *
 * Scenes and Groups
 *
 * @requestField ?canvasUuid      | String  | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName       | String  | Name of the scene the item is in
 * @requestField ?sceneUuid       | String  | UUID of the scene the item is in
 * @requestField sceneItemId      | Number  | Numeric ID of the scene item | >= 0
 * @requestField sceneItemEnabled | Boolean | New enable state of the scene item
 *
 * @requestType SetSceneItemEnabled
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::SetSceneItemEnabled(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!(sceneItem && request.ValidateBoolean("sceneItemEnabled", statusCode, comment)))
		return RequestResult::Error(statusCode, comment);

	bool sceneItemEnabled = request.RequestData["sceneItemEnabled"];

	obs_sceneitem_set_visible(sceneItem, sceneItemEnabled);

	return RequestResult::Success();
}

/**
 * Gets the lock state of a scene item.
 *
 * Scenes and Groups
 *
 * @requestField ?canvasUuid | String | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName  | String | Name of the scene the item is in
 * @requestField ?sceneUuid  | String | UUID of the scene the item is in
 * @requestField sceneItemId | Number | Numeric ID of the scene item | >= 0
 *
 * @responseField sceneItemLocked | Boolean | Whether the scene item is locked. `true` for locked, `false` for unlocked
 *
 * @requestType GetSceneItemLocked
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::GetSceneItemLocked(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["sceneItemLocked"] = obs_sceneitem_locked(sceneItem);

	return RequestResult::Success(responseData);
}

/**
 * Sets the lock state of a scene item.
 *
 * Scenes and Group
 *
 * @requestField ?canvasUuid     | String  | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName      | String  | Name of the scene the item is in
 * @requestField ?sceneUuid      | String  | UUID of the scene the item is in
 * @requestField sceneItemId     | Number  | Numeric ID of the scene item | >= 0
 * @requestField sceneItemLocked | Boolean | New lock state of the scene item
 *
 * @requestType SetSceneItemLocked
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::SetSceneItemLocked(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!(sceneItem && request.ValidateBoolean("sceneItemLocked", statusCode, comment)))
		return RequestResult::Error(statusCode, comment);

	bool sceneItemLocked = request.RequestData["sceneItemLocked"];

	obs_sceneitem_set_locked(sceneItem, sceneItemLocked);

	return RequestResult::Success();
}

/**
 * Gets the index position of a scene item in a scene.
 *
 * An index of 0 is at the bottom of the source list in the UI.
 *
 * Scenes and Groups
 *
 * @requestField ?canvasUuid | String | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName  | String | Name of the scene the item is in
 * @requestField ?sceneUuid  | String | UUID of the scene the item is in
 * @requestField sceneItemId | Number | Numeric ID of the scene item | >= 0
 *
 * @responseField sceneItemIndex | Number | Index position of the scene item
 *
 * @requestType GetSceneItemIndex
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::GetSceneItemIndex(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["sceneItemIndex"] = obs_sceneitem_get_order_position(sceneItem);

	return RequestResult::Success(responseData);
}

/**
 * Sets the index position of a scene item in a scene.
 *
 * Scenes and Groups
 *
 * @requestField ?canvasUuid    | String | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName     | String | Name of the scene the item is in
 * @requestField ?sceneUuid     | String | UUID of the scene the item is in
 * @requestField sceneItemId    | Number | Numeric ID of the scene item         | >= 0
 * @requestField sceneItemIndex | Number | New index position of the scene item | >= 0
 *
 * @requestType SetSceneItemIndex
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::SetSceneItemIndex(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!(sceneItem && request.ValidateNumber("sceneItemIndex", statusCode, comment, 0, 8192)))
		return RequestResult::Error(statusCode, comment);

	int sceneItemIndex = request.RequestData["sceneItemIndex"];

	obs_sceneitem_set_order_position(sceneItem, sceneItemIndex);

	return RequestResult::Success();
}

/**
 * Gets the blend mode of a scene item.
 *
 * Blend modes:
 *
 * - `OBS_BLEND_NORMAL`
 * - `OBS_BLEND_ADDITIVE`
 * - `OBS_BLEND_SUBTRACT`
 * - `OBS_BLEND_SCREEN`
 * - `OBS_BLEND_MULTIPLY`
 * - `OBS_BLEND_LIGHTEN`
 * - `OBS_BLEND_DARKEN`
 *
 * Scenes and Groups
 *
 * @requestField ?canvasUuid | String | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName  | String | Name of the scene the item is in
 * @requestField ?sceneUuid  | String | UUID of the scene the item is in
 * @requestField sceneItemId | Number | Numeric ID of the scene item | >= 0
 *
 * @responseField sceneItemBlendMode | String | Current blend mode
 *
 * @requestType GetSceneItemBlendMode
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::GetSceneItemBlendMode(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	auto blendMode = obs_sceneitem_get_blending_mode(sceneItem);

	json responseData;
	responseData["sceneItemBlendMode"] = blendMode;

	return RequestResult::Success(responseData);
}

/**
 * Sets the blend mode of a scene item.
 *
 * Scenes and Groups
 *
 * @requestField ?canvasUuid        | String | UUID of the canvas the scene is in, if using the sceneName field
 * @requestField ?sceneName         | String | Name of the scene the item is in
 * @requestField ?sceneUuid         | String | UUID of the scene the item is in
 * @requestField sceneItemId        | Number | Numeric ID of the scene item | >= 0
 * @requestField sceneItemBlendMode | String | New blend mode
 *
 * @requestType SetSceneItemBlendMode
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::SetSceneItemBlendMode(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!(sceneItem && request.ValidateString("sceneItemBlendMode", statusCode, comment)))
		return RequestResult::Error(statusCode, comment);

	enum obs_blending_type blendMode = request.RequestData["sceneItemBlendMode"];
	if (blendMode == OBS_BLEND_NORMAL && request.RequestData["sceneItemBlendMode"] != "OBS_BLEND_NORMAL")
		return RequestResult::Error(RequestStatus::InvalidRequestField,
					    "The field sceneItemBlendMode has an invalid value.");

	obs_sceneitem_set_blending_mode(sceneItem, blendMode);

	return RequestResult::Success();
}

// Intentionally undocumented
RequestResult RequestHandler::GetSceneItemPrivateSettings(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	OBSDataAutoRelease privateSettings = obs_sceneitem_get_private_settings(sceneItem);

	json responseData;
	responseData["sceneItemSettings"] = Utils::Json::ObsDataToJson(privateSettings);

	return RequestResult::Success(responseData);
}

// Intentionally undocumented
RequestResult RequestHandler::SetSceneItemPrivateSettings(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem = request.AcquireSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem || !request.ValidateObject("sceneItemSettings", statusCode, comment, true))
		return RequestResult::Error(statusCode, comment);

	OBSDataAutoRelease privateSettings = obs_sceneitem_get_private_settings(sceneItem);

	OBSDataAutoRelease newSettings = Utils::Json::JsonToObsData(request.RequestData["sceneItemSettings"]);

	// Always overlays to prevent destroying internal source unintentionally
	obs_data_apply(privateSettings, newSettings);

	return RequestResult::Success();
}

/**
 * Gets the show transition for a scene item.
 *
 * Scenes and Groups
 *
 * @requestField ?sceneName  | String | Name of the scene the item is in
 * @requestField ?sceneUuid  | String | UUID of the scene the item is in
 * @requestField sceneItemId | Number | Numeric ID of the scene item | >= 0
 *
 * @responseField transitionName     | String | Name of the transition or null if none set
 * @responseField transitionUuid     | String | UUID of the transition or null if none set
 * @responseField transitionKind     | String | Kind of the transition or null if none set
 * @responseField transitionDuration | Number | Duration of the transition in milliseconds or null if none set
 *
 * @requestType GetSceneItemShowTransition
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.7.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::GetSceneItemShowTransition(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem =
		request.ValidateSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	json responseData;

	OBSSourceAutoRelease transition = obs_sceneitem_get_transition(sceneItem, true);
	if (transition) {
		responseData["transitionName"] = obs_source_get_name(transition);
		responseData["transitionUuid"] = obs_source_get_uuid(transition);
		responseData["transitionKind"] = obs_source_get_id(transition);
		responseData["transitionDuration"] = obs_sceneitem_get_transition_duration(sceneItem, true);
	} else {
		responseData["transitionName"] = nullptr;
		responseData["transitionUuid"] = nullptr;
		responseData["transitionKind"] = nullptr;
		responseData["transitionDuration"] = nullptr;
	}

	return RequestResult::Success(responseData);
}

/**
 * Sets the show transition for a scene item.
 *
 * Scenes and Groups
 *
 * @requestField ?sceneName           | String  | Name of the scene the item is in
 * @requestField ?sceneUuid           | String  | UUID of the scene the item is in
 * @requestField sceneItemId          | Number  | Numeric ID of the scene item | >= 0
 * @requestField ?transitionName      | String  | Name of the transition to set. Pass null to remove transition | null
 * @requestField ?transitionDuration  | Number  | Duration of the transition in milliseconds | >= 0
 *
 * @requestType SetSceneItemShowTransition
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.7.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::SetSceneItemShowTransition(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem =
		request.ValidateSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	// Get transition name if provided
	std::optional<std::string> transitionName;
	if (request.RequestData.contains("transitionName") && !request.RequestData["transitionName"].is_null()) {
		if (!request.ValidateString("transitionName", statusCode, comment))
			return RequestResult::Error(statusCode, comment);
		transitionName = request.RequestData["transitionName"];
	}

	// Set transition if provided
	if (transitionName) {
		OBSSourceAutoRelease transition = Utils::Obs::SearchHelper::GetSceneTransitionByName(transitionName.value());
		if (!transition)
			return RequestResult::Error(RequestStatus::ResourceNotFound,
				"No transition was found with the name '" + transitionName.value() + "'.");
		obs_sceneitem_set_transition(sceneItem, true, transition);
	} else if (request.RequestData.contains("transitionName") && request.RequestData["transitionName"].is_null()) {
		// Remove transition if explicitly set to null
		obs_sceneitem_set_transition(sceneItem, true, nullptr);
	}

	// Set duration if provided
	if (request.RequestData.contains("transitionDuration")) {
		if (!request.ValidateNumber("transitionDuration", statusCode, comment, 0))
			return RequestResult::Error(statusCode, comment);
		uint32_t duration = request.RequestData["transitionDuration"];
		obs_sceneitem_set_transition_duration(sceneItem, true, duration);
	}

	return RequestResult::Success();
}

/**
 * Gets the hide transition for a scene item.
 *
 * Scenes and Groups
 *
 * @requestField ?sceneName  | String | Name of the scene the item is in
 * @requestField ?sceneUuid  | String | UUID of the scene the item is in
 * @requestField sceneItemId | Number | Numeric ID of the scene item | >= 0
 *
 * @responseField transitionName     | String | Name of the transition or null if none set
 * @responseField transitionUuid     | String | UUID of the transition or null if none set
 * @responseField transitionKind     | String | Kind of the transition or null if none set
 * @responseField transitionDuration | Number | Duration of the transition in milliseconds or null if none set
 *
 * @requestType GetSceneItemHideTransition
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.7.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::GetSceneItemHideTransition(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem =
		request.ValidateSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	json responseData;

	OBSSourceAutoRelease transition = obs_sceneitem_get_transition(sceneItem, false);
	if (transition) {
		responseData["transitionName"] = obs_source_get_name(transition);
		responseData["transitionUuid"] = obs_source_get_uuid(transition);
		responseData["transitionKind"] = obs_source_get_id(transition);
		responseData["transitionDuration"] = obs_sceneitem_get_transition_duration(sceneItem, false);
	} else {
		responseData["transitionName"] = nullptr;
		responseData["transitionUuid"] = nullptr;
		responseData["transitionKind"] = nullptr;
		responseData["transitionDuration"] = nullptr;
	}

	return RequestResult::Success(responseData);
}

/**
 * Sets the hide transition for a scene item.
 *
 * Scenes and Groups
 *
 * @requestField ?sceneName           | String  | Name of the scene the item is in
 * @requestField ?sceneUuid           | String  | UUID of the scene the item is in
 * @requestField sceneItemId          | Number  | Numeric ID of the scene item | >= 0
 * @requestField ?transitionName      | String  | Name of the transition to set. Pass null to remove transition | null
 * @requestField ?transitionDuration  | Number  | Duration of the transition in milliseconds | >= 0
 *
 * @requestType SetSceneItemHideTransition
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.7.0
 * @api requests
 * @category scene items
 */
RequestResult RequestHandler::SetSceneItemHideTransition(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSceneItemAutoRelease sceneItem =
		request.ValidateSceneItem(statusCode, comment, OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP);
	if (!sceneItem)
		return RequestResult::Error(statusCode, comment);

	// Get transition name if provided
	std::optional<std::string> transitionName;
	if (request.RequestData.contains("transitionName") && !request.RequestData["transitionName"].is_null()) {
		if (!request.ValidateString("transitionName", statusCode, comment))
			return RequestResult::Error(statusCode, comment);
		transitionName = request.RequestData["transitionName"];
	}

	// Set transition if provided
	if (transitionName) {
		OBSSourceAutoRelease transition = Utils::Obs::SearchHelper::GetSceneTransitionByName(transitionName.value());
		if (!transition)
			return RequestResult::Error(RequestStatus::ResourceNotFound,
				"No transition was found with the name '" + transitionName.value() + "'.");
		obs_sceneitem_set_transition(sceneItem, false, transition);
	} else if (request.RequestData.contains("transitionName") && request.RequestData["transitionName"].is_null()) {
		// Remove transition if explicitly set to null
		obs_sceneitem_set_transition(sceneItem, false, nullptr);
	}

	// Set duration if provided
	if (request.RequestData.contains("transitionDuration")) {
		if (!request.ValidateNumber("transitionDuration", statusCode, comment, 0))
			return RequestResult::Error(statusCode, comment);
		uint32_t duration = request.RequestData["transitionDuration"];
		obs_sceneitem_set_transition_duration(sceneItem, false, duration);
	}

	return RequestResult::Success();
}
