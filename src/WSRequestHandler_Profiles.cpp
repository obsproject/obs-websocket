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

	const char* profileName = obs_data_get_string(request.parameters(), "profile-name");
	if (!profileName) {
		return request.failed("invalid request parameters");
	}

	char** profiles = obs_frontend_get_profiles();
	bool profileExists = Utils::StringInStringList(profiles, profileName);
	bfree(profiles);
	if (!profileExists) {
		return request.failed("profile does not exist");
	}

	obs_queue_task(OBS_TASK_UI, [](void* param) {
		obs_frontend_set_current_profile(reinterpret_cast<const char*>(param));
	}, (void*)profileName, true);

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
 * @return {String} `profiles.*.profile-name` Filter name
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
