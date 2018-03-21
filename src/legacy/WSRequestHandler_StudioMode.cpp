#include <QString>
#include "src/Utils.h"

#include "WSRequestHandler.h"

/**
 * Indicates if Studio Mode is currently enabled.
 *
 * @return {boolean} `studio-mode` Indicates if Studio Mode is enabled.
 *
 * @api requests
 * @name GetStudioModeStatus
 * @category studio mode
 * @since 4.1.0
 */
 void WSRequestHandler::HandleGetStudioModeStatus(WSRequestHandler* req) {
    bool previewActive = obs_frontend_preview_program_mode_active();

    OBSDataAutoRelease response = obs_data_create();
    obs_data_set_bool(response, "studio-mode", previewActive);

    req->SendOKResponse(response);
}

/**
 * Get the name of the currently previewed scene and its list of sources.
 * Will return an `error` if Studio Mode is not enabled.
 *
 * @return {String} `name` The name of the active preview scene.
 * @return {Source|Array} `sources`
 *
 * @api requests
 * @name GetPreviewScene
 * @category studio mode
 * @since 4.1.0
 */
void WSRequestHandler::HandleGetPreviewScene(WSRequestHandler* req) {
    if (!obs_frontend_preview_program_mode_active()) {
        req->SendErrorResponse("studio mode not enabled");
        return;
    }

    OBSSourceAutoRelease scene = obs_frontend_get_current_preview_scene();
    OBSDataArrayAutoRelease sceneItems = Utils::GetSceneItems(scene);

    OBSDataAutoRelease data = obs_data_create();
    obs_data_set_string(data, "name", obs_source_get_name(scene));
    obs_data_set_array(data, "sources", sceneItems);

    req->SendOKResponse(data);
}

/**
 * Set the active preview scene.
 * Will return an `error` if Studio Mode is not enabled.
 *
 * @param {String} `scene-name` The name of the scene to preview.
 *
 * @api requests
 * @name SetPreviewScene
 * @category studio mode
 * @since 4.1.0
 */
void WSRequestHandler::HandleSetPreviewScene(WSRequestHandler* req) {
    if (!obs_frontend_preview_program_mode_active()) {
        req->SendErrorResponse("studio mode not enabled");
        return;
    }

    if (!req->hasField("scene-name")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* scene_name = obs_data_get_string(req->data, "scene-name");
    OBSSourceAutoRelease scene = Utils::GetSceneFromNameOrCurrent(scene_name);

    if (scene) {
        obs_frontend_set_current_preview_scene(scene);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("specified scene doesn't exist");
    }
}

/**
 * Transitions the currently previewed scene to the main output.
 * Will return an `error` if Studio Mode is not enabled.
 *
 * @param {Object (optional)} `with-transition` Change the active transition before switching scenes. Defaults to the active transition. 
 * @param {String} `with-transition.name` Name of the transition.
 * @param {int (optional)} `with-transition.duration` Transition duration (in milliseconds).
 *
 * @api requests
 * @name TransitionToProgram
 * @category studio mode
 * @since 4.1.0
 */
void WSRequestHandler::HandleTransitionToProgram(WSRequestHandler* req) {
    if (!obs_frontend_preview_program_mode_active()) {
        req->SendErrorResponse("studio mode not enabled");
        return;
    }

    if (req->hasField("with-transition")) {
        OBSDataAutoRelease transitionInfo =
            obs_data_get_obj(req->data, "with-transition");

        if (obs_data_has_user_value(transitionInfo, "name")) {
            QString transitionName =
                obs_data_get_string(transitionInfo, "name");
            if (transitionName.isEmpty()) {
                req->SendErrorResponse("invalid request parameters");
                return;
            }

            bool success = Utils::SetTransitionByName(transitionName);
            if (!success) {
                req->SendErrorResponse("specified transition doesn't exist");
                return;
            }
        }

        if (obs_data_has_user_value(transitionInfo, "duration")) {
            int transitionDuration =
                obs_data_get_int(transitionInfo, "duration");
            Utils::SetTransitionDuration(transitionDuration);
        }
    }

    Utils::TransitionToProgram();
    req->SendOKResponse();
}

/**
 * Enables Studio Mode.
 *
 * @api requests
 * @name EnableStudioMode
 * @category studio mode
 * @since 4.1.0
 */
void WSRequestHandler::HandleEnableStudioMode(WSRequestHandler* req) {
    obs_frontend_set_preview_program_mode(true);
    req->SendOKResponse();
}

/**
 * Disables Studio Mode.
 *
 * @api requests
 * @name DisableStudioMode
 * @category studio mode
 * @since 4.1.0
 */
void WSRequestHandler::HandleDisableStudioMode(WSRequestHandler* req) {
    obs_frontend_set_preview_program_mode(false);
    req->SendOKResponse();
}

/**
 * Toggles Studio Mode.
 *
 * @api requests
 * @name ToggleStudioMode
 * @category studio mode
 * @since 4.1.0
 */
void WSRequestHandler::HandleToggleStudioMode(WSRequestHandler* req) {
    bool previewProgramMode = obs_frontend_preview_program_mode_active();
    obs_frontend_set_preview_program_mode(!previewProgramMode);
    req->SendOKResponse();
}
