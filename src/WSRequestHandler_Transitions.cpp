#include "Utils.h"

#include "WSRequestHandler.h"

/**
 * List of all transitions available in the frontend's dropdown menu.
 *
 * @return {String} `current-transition` Name of the currently active transition.
 * @return {Array<Object>} `transitions` List of transitions.
 * @return {String} `transitions.*.name` Name of the transition.
 *
 * @api requests
 * @name GetTransitionList
 * @category transitions
 * @since 4.1.0
 */
RpcResponse WSRequestHandler::GetTransitionList(const RpcRequest& request) {
	OBSSourceAutoRelease currentTransition = obs_frontend_get_current_transition();
	obs_frontend_source_list transitionList = {};
	obs_frontend_get_transitions(&transitionList);

	OBSDataArrayAutoRelease transitions = obs_data_array_create();
	for (size_t i = 0; i < transitionList.sources.num; i++) {
		OBSSource transition = transitionList.sources.array[i];

		OBSDataAutoRelease obj = obs_data_create();
		obs_data_set_string(obj, "name", obs_source_get_name(transition));
		obs_data_array_push_back(transitions, obj);
	}
	obs_frontend_source_list_free(&transitionList);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "current-transition",
		obs_source_get_name(currentTransition));
	obs_data_set_array(response, "transitions", transitions);

	return request.success(response);
}

/**
 * Get the name of the currently selected transition in the frontend's dropdown menu.
 *
 * @return {String} `name` Name of the selected transition.
 * @return {int (optional)} `duration` Transition duration (in milliseconds) if supported by the transition.
 *
 * @api requests
 * @name GetCurrentTransition
 * @category transitions
 * @since 0.3
 */
RpcResponse WSRequestHandler::GetCurrentTransition(const RpcRequest& request) {
	OBSSourceAutoRelease currentTransition = obs_frontend_get_current_transition();

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "name",
		obs_source_get_name(currentTransition));

	if (!obs_transition_fixed(currentTransition))
		obs_data_set_int(response, "duration", obs_frontend_get_transition_duration());

	return request.success(response);
}

/**
 * Set the active transition.
 *
 * @param {String} `transition-name` The name of the transition.
 *
 * @api requests
 * @name SetCurrentTransition
 * @category transitions
 * @since 0.3
 */
RpcResponse WSRequestHandler::SetCurrentTransition(const RpcRequest& request) {
	if (!request.hasField("transition-name")) {
		return request.failed("missing request parameters");
	}

	QString name = obs_data_get_string(request.parameters(), "transition-name");
	bool success = Utils::SetTransitionByName(name);
	if (!success) {
		return request.failed("requested transition does not exist");
	}

	return request.success();
}

/**
 * Set the duration of the currently selected transition if supported.
 *
 * @param {int} `duration` Desired duration of the transition (in milliseconds).
 *
 * @api requests
 * @name SetTransitionDuration
 * @category transitions
 * @since 4.0.0
 */
RpcResponse WSRequestHandler::SetTransitionDuration(const RpcRequest& request) {
	if (!request.hasField("duration")) {
		return request.failed("missing request parameters");
	}

	int ms = obs_data_get_int(request.parameters(), "duration");
	obs_frontend_set_transition_duration(ms);
	return request.success();
}

/**
 * Get the duration of the currently selected transition if supported.
 *
 * @return {int} `transition-duration` Duration of the current transition (in milliseconds).
 *
 * @api requests
 * @name GetTransitionDuration
 * @category transitions
 * @since 4.1.0
 */
RpcResponse WSRequestHandler::GetTransitionDuration(const RpcRequest& request) {
	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_int(response, "transition-duration", obs_frontend_get_transition_duration());
	return request.success(response);
}

/**
 * Get the position of the current transition.
 *
 * @return {double} `position` current transition position. This value will be between 0.0 and 1.0. Note: Transition returns 1.0 when not active.
 *
 * @api requests
 * @name GetTransitionPosition
 * @category transitions
 * @since 4.9.0
 */
RpcResponse WSRequestHandler::GetTransitionPosition(const RpcRequest& request) {
	OBSSourceAutoRelease currentTransition = obs_frontend_get_current_transition();

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_double(response, "position", obs_transition_get_time(currentTransition));

	return request.success(response);
}

/**
 * Get the current settings of a transition
 *
 * @param {String} `transitionName` Transition name
 *
 * @return {Object} `transitionSettings` Current transition settings
 *
 * @api requests
 * @name GetTransitionSettings
 * @category transitions
 * @since 4.9.0
 */
RpcResponse WSRequestHandler::GetTransitionSettings(const RpcRequest& request) {
	if (!request.hasField("transitionName")) {
		return request.failed("missing request parameters");
	}

	const char* transitionName = obs_data_get_string(request.parameters(), "transitionName");
	OBSSourceAutoRelease transition = Utils::GetTransitionFromName(transitionName);
	if (!transition) {
		return request.failed("specified transition doesn't exist");
	}

	OBSDataAutoRelease transitionSettings = obs_source_get_settings(transition);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_obj(response, "transitionSettings", transitionSettings);
	return request.success(response);
}

/**
 * Change the current settings of a transition
 *
 * @param {String} `transitionName` Transition name
 * @param {Object} `transitionSettings` Transition settings (they can be partial)
 *
 * @return {Object} `transitionSettings` Updated transition settings
 *
 * @api requests
 * @name SetTransitionSettings
 * @category transitions
 * @since 4.9.0
 */
RpcResponse WSRequestHandler::SetTransitionSettings(const RpcRequest& request) {
	if (!request.hasField("transitionName") || !request.hasField("transitionSettings")) {
		return request.failed("missing request parameters");
	}

	const char* transitionName = obs_data_get_string(request.parameters(), "transitionName");
	OBSSourceAutoRelease transition = Utils::GetTransitionFromName(transitionName);
	if (!transition) {
		return request.failed("specified transition doesn't exist");
	}

	OBSDataAutoRelease newSettings = obs_data_get_obj(request.parameters(), "transitionSettings");
	obs_source_update(transition, newSettings);
	obs_source_update_properties(transition);

	OBSDataAutoRelease updatedSettings = obs_source_get_settings(transition);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_obj(response, "transitionSettings", updatedSettings);
	return request.success(response);
}

/**
 * Release the T-Bar (like a user releasing their mouse button after moving it).
 * *YOU MUST CALL THIS if you called `SetTBarPosition` with the `release` parameter set to `false`.*
 *
 * @api requests
 * @name ReleaseTBar
 * @category transitions
 * @since 4.9.0
 */
RpcResponse WSRequestHandler::ReleaseTBar(const RpcRequest& request) {
	if (!obs_frontend_preview_program_mode_active()) {
		return request.failed("studio mode not enabled");
	}

    OBSSourceAutoRelease transition = obs_frontend_get_current_transition();
	if (obs_transition_fixed(transition)) {
		return request.failed("current transition doesn't support t-bar control");
	}

	obs_frontend_release_tbar();

	return request.success();
}

/**
 * Set the manual position of the T-Bar (in Studio Mode) to the specified value. Will return an error if OBS is not in studio mode
 * or if the current transition doesn't support T-Bar control.
 *
 * If your code needs to perform multiple successive T-Bar moves (e.g. : in an animation, or in response to a user moving a T-Bar control in your User Interface), set `release` to false and call `ReleaseTBar` later once the animation/interaction is over.
 *
 * @param {double} `position` T-Bar position. This value must be between 0.0 and 1.0.
 * @param {boolean (optional)} `release` Whether or not the T-Bar gets released automatically after setting its new position (like a user releasing their mouse button after moving the T-Bar). Call `ReleaseTBar` manually if you set `release` to false. Defaults to true.
 *
 * @api requests
 * @name SetTBarPosition
 * @category transitions
 * @since 4.9.0
 */
RpcResponse WSRequestHandler::SetTBarPosition(const RpcRequest& request) {
	if (!obs_frontend_preview_program_mode_active()) {
		return request.failed("studio mode not enabled");
	}

    OBSSourceAutoRelease transition = obs_frontend_get_current_transition();
	if (obs_transition_fixed(transition)) {
		return request.failed("current transition doesn't support t-bar control");
	}

	if (!request.hasField("position")) {
		return request.failed("missing request parameters");
	}

	double position = obs_data_get_double(request.parameters(), "position");
	if (position < 0.0 || position > 1.0) {
		return request.failed("position is out of range");
	}

	bool release = true;
	if (request.hasField("release")) {
		release = obs_data_get_bool(request.parameters(), "release");
	}

	obs_frontend_set_tbar_position((int)((float)position * 1024.0));
	if (release) {
		obs_frontend_release_tbar();
	}

	return request.success();
}
