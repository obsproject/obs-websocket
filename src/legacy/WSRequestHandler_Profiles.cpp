#include <QString>
#include "src/Utils.h"

#include "WSRequestHandler.h"

/**
 * Set the currently active profile.
 * 
 * @param {String} `profile-name` Name of the desired profile.
 *
 * @api requests
 * @name SetCurrentProfile
 * @category profiles
 * @since 4.0.0
 */
 void WSRequestHandler::HandleSetCurrentProfile(WSRequestHandler* req) {
    if (!req->hasField("profile-name")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    QString profileName = obs_data_get_string(req->data, "profile-name");
    if (!profileName.isEmpty()) {
        // TODO : check if profile exists
        obs_frontend_set_current_profile(profileName.toUtf8());
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("invalid request parameters");
    }
}

 /**
 * Get the name of the current profile.
 * 
 * @return {String} `profile-name` Name of the currently active profile.
 *
 * @api requests
 * @name GetCurrentProfile
 * @category profiles
 * @since 4.0.0
 */
void WSRequestHandler::HandleGetCurrentProfile(WSRequestHandler* req) {
    OBSDataAutoRelease response = obs_data_create();
    obs_data_set_string(response, "profile-name",
        obs_frontend_get_current_profile());

    req->SendOKResponse(response);
}

/**
 * Get a list of available profiles.
 *
 * @return {Object|Array} `profiles` List of available profiles.
 *
 * @api requests
 * @name ListProfiles
 * @category profiles
 * @since 4.0.0
 */
void WSRequestHandler::HandleListProfiles(WSRequestHandler* req) {
    char** profiles = obs_frontend_get_profiles();
    OBSDataArrayAutoRelease list =
        Utils::StringListToArray(profiles, "profile-name");
    bfree(profiles);

    OBSDataAutoRelease response = obs_data_create();
    obs_data_set_array(response, "profiles", list);

    req->SendOKResponse(response);
}
