#include <QString>
#include "src/Utils.h"

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
 void WSRequestHandler::HandleSetCurrentSceneCollection(WSRequestHandler* req) {
    if (!req->hasField("sc-name")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    QString sceneCollection = obs_data_get_string(req->data, "sc-name");
    if (!sceneCollection.isEmpty()) {
        // TODO : Check if specified profile exists and if changing is allowed
        obs_frontend_set_current_scene_collection(sceneCollection.toUtf8());
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("invalid request parameters");
    }
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
void WSRequestHandler::HandleGetCurrentSceneCollection(WSRequestHandler* req) {
    OBSDataAutoRelease response = obs_data_create();
    obs_data_set_string(response, "sc-name",
        obs_frontend_get_current_scene_collection());

    req->SendOKResponse(response);
}

/**
 * List available scene collections
 *
 * @return {Object|Array} `scene-collections` Scene collections list
 * @return {String} `scene-collections.*.`
 *
 * @api requests
 * @name ListSceneCollections
 * @category scene collections
 * @since 4.0.0
 */
void WSRequestHandler::HandleListSceneCollections(WSRequestHandler* req) {
    char** sceneCollections = obs_frontend_get_scene_collections();
    OBSDataArrayAutoRelease list =
        Utils::StringListToArray(sceneCollections, "sc-name");
    bfree(sceneCollections);

    OBSDataAutoRelease response = obs_data_create();
    obs_data_set_array(response, "scene-collections", list);

    req->SendOKResponse(response);
}
