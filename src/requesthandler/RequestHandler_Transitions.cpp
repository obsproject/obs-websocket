/*
obs-websocket
Copyright (C) 2016-2021 Stephane Lepin <stephane.lepin@gmail.com>
Copyright (C) 2020-2021 Kyle Manning <tt2468@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include "RequestHandler.h"

RequestResult RequestHandler::GetTransitionKindList(const Request&)
{
	json responseData;
	responseData["transitionKinds"] = Utils::Obs::ArrayHelper::GetTransitionKindList();
	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::GetSceneTransitionList(const Request&)
{
	json responseData;

	OBSSourceAutoRelease transition = obs_frontend_get_current_transition();
	if (transition) {
		responseData["currentSceneTransitionName"] = obs_source_get_name(transition);
		responseData["currentSceneTransitionKind"] = obs_source_get_id(transition);
	} else {
		responseData["currentSceneTransitionName"] = nullptr;
		responseData["currentSceneTransitionKind"] = nullptr;
	}

	responseData["transitions"] = Utils::Obs::ArrayHelper::GetSceneTransitionList();

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::GetCurrentSceneTransition(const Request&)
{
	OBSSourceAutoRelease transition = obs_frontend_get_current_transition();
	if (!transition)
		return RequestResult::Error(RequestStatus::InvalidResourceState, "OBS does not currently have a scene transition set."); // This should not happen!

	json responseData;
	responseData["transitionName"] = obs_source_get_name(transition);
	responseData["transitionKind"] = obs_source_get_id(transition);

	if (obs_transition_fixed(transition)) {
		responseData["transitionFixed"] = true;
		responseData["transitionDuration"] = nullptr;
	} else {
		responseData["transitionFixed"] = false;
		responseData["transitionDuration"] = obs_frontend_get_transition_duration();
	}

	if (obs_source_configurable(transition)) {
		responseData["transitionConfigurable"] = true;
		OBSDataAutoRelease transitionSettings = obs_source_get_settings(transition);
		responseData["transitionSettings"] = Utils::Json::ObsDataToJson(transitionSettings);
	} else {
		responseData["transitionConfigurable"] = false;
		responseData["transitionSettings"] = nullptr;
	}

	return RequestResult::Success(responseData);
}

// Transition names being unique are a UI concept and are not enforced by libobs
RequestResult RequestHandler::SetCurrentSceneTransition(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateString("transitionName", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	std::string transitionName = request.RequestData["transitionName"];

	OBSSourceAutoRelease transition = Utils::Obs::SearchHelper::GetSceneTransitionByName(transitionName);
	if (!transition)
		return RequestResult::Error(RequestStatus::ResourceNotFound, "No scene transition was found by that name.");

	obs_frontend_set_current_transition(transition);

	return RequestResult::Success();
}
