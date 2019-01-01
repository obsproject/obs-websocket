#include <QString>
#include "Utils.h"

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
HandlerResponse WSRequestHandler::HandleSetCurrentProfile(WSRequestHandler* req) {
	if (!req->hasField("profile-name")) {
		return req->SendErrorResponse("missing request parameters");
	}

	QString profileName = obs_data_get_string(req->data, "profile-name");
	if (profileName.isEmpty()) {
		return req->SendErrorResponse("invalid request parameters");
	}

	// TODO : check if profile exists
	obs_frontend_set_current_profile(profileName.toUtf8());
	return req->SendOKResponse();
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
HandlerResponse WSRequestHandler::HandleGetCurrentProfile(WSRequestHandler* req) {
	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "profile-name", obs_frontend_get_current_profile());
	return req->SendOKResponse(response);
}

/**
 * Get a list of available profiles.
 *
 * @return {Array<Object>} `profiles` List of available profiles.
 *
 * @api requests
 * @name ListProfiles
 * @category profiles
 * @since 4.0.0
 */
HandlerResponse WSRequestHandler::HandleListProfiles(WSRequestHandler* req) {
	char** profiles = obs_frontend_get_profiles();
	OBSDataArrayAutoRelease list = Utils::StringListToArray(profiles, "profile-name");
	bfree(profiles);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_array(response, "profiles", list);

	return req->SendOKResponse(response);
}
