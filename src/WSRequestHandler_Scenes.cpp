#include <QString>
#include "Utils.h"

#include "WSRequestHandler.h"

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
 void WSRequestHandler::HandleSetCurrentScene(WSRequestHandler* req) {
    if (!req->hasField("scene-name")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* sceneName = obs_data_get_string(req->data, "scene-name");
    OBSSourceAutoRelease source = obs_get_source_by_name(sceneName);

    if (source) {
        obs_frontend_set_current_scene(source);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("requested scene does not exist");
    }
}

/**
 * Get the current scene's name and source items.
 * 
 * @return {String} `name` Name of the currently active scene.
 * @return {Source|Array} `sources` Ordered list of the current scene's source items.
 *
 * @api requests
 * @name GetCurrentScene
 * @category scenes
 * @since 0.3
 */
void WSRequestHandler::HandleGetCurrentScene(WSRequestHandler* req) {
    OBSSourceAutoRelease currentScene = obs_frontend_get_current_scene();
    OBSDataArrayAutoRelease sceneItems = Utils::GetSceneItems(currentScene);

    OBSDataAutoRelease data = obs_data_create();
    obs_data_set_string(data, "name", obs_source_get_name(currentScene));
    obs_data_set_array(data, "sources", sceneItems);

    req->SendOKResponse(data);
}

/**
 * Get a list of scenes in the currently active profile.
 * 
 * @return {String} `current-scene` Name of the currently active scene.
 * @return {Scene|Array} `scenes` Ordered list of the current profile's scenes (See `[GetCurrentScene](#getcurrentscene)` for more information).
 *
 * @api requests
 * @name GetSceneList
 * @category scenes
 * @since 0.3
 */
void WSRequestHandler::HandleGetSceneList(WSRequestHandler* req) {
    OBSSourceAutoRelease currentScene = obs_frontend_get_current_scene();
    OBSDataArrayAutoRelease scenes = Utils::GetScenes();

    OBSDataAutoRelease data = obs_data_create();
    obs_data_set_string(data, "current-scene",
        obs_source_get_name(currentScene));
    obs_data_set_array(data, "scenes", scenes);

    req->SendOKResponse(data);
}

/**
* Changes the order of scene items in the requested scene.
*
* @param {String (optional)} `scene` Name of the scene to reorder (defaults to current).
* @param {Scene|Array} `items` Ordered list of objects with name and/or id specified. Id prefered due to uniqueness per scene
* @param {int (optional)} `items[].id` Id of a specific scene item. Unique on a scene by scene basis.
* @param {String (optional)} `items[].name` Name of a scene item. Sufficiently unique if no scene items share sources within the scene.
*
* @api requests
* @name SetSceneItemOrder
* @category scenes
* @since unreleased
*/
void WSRequestHandler::HandleSetSceneItemOrder(WSRequestHandler* req) {
    QString sceneName = obs_data_get_string(req->data, "scene");
    OBSSourceAutoRelease scene = Utils::GetSceneFromNameOrCurrent(sceneName);
    if (!scene) {
        req->SendErrorResponse("requested scene doesn't exist");
        return;
    }

    OBSDataArrayAutoRelease items = obs_data_get_array(req->data, "items");
    if (!items) {
        req->SendErrorResponse("sceneItem order not specified");
        return;
    }

    size_t count = obs_data_array_count(items);

    std::vector<obs_sceneitem_t*> newOrder;
    newOrder.reserve(count);
    for (size_t i = 0; i < count; i++) {
        OBSDataAutoRelease item = obs_data_array_item(items, i);
        OBSSceneItemAutoRelease sceneItem = Utils::GetSceneItemFromItem(scene, item);
        if (!sceneItem) {
            req->SendErrorResponse("Invalid sceneItem id or name in order");
            return;
        }
        for (size_t j = 0; j < i; j++) {
            if (sceneItem == newOrder[j]) {
                req->SendErrorResponse("Duplicate sceneItem in specified order");
                for (size_t i = 0; i < count; i++) {
                    obs_sceneitem_release(newOrder[i]);
                }
                return;
            }
        }
        newOrder.push_back(sceneItem);
    }

    if (obs_scene_reorder_items(obs_scene_from_source(scene), newOrder.data(), count)) {
        req->SendOKResponse();
    }
    else {
        req->SendErrorResponse("Invalid sceneItem order");
    }
    for (size_t i = 0; i < count; i++) {
        obs_sceneitem_release(newOrder[i]);
    }
}
