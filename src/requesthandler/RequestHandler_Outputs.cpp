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

static bool VirtualCamAvailable()
{
	OBSDataAutoRelease privateData = obs_get_private_data();
	if (!privateData)
		return false;

	return obs_data_get_bool(privateData, "vcamEnabled");
}

/**
 * Gets the status of the virtualcam output.
 *
 * @responseField outputActive | Boolean | Whether the output is active
 *
 * @requestType GetVirtualCamStatus
 * @complexity 1
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @category outputs
 * @api requests
 */
RequestResult RequestHandler::GetVirtualCamStatus(const Request&)
{
	if (!VirtualCamAvailable())
		return RequestResult::Error(RequestStatus::InvalidResourceState, "VirtualCam is not available.");

	json responseData;
	responseData["outputActive"] = obs_frontend_virtualcam_active();
	return RequestResult::Success(responseData);
}

/**
 * Toggles the state of the virtualcam output.
 *
 * @responseField outputActive | Boolean | Whether the output is active
 *
 * @requestType ToggleVirtualCam
 * @complexity 1
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @category outputs
 * @api requests
 */
RequestResult RequestHandler::ToggleVirtualCam(const Request&)
{
	if (!VirtualCamAvailable())
		return RequestResult::Error(RequestStatus::InvalidResourceState, "VirtualCam is not available.");

	bool outputActive = obs_frontend_virtualcam_active();

	if (outputActive)
		obs_frontend_stop_virtualcam();
	else
		obs_frontend_start_virtualcam();

	json responseData;
	responseData["outputActive"] = !outputActive;
	return RequestResult::Success(responseData);
}

/**
 * Starts the virtualcam output.
 *
 * @requestType StartVirtualCam
 * @complexity 1
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category outputs
 */
RequestResult RequestHandler::StartVirtualCam(const Request&)
{
	if (!VirtualCamAvailable())
		return RequestResult::Error(RequestStatus::InvalidResourceState, "VirtualCam is not available.");

	if (obs_frontend_virtualcam_active())
		return RequestResult::Error(RequestStatus::OutputRunning);

	obs_frontend_start_virtualcam();

	return RequestResult::Success();
}

/**
 * Stops the virtualcam output.
 *
 * @requestType StopVirtualCam
 * @complexity 1
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category outputs
 */
RequestResult RequestHandler::StopVirtualCam(const Request&)
{
	if (!VirtualCamAvailable())
		return RequestResult::Error(RequestStatus::InvalidResourceState, "VirtualCam is not available.");

	if (!obs_frontend_virtualcam_active())
		return RequestResult::Error(RequestStatus::OutputNotRunning);

	obs_frontend_stop_virtualcam();

	return RequestResult::Success();
}
