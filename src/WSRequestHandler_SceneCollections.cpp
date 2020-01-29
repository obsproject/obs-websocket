#include "Utils.h"

#include "WSRequestHandler.h"

/**
 * Change the active scene collection.
 *
 * @param {String} `sc-name` Name of the desired scene collection.
 *
 * @api requests
 * @name SetCurrentSceneCollection
 * @category scene collections
 * @since 4.0.0
 */
RpcResponse WSRequestHandler::SetCurrentSceneCollection(const RpcRequest& request) {
	if (!request.hasField("sc-name")) {
		return request.failed("missing request parameters");
	}

	QString sceneCollection = obs_data_get_string(request.parameters(), "sc-name");
	if (sceneCollection.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	// TODO : Check if specified profile exists and if changing is allowed
	obs_frontend_set_current_scene_collection(sceneCollection.toUtf8());
	return request.success();
}

/**
 * Get the name of the current scene collection.
 *
 * @return {String} `sc-name` Name of the currently active scene collection.
 *
 * @api requests
 * @name GetCurrentSceneCollection
 * @category scene collections
 * @since 4.0.0
 */
RpcResponse WSRequestHandler::GetCurrentSceneCollection(const RpcRequest& request) {
	OBSDataAutoRelease response = obs_data_create();

	char* sceneCollection = obs_frontend_get_current_scene_collection();
	obs_data_set_string(response, "sc-name", sceneCollection);
	bfree(sceneCollection);

	return request.success(response);
}

/**
 * List available scene collections
 *
 * @return {Array<String>} `scene-collections` Scene collections list
 *
 * @api requests
 * @name ListSceneCollections
 * @category scene collections
 * @since 4.0.0
 */
RpcResponse WSRequestHandler::ListSceneCollections(const RpcRequest& request) {
	char** sceneCollections = obs_frontend_get_scene_collections();
	OBSDataArrayAutoRelease list =
		Utils::StringListToArray(sceneCollections, "sc-name");
	bfree(sceneCollections);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_array(response, "scene-collections", list);

	return request.success(response);
}
