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
RpcResponse WSRequestHandler::SetCurrentProfile(const RpcRequest& request) {
	if (!request.hasField("profile-name")) {
		return request.failed("missing request parameters");
	}

	QString profileName = obs_data_get_string(request.parameters(), "profile-name");
	if (profileName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	// TODO : check if profile exists
	obs_frontend_set_current_profile(profileName.toUtf8());
	return request.success();
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
RpcResponse WSRequestHandler::GetCurrentProfile(const RpcRequest& request) {
	OBSDataAutoRelease response = obs_data_create();
	char* currentProfile = obs_frontend_get_current_profile();
	obs_data_set_string(response, "profile-name", currentProfile);
	bfree(currentProfile);
	return request.success(response);
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
RpcResponse WSRequestHandler::ListProfiles(const RpcRequest& request) {
	char** profiles = obs_frontend_get_profiles();
	OBSDataArrayAutoRelease list = Utils::StringListToArray(profiles, "profile-name");
	bfree(profiles);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_array(response, "profiles", list);

	return request.success(response);
}
