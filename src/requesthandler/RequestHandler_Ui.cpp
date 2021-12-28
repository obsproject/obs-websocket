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

/**
 * Gets whether studio is enabled.
 *
 * @responseField studioModeEnabled | Boolean | Whether studio mode is enabled
 *
 * @requestType GetStudioModeEnabled
 * @complexity 1
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @category ui
 * @api requests
 */
RequestResult RequestHandler::GetStudioModeEnabled(const Request&)
{
	json responseData;
	responseData["studioModeEnabled"] = obs_frontend_preview_program_mode_active();
	return RequestResult::Success(responseData);
}

/**
 * Enables or disables studio mode
 *
 * @requestField studioModeEnabled | Boolean | True == Enabled, False == Disabled
 *
 * @requestType SetStudioModeEnabled
 * @complexity 1
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @category ui
 * @api requests
 */
RequestResult RequestHandler::SetStudioModeEnabled(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateBoolean("studioModeEnabled", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	// Avoid queueing tasks if nothing will change
	if (obs_frontend_preview_program_mode_active() != request.RequestData["studioModeEnabled"]) {
		// (Bad) Create a boolean then pass it as a reference to the task. Requires `wait` in obs_queue_task() to be true, else undefined behavior
		bool studioModeEnabled = request.RequestData["studioModeEnabled"];
		// Queue the task inside of the UI thread to prevent race conditions
		obs_queue_task(OBS_TASK_UI, [](void* param) {
			auto studioModeEnabled = (bool*)param;
			obs_frontend_set_preview_program_mode(*studioModeEnabled);
		}, &studioModeEnabled, true);
	}

	return RequestResult::Success();
}
