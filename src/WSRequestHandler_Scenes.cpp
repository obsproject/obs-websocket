#include "Utils.h"

#include "WSRequestHandler.h"

/**
* @typedef {Object} `Scene`
* @property {String} `name` Name of the currently active scene.
* @property {Array<SceneItem>} `sources` Ordered list of the current scene's source items.
*/

/**
 * Switch to the specified scene.
 *
 * @param {String} `scene-name` Name of the scene to switch to.
 *
 * @api requests
 * @name SetCurrentScene
 * @category scenes
 * @since 0.3
 */
RpcResponse WSRequestHandler::SetCurrentScene(const RpcRequest& request) {
	if (!request.hasField("scene-name")) {
		return request.failed("missing request parameters");
	}

	const char* sceneName = obs_data_get_string(request.parameters(), "scene-name");
	OBSSourceAutoRelease source = obs_get_source_by_name(sceneName);

	if (source) {
		obs_frontend_set_current_scene(source);
		return request.success();
	} else {
		return request.failed("requested scene does not exist");
	}
}

/**
 * Get the current scene's name and source items.
 *
 * @return {String} `name` Name of the currently active scene.
 * @return {Array<SceneItem>} `sources` Ordered list of the current scene's source items.
 *
 * @api requests
 * @name GetCurrentScene
 * @category scenes
 * @since 0.3
 */
RpcResponse WSRequestHandler::GetCurrentScene(const RpcRequest& request) {
	OBSSourceAutoRelease currentScene = obs_frontend_get_current_scene();
	OBSDataArrayAutoRelease sceneItems = Utils::GetSceneItems(currentScene);

	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_string(data, "name", obs_source_get_name(currentScene));
	obs_data_set_array(data, "sources", sceneItems);

	return request.success(data);
}

/**
 * Get a list of scenes in the currently active profile.
 *
 * @return {String} `current-scene` Name of the currently active scene.
 * @return {Array<Scene>} `scenes` Ordered list of the current profile's scenes (See [GetCurrentScene](#getcurrentscene) for more information).
 *
 * @api requests
 * @name GetSceneList
 * @category scenes
 * @since 0.3
 */
RpcResponse WSRequestHandler::GetSceneList(const RpcRequest& request) {
	OBSSourceAutoRelease currentScene = obs_frontend_get_current_scene();
	OBSDataArrayAutoRelease scenes = Utils::GetScenes();

	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_string(data, "current-scene",
		obs_source_get_name(currentScene));
	obs_data_set_array(data, "scenes", scenes);

	return request.success(data);
}

/**
 * Create a new scene scene.
 *
 * @param {String} `sceneName` Name of the scene to create.
 *
 * @api requests
 * @name CreateScene
 * @category scenes
 * @since 4.8.0
 */
RpcResponse WSRequestHandler::CreateScene(const RpcRequest& request) {
	if (!request.hasField("sceneName")) {
		return request.failed("missing request parameters");
	}

	const char* sceneName = obs_data_get_string(request.parameters(), "sceneName");
	OBSSourceAutoRelease source = obs_get_source_by_name(sceneName);

	if (source) {
		return request.failed("scene with this name already exists");
	}
	obs_scene_create(sceneName);
	return request.success();
}

/**
* Changes the order of scene items in the requested scene.
*
* @param {String (optional)} `scene` Name of the scene to reorder (defaults to current).
* @param {Array<Scene>} `items` Ordered list of objects with name and/or id specified. Id preferred due to uniqueness per scene
* @param {int (optional)} `items.*.id` Id of a specific scene item. Unique on a scene by scene basis.
* @param {String (optional)} `items.*.name` Name of a scene item. Sufficiently unique if no scene items share sources within the scene.
*
* @api requests
* @name ReorderSceneItems
* @category scenes
* @since 4.5.0
*/
RpcResponse WSRequestHandler::ReorderSceneItems(const RpcRequest& request) {
	QString sceneName = obs_data_get_string(request.parameters(), "scene");
	OBSScene scene = Utils::GetSceneFromNameOrCurrent(sceneName);
	if (!scene) {
		return request.failed("requested scene doesn't exist");
	}

	OBSDataArrayAutoRelease items = obs_data_get_array(request.parameters(), "items");
	if (!items) {
		return request.failed("sceneItem order not specified");
	}

	struct reorder_context {
		obs_data_array_t* items;
		bool success;
		QString errorMessage;
	};

	struct reorder_context ctx;
	ctx.success = false;
	ctx.items = items;

	obs_scene_atomic_update(scene, [](void* param, obs_scene_t* scene) {
		auto ctx = reinterpret_cast<struct reorder_context*>(param);

		QVector<struct obs_sceneitem_order_info> orderList;
		struct obs_sceneitem_order_info info;

		size_t itemCount = obs_data_array_count(ctx->items);
		for (uint i = 0; i < itemCount; i++) {
			OBSDataAutoRelease item = obs_data_array_item(ctx->items, i);

			OBSSceneItemAutoRelease sceneItem = Utils::GetSceneItemFromItem(scene, item);
			if (!sceneItem) {
				ctx->success = false;
				ctx->errorMessage = "Invalid sceneItem id or name specified";
				return;
			}

			info.group = nullptr;
			info.item = sceneItem;
			orderList.insert(0, info);
		}

		ctx->success = obs_scene_reorder_items2(scene, orderList.data(), orderList.size());
		if (!ctx->success) {
			ctx->errorMessage = "Invalid sceneItem order";
		}
	}, &ctx);

	if (!ctx.success) {
		return request.failed(ctx.errorMessage);
	}

	return request.success();
}

/**
 * Set a scene to use a specific transition override.
 *
 * @param {String} `sceneName` Name of the scene to switch to.
 * @param {String} `transitionName` Name of the transition to use.
 * @param {int (Optional)} `transitionDuration` Duration in milliseconds of the transition if transition is not fixed. Defaults to the current duration specified in the UI if there is no current override and this value is not given.
 *
 * @api requests
 * @name SetSceneTransitionOverride
 * @category scenes
 * @since 4.8.0
 */
RpcResponse WSRequestHandler::SetSceneTransitionOverride(const RpcRequest& request) {
	if (!request.hasField("sceneName") || !request.hasField("transitionName")) {
		return request.failed("missing request parameters");
	}

	QString sceneName = obs_data_get_string(request.parameters(), "sceneName");
	OBSSourceAutoRelease source = obs_get_source_by_name(sceneName.toUtf8());
	if (!source) {
		return request.failed("requested scene does not exist");
	}

	enum obs_source_type sourceType = obs_source_get_type(source);
	if (sourceType != OBS_SOURCE_TYPE_SCENE) {
		return request.failed("requested scene is invalid");
	}

	QString transitionName = obs_data_get_string(request.parameters(), "transitionName");
	if (!Utils::GetTransitionFromName(transitionName)) {
		return request.failed("requested transition does not exist");
	}

	OBSDataAutoRelease sourceData = obs_source_get_private_settings(source);
	obs_data_set_string(sourceData, "transition", transitionName.toUtf8().constData());

	if (request.hasField("transitionDuration")) {
		int transitionOverrideDuration = obs_data_get_int(request.parameters(), "transitionDuration");
		obs_data_set_int(sourceData, "transition_duration", transitionOverrideDuration);
	} else if(!obs_data_has_user_value(sourceData, "transition_duration")) {
		obs_data_set_int(sourceData, "transition_duration",
			obs_frontend_get_transition_duration()
		);
	}

	return request.success();
}

/**
 * Remove any transition override on a scene.
 *
 * @param {String} `sceneName` Name of the scene to switch to.
 *
 * @api requests
 * @name RemoveSceneTransitionOverride
 * @category scenes
 * @since 4.8.0
 */
RpcResponse WSRequestHandler::RemoveSceneTransitionOverride(const RpcRequest& request) {
	if (!request.hasField("sceneName")) {
		return request.failed("missing request parameters");
	}

	QString sceneName = obs_data_get_string(request.parameters(), "sceneName");
	OBSSourceAutoRelease source = obs_get_source_by_name(sceneName.toUtf8());
	if (!source) {
		return request.failed("requested scene does not exist");
	}

	enum obs_source_type sourceType = obs_source_get_type(source);
	if (sourceType != OBS_SOURCE_TYPE_SCENE) {
		return request.failed("requested scene is invalid");
	}

	OBSDataAutoRelease sourceData = obs_source_get_private_settings(source);
	obs_data_erase(sourceData, "transition");
	obs_data_erase(sourceData, "transition_duration");

	return request.success();
}

/**
 * Get the current scene transition override.
 *
 * @param {String} `sceneName` Name of the scene to switch to.
 *
 * @return {String} `transitionName` Name of the current overriding transition. Empty string if no override is set.
 * @return {int} `transitionDuration` Transition duration. `-1` if no override is set.
 *
 * @api requests
 * @name GetSceneTransitionOverride
 * @category scenes
 * @since 4.8.0
 */
RpcResponse WSRequestHandler::GetSceneTransitionOverride(const RpcRequest& request) {
	if (!request.hasField("sceneName")) {
		return request.failed("missing request parameters");
	}

	QString sceneName = obs_data_get_string(request.parameters(), "sceneName");
	OBSSourceAutoRelease source = obs_get_source_by_name(sceneName.toUtf8());
	if (!source) {
		return request.failed("requested scene does not exist");
	}

	enum obs_source_type sourceType = obs_source_get_type(source);
	if (sourceType != OBS_SOURCE_TYPE_SCENE) {
		return request.failed("requested scene is invalid");
	}

	OBSDataAutoRelease sourceData = obs_source_get_private_settings(source);
	const char* transitionOverrideName = obs_data_get_string(sourceData, "transition");

	bool hasDurationOverride = obs_data_has_user_value(sourceData, "transition_duration");
	int transitionOverrideDuration = obs_data_get_int(sourceData, "transition_duration");

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "transitionName", transitionOverrideName);
	obs_data_set_int(fields, "transitionDuration",
		(hasDurationOverride ? transitionOverrideDuration : -1)
	);

	return request.success(fields);
}
