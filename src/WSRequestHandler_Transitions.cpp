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
HandlerResponse WSRequestHandler::HandleGetTransitionList(WSRequestHandler* req) {
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

	return req->SendOKResponse(response);
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
HandlerResponse WSRequestHandler::HandleGetCurrentTransition(WSRequestHandler* req) {
	OBSSourceAutoRelease currentTransition = obs_frontend_get_current_transition();

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "name",
		obs_source_get_name(currentTransition));

	if (!obs_transition_fixed(currentTransition))
		obs_data_set_int(response, "duration", obs_frontend_get_transition_duration());

	return req->SendOKResponse(response);
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
HandlerResponse WSRequestHandler::HandleSetCurrentTransition(WSRequestHandler* req) {
	if (!req->hasField("transition-name")) {
		return req->SendErrorResponse("missing request parameters");
	}

	QString name = obs_data_get_string(req->data, "transition-name");
	bool success = Utils::SetTransitionByName(name);
	if (!success) {
		return req->SendErrorResponse("requested transition does not exist");
	}
	
	return req->SendOKResponse();
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
HandlerResponse WSRequestHandler::HandleSetTransitionDuration(WSRequestHandler* req) {
	if (!req->hasField("duration")) {
		return req->SendErrorResponse("missing request parameters");
	}

	int ms = obs_data_get_int(req->data, "duration");
	obs_frontend_set_transition_duration(ms);
	return req->SendOKResponse();
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
HandlerResponse WSRequestHandler::HandleGetTransitionDuration(WSRequestHandler* req) {
	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_int(response, "transition-duration", obs_frontend_get_transition_duration());
	return req->SendOKResponse(response);
}
