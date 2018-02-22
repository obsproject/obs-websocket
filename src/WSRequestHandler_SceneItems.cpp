#include <QString>
#include "Utils.h"

#include "WSRequestHandler.h"

/**
* Gets the scene specific properties of the specified source item.
*
* @param {String (optional)} `scene` the name of the scene that the source item belongs to. Defaults to the current scene.
* @param {String} `item.id` The name of the source.
* @param {String} `item.name` The name of the source.
*
* @return {String} `scene` The name of the scene.
* @return {String} `item.name` The name of the source.
* @return {String} `item.id` The id of the scene item.
* @return {int} `item.position.x` The x position of the source from the left.
* @return {int} `item.position.y` The y position of the source from the top.
* @return {int} `item.position.alignment` The point on the source that the item is manipulated from.
* @return {double} `item.rotation` The clockwise rotation of the item in degrees around the point of alignment.
* @return {double} `item.scale.x` The x-scale factor of the source.
* @return {double} `item.scale.y` The y-scale factor of the source.
* @return {int} `item.crop.top` The number of pixels cropped off the top of the source before scaling.
* @return {int} `item.crop.right` The number of pixels cropped off the right of the source before scaling.
* @return {int} `item.crop.bottom` The number of pixels cropped off the bottom of the source before scaling.
* @return {int} `item.crop.left` The number of pixels cropped off the left of the source before scaling.
* @return {bool} `item.visible` If the source is visible.
* @return {bool} `item.locked` If the source is locked.
* @return {String} `item.bounds.type` Type of bounding box.
* @return {int} `item.bounds.alignment` Alignment of the bounding box.
* @return {double} `item.bounds.x` Width of the bounding box.
* @return {double} `item.bounds.y` Height of the bounding box.
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
    OBSDataAutoRelease *item = (OBSDataAutoRelease *)obs_data_get_obj(req->data, "item");
    if (!item) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    QString sceneName = obs_data_get_string(req->data, "scene");
    OBSSourceAutoRelease scene = Utils::GetSceneFromNameOrCurrent(sceneName);
    if (!scene) {
        req->SendErrorResponse("requested scene doesn't exist");
        return;
    }

    OBSSceneItemAutoRelease sceneItem = Utils::GetSceneItemFromItem(scene, (obs_data_t *)item);

    if (!sceneItem) {
        req->SendErrorResponse("specified scene item doesn't exist");
        return;
    }

    OBSDataAutoRelease resData = obs_data_create();
    OBSDataAutoRelease resItem = obs_data_create();
    obs_data_set_string(resItem, "name", obs_source_get_name(obs_sceneitem_get_source((obs_sceneitem_t *)sceneItem)));

    OBSDataAutoRelease posData = obs_data_create();
    vec2 pos;
    obs_sceneitem_get_pos(sceneItem, &pos);
    obs_data_set_double(posData, "x", pos.x);
    obs_data_set_double(posData, "y", pos.y);
    obs_data_set_int(posData, "alignment", obs_sceneitem_get_alignment(sceneItem));
    obs_data_set_obj(resItem, "position", posData);

    obs_data_set_double(resItem, "rotation", obs_sceneitem_get_rot(sceneItem));

    OBSDataAutoRelease scaleData = obs_data_create();
    vec2 scale;
    obs_sceneitem_get_scale(sceneItem, &scale);
    obs_data_set_double(scaleData, "x", scale.x);
    obs_data_set_double(scaleData, "y", scale.y);
    obs_data_set_obj(resItem, "scale", scaleData);

    OBSDataAutoRelease cropData = obs_data_create();
    obs_sceneitem_crop crop;
    obs_sceneitem_get_crop(sceneItem, &crop);
    obs_data_set_int(cropData, "left", crop.left);
    obs_data_set_int(cropData, "top", crop.top);
    obs_data_set_int(cropData, "right", crop.right);
    obs_data_set_int(cropData, "bottom", crop.bottom);
    obs_data_set_obj(resItem, "crop", cropData);

    obs_data_set_bool(resItem, "visible", obs_sceneitem_visible(sceneItem));

    obs_data_set_bool(resItem, "locked", obs_sceneitem_locked(sceneItem));

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
    obs_data_set_obj(resItem, "bounds", boundsData);
    obs_data_set_obj(resData, "item", resItem);
    obs_data_set_string(resData, "scene", obs_source_get_name(scene));
    req->SendOKResponse(resData);
}

/**
* Sets the scene specific properties of a source. Unspecified properties will remain unchanged.
*
* @param {String (optional)} `scene` the name of the scene that the source item belongs to. Defaults to the current scene.
* @param {String} `item.name` The name of the item.
* @param {int} `item.id` The id of the item.
* @param {int} `item.position.x` The new x position of the item.
* @param {int} `item.position.y` The new y position of the item.
* @param {int} `item.position.alignment` The new alignment of the item.
* @param {double} `item.rotation` The new clockwise rotation of the item in degrees.
* @param {double} `item.scale.x` The new x scale of the item.
* @param {double} `item.scale.y` The new y scale of the item.
* @param {int} `item.crop.top` The new amount of pixels cropped off the top of the source before scaling.
* @param {int} `item.crop.bottom` The new amount of pixels cropped off the bottom of the source before scaling.
* @param {int} `item.crop.left` The new amount of pixels cropped off the left of the source before scaling.
* @param {int} `item.crop.right` The new amount of pixels cropped off the right of the source before scaling.
* @param {bool} `item.visible` The new visibility of the item. 'true' shows source, 'false' hides source.
* @param {bool} `item.locked` The new locked of the item. 'true' is locked, 'false' is unlocked.
* @param {String} `item.bounds.type` The new bounds type of the item.
* @param {int} `item.bounds.alignment` The new alignment of the bounding box. (0-2, 4-6, 8-10)
* @param {double} `item.bounds.x` The new width of the bounding box.
* @param {double} `item.bounds.y` The new height of the bounding box.
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

	OBSDataAutoRelease reqItem = obs_data_get_obj(req->data, "item");
	if (!reqItem) {
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	QString sceneName = obs_data_get_string(req->data, "scene");
	OBSSourceAutoRelease scene = Utils::GetSceneFromNameOrCurrent(sceneName);
	if (!scene) {
		req->SendErrorResponse("requested scene doesn't exist");
		return;
	}

	OBSSceneItemAutoRelease sceneItem = Utils::GetSceneItemFromItem(scene, reqItem);
	if (!sceneItem) {
		req->SendErrorResponse("specified scene item doesn't exist");
		return;
	}

	bool badRequest = false;
	OBSDataAutoRelease errorMessage = obs_data_create();

	if (obs_data_has_user_value(reqItem, "position")) {
		vec2 oldPosition;
		OBSDataAutoRelease positionError = obs_data_create();
		obs_sceneitem_get_pos(sceneItem, &oldPosition);
		OBSDataAutoRelease reqPosition = obs_data_get_obj(reqItem, "position");
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

  if (obs_data_has_user_value(reqItem, "rotation")) {
		obs_sceneitem_set_rot(sceneItem, (float)obs_data_get_double(reqItem, "rotation"));
	}

  if (obs_data_has_user_value(reqItem, "scale")) {
		vec2 oldScale;
		obs_sceneitem_get_scale(sceneItem, &oldScale);
		OBSDataAutoRelease reqScale = obs_data_get_obj(reqItem, "scale");
		vec2 newScale = oldScale;
		if (obs_data_has_user_value(reqScale, "x")) {
			newScale.x = obs_data_get_double(reqScale, "x");
		}
		if (obs_data_has_user_value(reqScale, "y")) {
			newScale.y = obs_data_get_double(reqScale, "y");
		}
		obs_sceneitem_set_scale(sceneItem, &newScale);
	}

  if (obs_data_has_user_value(reqItem, "crop")) {
		obs_sceneitem_crop oldCrop;
		obs_sceneitem_get_crop(sceneItem, &oldCrop);
		OBSDataAutoRelease reqCrop = obs_data_get_obj(reqItem, "crop");
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

  if (obs_data_has_user_value(reqItem, "visible")) {
    obs_sceneitem_set_visible(sceneItem, obs_data_get_bool(reqItem, "visible"));
  }

  if (obs_data_has_user_value(reqItem, "locked")) {
    obs_sceneitem_set_locked(sceneItem, obs_data_get_bool(reqItem, "locked"));
  }

  if (obs_data_has_user_value(reqItem, "bounds")) {
		bool badBounds = false;
		OBSDataAutoRelease boundsError = obs_data_create();
		OBSDataAutoRelease reqBounds = obs_data_get_obj(reqItem, "bounds");
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
* @param {String (optional)} `scene-name` Name of the scene the source belogns to. Defaults to the current scene.
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
