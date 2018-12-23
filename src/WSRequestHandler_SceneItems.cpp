#include <QString>
#include "Utils.h"

#include "WSRequestHandler.h"

/**
* Gets the scene specific properties of the specified source item.
*
* @param {String (optional)} `scene-name` the name of the scene that the source item belongs to. Defaults to the current scene.
* @param {String} `item` The name of the source.
*
* @return {String} `name` The name of the source.
* @return {int} `position.x` The x position of the source from the left.
* @return {int} `position.y` The y position of the source from the top.
* @return {int} `position.alignment` The point on the source that the item is manipulated from.
* @return {double} `rotation` The clockwise rotation of the item in degrees around the point of alignment.
* @return {double} `scale.x` The x-scale factor of the source.
* @return {double} `scale.y` The y-scale factor of the source.
* @return {int} `crop.top` The number of pixels cropped off the top of the source before scaling.
* @return {int} `crop.right` The number of pixels cropped off the right of the source before scaling.
* @return {int} `crop.bottom` The number of pixels cropped off the bottom of the source before scaling.
* @return {int} `crop.left` The number of pixels cropped off the left of the source before scaling.
* @return {bool} `visible` If the source is visible.
* @return {String} `bounds.type` Type of bounding box.
* @return {int} `bounds.alignment` Alignment of the bounding box.
* @return {double} `bounds.x` Width of the bounding box.
* @return {double} `bounds.y` Height of the bounding box.
*
* @api requests
* @name GetSceneItemProperties
* @category scene items
* @since 4.3.0
*/
void WSRequestHandler::HandleGetSceneItemProperties(WSRequestHandler* req) {
	if (!req->hasField("item")) {
		req->SendErrorResponse("missing request parameters");
		return;
	}

	QString itemName = obs_data_get_string(req->data, "item");
	if (itemName.isEmpty()) {
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	QString sceneName = obs_data_get_string(req->data, "scene-name");
	OBSSourceAutoRelease scene = Utils::GetSceneFromNameOrCurrent(sceneName);
	if (!scene) {
		req->SendErrorResponse("requested scene doesn't exist");
		return;
	}

	OBSSceneItemAutoRelease sceneItem =
		Utils::GetSceneItemFromName(scene, itemName);
	if (!sceneItem) {
		req->SendErrorResponse("specified scene item doesn't exist");
		return;
	}

	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_string(data, "name", itemName.toUtf8());

	OBSDataAutoRelease posData = obs_data_create();
	vec2 pos;
	obs_sceneitem_get_pos(sceneItem, &pos);
	obs_data_set_double(posData, "x", pos.x);
	obs_data_set_double(posData, "y", pos.y);
	obs_data_set_int(posData, "alignment", obs_sceneitem_get_alignment(sceneItem));
	obs_data_set_obj(data, "position", posData);

	obs_data_set_double(data, "rotation", obs_sceneitem_get_rot(sceneItem));

	OBSDataAutoRelease scaleData = obs_data_create();
	vec2 scale;
	obs_sceneitem_get_scale(sceneItem, &scale);
	obs_data_set_double(scaleData, "x", scale.x);
	obs_data_set_double(scaleData, "y", scale.y);
	obs_data_set_obj(data, "scale", scaleData);

	OBSDataAutoRelease cropData = obs_data_create();
	obs_sceneitem_crop crop;
	obs_sceneitem_get_crop(sceneItem, &crop);
	obs_data_set_int(cropData, "left", crop.left);
	obs_data_set_int(cropData, "top", crop.top);
	obs_data_set_int(cropData, "right", crop.right);
	obs_data_set_int(cropData, "bottom", crop.bottom);
	obs_data_set_obj(data, "crop", cropData);

	obs_data_set_bool(data, "visible", obs_sceneitem_visible(sceneItem));

	OBSDataAutoRelease boundsData = obs_data_create();
	obs_bounds_type boundsType = obs_sceneitem_get_bounds_type(sceneItem);
	if (boundsType == OBS_BOUNDS_NONE) {
		obs_data_set_string(boundsData, "type", "OBS_BOUNDS_NONE");
	}
	else {
		switch (boundsType) {
		case OBS_BOUNDS_STRETCH: {
			obs_data_set_string(boundsData, "type", "OBS_BOUNDS_STRETCH");
			break;
		}
		case OBS_BOUNDS_SCALE_INNER: {
			obs_data_set_string(boundsData, "type", "OBS_BOUNDS_SCALE_INNER");
			break;
		}
		case OBS_BOUNDS_SCALE_OUTER: {
			obs_data_set_string(boundsData, "type", "OBS_BOUNDS_SCALE_OUTER");
			break;
		}
		case OBS_BOUNDS_SCALE_TO_WIDTH: {
			obs_data_set_string(boundsData, "type", "OBS_BOUNDS_SCALE_TO_WIDTH");
			break;
		}
		case OBS_BOUNDS_SCALE_TO_HEIGHT: {
			obs_data_set_string(boundsData, "type", "OBS_BOUNDS_SCALE_TO_HEIGHT");
			break;
		}
		case OBS_BOUNDS_MAX_ONLY: {
			obs_data_set_string(boundsData, "type", "OBS_BOUNDS_MAX_ONLY");
			break;
		}
		}
		obs_data_set_int(boundsData, "alignment", obs_sceneitem_get_bounds_alignment(sceneItem));
		vec2 bounds;
		obs_sceneitem_get_bounds(sceneItem, &bounds);
		obs_data_set_double(boundsData, "x", bounds.x);
		obs_data_set_double(boundsData, "y", bounds.y);
	}
	obs_data_set_obj(data, "bounds", boundsData);

	req->SendOKResponse(data);
}

/**
* Sets the scene specific properties of a source. Unspecified properties will remain unchanged.
*
* @param {String (optional)} `scene-name` the name of the scene that the source item belongs to. Defaults to the current scene.
* @param {String} `item` The name of the source.
* @param {int} `position.x` The new x position of the source.
* @param {int} `position.y` The new y position of the source.
* @param {int} `position.alignment` The new alignment of the source.
* @param {double} `rotation` The new clockwise rotation of the item in degrees.
* @param {double} `scale.x` The new x scale of the item.
* @param {double} `scale.y` The new y scale of the item.
* @param {int} `crop.top` The new amount of pixels cropped off the top of the source before scaling.
* @param {int} `crop.bottom` The new amount of pixels cropped off the bottom of the source before scaling.
* @param {int} `crop.left` The new amount of pixels cropped off the left of the source before scaling.
* @param {int} `crop.right` The new amount of pixels cropped off the right of the source before scaling.
* @param {bool} `visible` The new visibility of the source. 'true' shows source, 'false' hides source.
* @param {String} `bounds.type` The new bounds type of the source.
* @param {int} `bounds.alignment` The new alignment of the bounding box. (0-2, 4-6, 8-10)
* @param {double} `bounds.x` The new width of the bounding box.
* @param {double} `bounds.y` The new height of the bounding box.
*
* @api requests
* @name SetSceneItemProperties
* @category scene items
* @since 4.3.0
*/
void WSRequestHandler::HandleSetSceneItemProperties(WSRequestHandler* req) {
	if (!req->hasField("item")) {
		req->SendErrorResponse("missing request parameters");
		return;
	}

	QString itemName = obs_data_get_string(req->data, "item");
	if (itemName.isEmpty()) {
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	QString sceneName = obs_data_get_string(req->data, "scene-name");
	OBSSourceAutoRelease scene = Utils::GetSceneFromNameOrCurrent(sceneName);
	if (!scene) {
		req->SendErrorResponse("requested scene doesn't exist");
		return;
	}

	OBSSceneItemAutoRelease sceneItem =
		Utils::GetSceneItemFromName(scene, itemName);
	if (!sceneItem) {
		req->SendErrorResponse("specified scene item doesn't exist");
		return;
	}

	bool badRequest = false;
	OBSDataAutoRelease errorMessage = obs_data_create();

	if (req->hasField("position")) {
		vec2 oldPosition;
		OBSDataAutoRelease positionError = obs_data_create();
		obs_sceneitem_get_pos(sceneItem, &oldPosition);
		OBSDataAutoRelease reqPosition = obs_data_get_obj(req->data, "position");
		vec2 newPosition = oldPosition;
		if (obs_data_has_user_value(reqPosition, "x")) {
			newPosition.x = obs_data_get_int(reqPosition, "x");
		}
		if (obs_data_has_user_value(reqPosition, "y")) {
			newPosition.y = obs_data_get_int(reqPosition, "y");
		}
		if (obs_data_has_user_value(reqPosition, "alignment")) {
			const uint32_t alignment = obs_data_get_int(reqPosition, "alignment");
			if (Utils::IsValidAlignment(alignment)) {
				obs_sceneitem_set_alignment(sceneItem, alignment);
			}
			else {
				badRequest = true;
				obs_data_set_string(positionError, "alignment", "invalid");
				obs_data_set_obj(errorMessage, "position", positionError);
			}
		}
		obs_sceneitem_set_pos(sceneItem, &newPosition);
	}

	if (req->hasField("rotation")) {
		obs_sceneitem_set_rot(sceneItem, (float)obs_data_get_double(req->data, "rotation"));
	}

	if (req->hasField("scale")) {
		vec2 oldScale;
		obs_sceneitem_get_scale(sceneItem, &oldScale);
		OBSDataAutoRelease reqScale = obs_data_get_obj(req->data, "scale");
		vec2 newScale = oldScale;
		if (obs_data_has_user_value(reqScale, "x")) {
			newScale.x = obs_data_get_double(reqScale, "x");
		}
		if (obs_data_has_user_value(reqScale, "y")) {
			newScale.y = obs_data_get_double(reqScale, "y");
		}
		obs_sceneitem_set_scale(sceneItem, &newScale);
	}

	if (req->hasField("crop")) {
		obs_sceneitem_crop oldCrop;
		obs_sceneitem_get_crop(sceneItem, &oldCrop);
		OBSDataAutoRelease reqCrop = obs_data_get_obj(req->data, "crop");
		obs_sceneitem_crop newCrop = oldCrop;
		if (obs_data_has_user_value(reqCrop, "top")) {
			newCrop.top = obs_data_get_int(reqCrop, "top");
		}
		if (obs_data_has_user_value(reqCrop, "right")) {
			newCrop.right = obs_data_get_int(reqCrop, "right");
		}
		if (obs_data_has_user_value(reqCrop, "bottom")) {
			newCrop.bottom = obs_data_get_int(reqCrop, "bottom");
		}
		if (obs_data_has_user_value(reqCrop, "left")) {
			newCrop.left = obs_data_get_int(reqCrop, "left");
		}
		obs_sceneitem_set_crop(sceneItem, &newCrop);
	}

	if (req->hasField("visible")) {
		obs_sceneitem_set_visible(sceneItem, obs_data_get_bool(req->data, "visible"));
	}

	if (req->hasField("bounds")) {
		bool badBounds = false;
		OBSDataAutoRelease boundsError = obs_data_create();
		OBSDataAutoRelease reqBounds = obs_data_get_obj(req->data, "bounds");
		if (obs_data_has_user_value(reqBounds, "type")) {
			const char* newBoundsType = obs_data_get_string(reqBounds, "type");
			if (newBoundsType == "OBS_BOUNDS_NONE") {
				obs_sceneitem_set_bounds_type(sceneItem, OBS_BOUNDS_NONE);
			}
			else if (newBoundsType == "OBS_BOUNDS_STRETCH") {
				obs_sceneitem_set_bounds_type(sceneItem, OBS_BOUNDS_STRETCH);
			}
			else if (newBoundsType == "OBS_BOUNDS_SCALE_INNER") {
				obs_sceneitem_set_bounds_type(sceneItem, OBS_BOUNDS_SCALE_INNER);
			}
			else if (newBoundsType == "OBS_BOUNDS_SCALE_OUTER") {
				obs_sceneitem_set_bounds_type(sceneItem, OBS_BOUNDS_SCALE_OUTER);
			}
			else if (newBoundsType == "OBS_BOUNDS_SCALE_TO_WIDTH") {
				obs_sceneitem_set_bounds_type(sceneItem, OBS_BOUNDS_SCALE_TO_WIDTH);
			}
			else if (newBoundsType == "OBS_BOUNDS_SCALE_TO_HEIGHT") {
				obs_sceneitem_set_bounds_type(sceneItem, OBS_BOUNDS_SCALE_TO_HEIGHT);
			}
			else if (newBoundsType == "OBS_BOUNDS_MAX_ONLY") {
				obs_sceneitem_set_bounds_type(sceneItem, OBS_BOUNDS_MAX_ONLY);
			}
			else {
				badRequest = badBounds = true;
				obs_data_set_string(boundsError, "type", "invalid");
			}
		}
		vec2 oldBounds;
		obs_sceneitem_get_bounds(sceneItem, &oldBounds);
		vec2 newBounds = oldBounds;
		if (obs_data_has_user_value(reqBounds, "x")) {
			newBounds.x = obs_data_get_double(reqBounds, "x");
		}
		if (obs_data_has_user_value(reqBounds, "y")) {
			newBounds.y = obs_data_get_double(reqBounds, "y");
		}
		obs_sceneitem_set_bounds(sceneItem, &newBounds);
		if (obs_data_has_user_value(reqBounds, "alignment")) {
			const uint32_t bounds_alignment = obs_data_get_int(reqBounds, "alignment");
			if (Utils::IsValidAlignment(bounds_alignment)) {
				obs_sceneitem_set_bounds_alignment(sceneItem, bounds_alignment);
			}
			else {
				badRequest = badBounds = true;
				obs_data_set_string(boundsError, "alignment", "invalid");
			}
		}
		if (badBounds) {
			obs_data_set_obj(errorMessage, "bounds", boundsError);
		}
	}

	if (badRequest) {
		req->SendErrorResponse(errorMessage);
	}
	else {
		req->SendOKResponse();
	}
}

/**
* Reset a scene item.
*
* @param {String (optional)} `scene-name` Name of the scene the source belongs to. Defaults to the current scene.
* @param {String} `item` Name of the source item.
*
* @api requests
* @name ResetSceneItem
* @category scene items
* @since 4.2.0
*/
void WSRequestHandler::HandleResetSceneItem(WSRequestHandler* req) {
	// TODO: remove this request, or refactor it to ResetSource

	if (!req->hasField("item")) {
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* itemName = obs_data_get_string(req->data, "item");
	if (!itemName) {
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	const char* sceneName = obs_data_get_string(req->data, "scene-name");
	OBSSourceAutoRelease scene = Utils::GetSceneFromNameOrCurrent(sceneName);
	if (!scene) {
		req->SendErrorResponse("requested scene doesn't exist");
		return;
	}

	OBSSceneItemAutoRelease sceneItem = Utils::GetSceneItemFromName(scene, itemName);
	if (sceneItem) {
		OBSSource sceneItemSource = obs_sceneitem_get_source(sceneItem);

		OBSDataAutoRelease settings = obs_source_get_settings(sceneItemSource);
		obs_source_update(sceneItemSource, settings);

		req->SendOKResponse();
	}
	else {
		req->SendErrorResponse("specified scene item doesn't exist");
	}
}

/**
* Show or hide a specified source item in a specified scene.
*
* @param {String} `source` Scene item name in the specified scene.
* @param {boolean} `render` true = shown ; false = hidden
* @param {String (optional)} `scene-name` Name of the scene where the source resides. Defaults to the currently active scene.
*
* @api requests
* @name SetSceneItemRender
* @category scene items
* @since 0.3
* @deprecated Since 4.3.0. Prefer the use of SetSceneItemProperties.
*/
void WSRequestHandler::HandleSetSceneItemRender(WSRequestHandler* req) {
	if (!req->hasField("source") ||
		!req->hasField("render"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* itemName = obs_data_get_string(req->data, "source");
	bool isVisible = obs_data_get_bool(req->data, "render");

	if (!itemName) {
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	const char* sceneName = obs_data_get_string(req->data, "scene-name");
	OBSSourceAutoRelease scene = Utils::GetSceneFromNameOrCurrent(sceneName);
	if (!scene) {
		req->SendErrorResponse("requested scene doesn't exist");
		return;
	}

	OBSSceneItemAutoRelease sceneItem =
		Utils::GetSceneItemFromName(scene, itemName);
	if (sceneItem) {
		obs_sceneitem_set_visible(sceneItem, isVisible);
		req->SendOKResponse();
	}
	else {
		req->SendErrorResponse("specified scene item doesn't exist");
	}
}

/**
* Sets the coordinates of a specified source item.
*
* @param {String (optional)} `scene-name` The name of the scene that the source item belongs to. Defaults to the current scene.
* @param {String} `item` The name of the source item.
* @param {double} `x` X coordinate.
* @param {double} `y` Y coordinate.

*
* @api requests
* @name SetSceneItemPosition
* @category scene items
* @since 4.0.0
* @deprecated Since 4.3.0. Prefer the use of SetSceneItemProperties.
*/
void WSRequestHandler::HandleSetSceneItemPosition(WSRequestHandler* req) {
	if (!req->hasField("item") ||
		!req->hasField("x") || !req->hasField("y")) {
		req->SendErrorResponse("missing request parameters");
		return;
	}

	QString itemName = obs_data_get_string(req->data, "item");
	if (itemName.isEmpty()) {
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	QString sceneName = obs_data_get_string(req->data, "scene-name");
	OBSSourceAutoRelease scene = Utils::GetSceneFromNameOrCurrent(sceneName);
	if (!scene) {
		req->SendErrorResponse("requested scene could not be found");
		return;
	}

	OBSSceneItem sceneItem = Utils::GetSceneItemFromName(scene, itemName);
	if (sceneItem) {
		vec2 item_position = { 0 };
		item_position.x = obs_data_get_double(req->data, "x");
		item_position.y = obs_data_get_double(req->data, "y");
		obs_sceneitem_set_pos(sceneItem, &item_position);

		req->SendOKResponse();
	}
	else {
		req->SendErrorResponse("specified scene item doesn't exist");
	}
}

/**
* Set the transform of the specified source item.
*
* @param {String (optional)} `scene-name` The name of the scene that the source item belongs to. Defaults to the current scene.
* @param {String} `item` The name of the source item.
* @param {double} `x-scale` Width scale factor.
* @param {double} `y-scale` Height scale factor.
* @param {double} `rotation` Source item rotation (in degrees).
*
* @api requests
* @name SetSceneItemTransform
* @category scene items
* @since 4.0.0
* @deprecated Since 4.3.0. Prefer the use of SetSceneItemProperties.
*/
void WSRequestHandler::HandleSetSceneItemTransform(WSRequestHandler* req) {
	if (!req->hasField("item") ||
		!req->hasField("x-scale") ||
		!req->hasField("y-scale") ||
		!req->hasField("rotation"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	QString itemName = obs_data_get_string(req->data, "item");
	if (itemName.isEmpty()) {
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	QString sceneName = obs_data_get_string(req->data, "scene-name");
	OBSSourceAutoRelease scene = Utils::GetSceneFromNameOrCurrent(sceneName);
	if (!scene) {
		req->SendErrorResponse("requested scene doesn't exist");
		return;
	}

	vec2 scale;
	scale.x = obs_data_get_double(req->data, "x-scale");
	scale.y = obs_data_get_double(req->data, "y-scale");
	float rotation = obs_data_get_double(req->data, "rotation");

	OBSSceneItemAutoRelease sceneItem = Utils::GetSceneItemFromName(scene, itemName);
	if (sceneItem) {
		obs_sceneitem_set_scale(sceneItem, &scale);
		obs_sceneitem_set_rot(sceneItem, rotation);
		req->SendOKResponse();
	}
	else {
		req->SendErrorResponse("specified scene item doesn't exist");
	}
}

/**
* Sets the crop coordinates of the specified source item.
*
* @param {String (optional)} `scene-name` the name of the scene that the source item belongs to. Defaults to the current scene.
* @param {String} `item` The name of the source.
* @param {int} `top` Pixel position of the top of the source item.
* @param {int} `bottom` Pixel position of the bottom of the source item.
* @param {int} `left` Pixel position of the left of the source item.
* @param {int} `right` Pixel position of the right of the source item.
*
* @api requests
* @name SetSceneItemCrop
* @category scene items
* @since 4.1.0
* @deprecated Since 4.3.0. Prefer the use of SetSceneItemProperties.
*/
void WSRequestHandler::HandleSetSceneItemCrop(WSRequestHandler* req) {
	if (!req->hasField("item")) {
		req->SendErrorResponse("missing request parameters");
		return;
	}

	QString itemName = obs_data_get_string(req->data, "item");
	if (itemName.isEmpty()) {
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	QString sceneName = obs_data_get_string(req->data, "scene-name");
	OBSSourceAutoRelease scene = Utils::GetSceneFromNameOrCurrent(sceneName);
	if (!scene) {
		req->SendErrorResponse("requested scene doesn't exist");
		return;
	}

	OBSSceneItemAutoRelease sceneItem = Utils::GetSceneItemFromName(scene, itemName);
	if (sceneItem) {
		struct obs_sceneitem_crop crop = { 0 };
		crop.top = obs_data_get_int(req->data, "top");
		crop.bottom = obs_data_get_int(req->data, "bottom");
		crop.left = obs_data_get_int(req->data, "left");
		crop.right = obs_data_get_int(req->data, "right");

		obs_sceneitem_set_crop(sceneItem, &crop);

		req->SendOKResponse();
	}
	else {
		req->SendErrorResponse("specified scene item doesn't exist");
	}
}

/**
 * Deletes a scene item.
 *
 * @param {String (optional)} `scene` Name of the scene the source belongs to. Defaults to the current scene.
 * @param {Object} `item` item to delete (required)
 * @param {String} `item.name` name of the scene item (prefer `id`, including both is acceptable).
 * @param {int} `item.id` id of the scene item.
 *
 * @api requests
 * @name DeleteSceneItem
 * @category scene items
 * @since 4.5.0
 */
void WSRequestHandler::HandleDeleteSceneItem(WSRequestHandler* req) {
	if (!req->hasField("item")) {
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* sceneName = obs_data_get_string(req->data, "scene");
	OBSSourceAutoRelease scene = Utils::GetSceneFromNameOrCurrent(sceneName);
	if (!scene) {
		req->SendErrorResponse("requested scene doesn't exist");
		return;
	}

	OBSDataAutoRelease item = obs_data_get_obj(req->data, "item");
	OBSSceneItemAutoRelease sceneItem = Utils::GetSceneItemFromItem(scene, item);
	if (!sceneItem) {
		req->SendErrorResponse("item with id/name combination not found in specified scene");
		return;
	}

	obs_sceneitem_remove(sceneItem);

	req->SendOKResponse();
}

struct DuplicateSceneItemData {
	obs_sceneitem_t *referenceItem;
	obs_source_t *fromSource;
	obs_sceneitem_t *newItem;
};

static void DuplicateSceneItem(void *_data, obs_scene_t *scene) {
	DuplicateSceneItemData *data = (DuplicateSceneItemData *)_data;
	data->newItem = obs_scene_add(scene, data->fromSource);
	obs_sceneitem_set_visible(data->newItem, obs_sceneitem_visible(data->referenceItem));
}

/**
 * Duplicates a scene item.
 *
 * @param {String (optional)} `fromScene` Name of the scene to copy the item from. Defaults to the current scene.
 * @param {String (optional)} `toScene` Name of the scene to create the item in. Defaults to the current scene.
 * @param {Object} `item` item to duplicate (required)
 * @param {String} `item.name` name of the scene item (prefer `id`, including both is acceptable).
 * @param {int} `item.id` id of the scene item.
 *
 * @return {String} `scene` Name of the scene where the new item was created
 * @return {Object} `item` New item info
 * @return {int} `̀item.id` New item ID
 * @return {String} `item.name` New item name
 * 
 * @api requests
 * @name DuplicateSceneItem
 * @category scene items
 * @since 4.5.0
 */
void WSRequestHandler::HandleDuplicateSceneItem(WSRequestHandler* req) {
	if (!req->hasField("item")) {
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* fromSceneName = obs_data_get_string(req->data, "fromScene");
	OBSSourceAutoRelease fromScene = Utils::GetSceneFromNameOrCurrent(fromSceneName);
	if (!fromScene) {
		req->SendErrorResponse("requested fromScene doesn't exist");
		return;
	}

	const char* toSceneName = obs_data_get_string(req->data, "toScene");
	OBSSourceAutoRelease toScene = Utils::GetSceneFromNameOrCurrent(toSceneName);
	if (!toScene) {
		req->SendErrorResponse("requested toScene doesn't exist");
		return;
	}

	OBSDataAutoRelease item = obs_data_get_obj(req->data, "item");
	OBSSceneItemAutoRelease referenceItem = Utils::GetSceneItemFromItem(fromScene, item);
	if (!referenceItem) {
		req->SendErrorResponse("item with id/name combination not found in specified scene");
		return;
	}

	DuplicateSceneItemData data;
	data.fromSource = obs_sceneitem_get_source(referenceItem);
	data.referenceItem = referenceItem;

	obs_enter_graphics();
	obs_scene_atomic_update(obs_scene_from_source(toScene), DuplicateSceneItem, &data);
	obs_leave_graphics();

	obs_sceneitem_t *newItem = data.newItem;
	if (!newItem) {
		req->SendErrorResponse("Error duplicating scene item");
		return;
	}

	OBSDataAutoRelease itemData = obs_data_create();
	obs_data_set_int(itemData, "id", obs_sceneitem_get_id(newItem));
	obs_data_set_string(itemData, "name", obs_source_get_name(obs_sceneitem_get_source(newItem)));

	OBSDataAutoRelease responseData = obs_data_create();
	obs_data_set_obj(responseData, "item", itemData);
	obs_data_set_string(responseData, "scene", obs_source_get_name(toScene));

	req->SendOKResponse(responseData);
}