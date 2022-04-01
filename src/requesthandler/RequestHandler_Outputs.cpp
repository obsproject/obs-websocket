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

static bool ReplayBufferAvailable()
{
	OBSOutputAutoRelease output = obs_frontend_get_replay_buffer_output();
	return output != nullptr;
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

/**
 * Gets the status of the replay buffer output.
 *
 * @responseField outputActive | Boolean | Whether the output is active
 *
 * @requestType GetReplayBufferStatus
 * @complexity 1
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @category outputs
 * @api requests
 */
RequestResult RequestHandler::GetReplayBufferStatus(const Request&)
{
	if (!ReplayBufferAvailable())
		return RequestResult::Error(RequestStatus::InvalidResourceState, "Replay buffer is not available.");

	json responseData;
	responseData["outputActive"] = obs_frontend_replay_buffer_active();
	return RequestResult::Success(responseData);
}

/**
 * Toggles the state of the replay buffer output.
 *
 * @responseField outputActive | Boolean | Whether the output is active
 *
 * @requestType ToggleReplayBuffer
 * @complexity 1
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @category outputs
 * @api requests
 */
RequestResult RequestHandler::ToggleReplayBuffer(const Request&)
{
	if (!ReplayBufferAvailable())
		return RequestResult::Error(RequestStatus::InvalidResourceState, "Replay buffer is not available.");

	bool outputActive = obs_frontend_replay_buffer_active();

	if (outputActive)
		obs_frontend_replay_buffer_stop();
	else
		obs_frontend_replay_buffer_start();

	json responseData;
	responseData["outputActive"] = !outputActive;
	return RequestResult::Success(responseData);
}

/**
 * Starts the replay buffer output.
 *
 * @requestType StartReplayBuffer
 * @complexity 1
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category outputs
 */
RequestResult RequestHandler::StartReplayBuffer(const Request&)
{
	if (!ReplayBufferAvailable())
		return RequestResult::Error(RequestStatus::InvalidResourceState, "Replay buffer is not available.");

	if (obs_frontend_replay_buffer_active())
		return RequestResult::Error(RequestStatus::OutputRunning);

	obs_frontend_replay_buffer_start();

	return RequestResult::Success();
}

/**
 * Stops the replay buffer output.
 *
 * @requestType StopReplayBuffer
 * @complexity 1
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category outputs
 */
RequestResult RequestHandler::StopReplayBuffer(const Request&)
{
	if (!ReplayBufferAvailable())
		return RequestResult::Error(RequestStatus::InvalidResourceState, "Replay buffer is not available.");

	if (!obs_frontend_replay_buffer_active())
		return RequestResult::Error(RequestStatus::OutputNotRunning);

	obs_frontend_replay_buffer_stop();

	return RequestResult::Success();
}

/**
 * Saves the contents of the replay buffer output.
 *
 * @requestType SaveReplayBuffer
 * @complexity 1
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category outputs
 */
RequestResult RequestHandler::SaveReplayBuffer(const Request&)
{
	if (!ReplayBufferAvailable())
		return RequestResult::Error(RequestStatus::InvalidResourceState, "Replay buffer is not available.");

	if (!obs_frontend_replay_buffer_active())
		return RequestResult::Error(RequestStatus::OutputNotRunning);

	obs_frontend_replay_buffer_save();

	return RequestResult::Success();
}

/**
 * Gets the filename of the last replay buffer save file.
 *
 * @responseField savedReplayPath | String | File path
 *
 * @requestType GetLastReplayBufferReplay
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category outputs
 */
RequestResult RequestHandler::GetLastReplayBufferReplay(const Request&)
{
	if (!ReplayBufferAvailable())
		return RequestResult::Error(RequestStatus::InvalidResourceState, "Replay buffer is not available.");

	if (!obs_frontend_replay_buffer_active())
		return RequestResult::Error(RequestStatus::OutputNotRunning);

	json responseData;
	responseData["savedReplayPath"] = Utils::Obs::StringHelper::GetLastReplayBufferFilePath();
	return RequestResult::Success(responseData);
}

/**
 * Opens a projector window or creates a projector on a monitor.
 *
 * @requestField videoType | String | Type of projector to open | "Preview", "Source", "Scene", "StudioProgram" or "Multiveiew" (Case insensitive)
 * @requestField ?projectorMonitor | Number | Monitor to open projector on | None | -1: Opens projector in a window
 * @requestField ?projectorGeometry | String | Size and position of projector window, encoded in Base64 using Qt's geometry encoding. Only applies if projectorMonitor is -1 | None | N/A
 * @requestfield ?sourceName | String | If videoType is "Source", the name of the source to create a projector for | None | N/A
 * *
 * @requestType OpenProjector
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category outputs
 */
RequestResult RequestHandler::OpenProjector(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;

	if (!request.ValidateString("videoType", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	bool hasMonitor = request.Contains("projectorMonitor");
	if (hasMonitor && !request.ValidateOptionalNumber("projectorMonitor", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	bool hasGeometry = request.Contains("projectorGeometry");
	if (hasGeometry && !request.ValidateOptionalNumber("projectorGeometry", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	bool hasSource = request.Contains("sourceName");
	if (hasSource) {
		OBSSourceAutoRelease source = request.ValidateSource("sourceName", statusCode, comment);
		if (!source)
			return RequestResult::Error(statusCode, comment);
	}

	std::string videoType = request.RequestData["videoType"];
	int monitor = -1;
	std::string geometry;
	std::string sourceName;

	if (hasMonitor)
		monitor = request.RequestData["projectorMonitor"];

	if (hasGeometry)
		geometry = request.RequestData["projectorGeometry"];

	if (hasSource)
		sourceName = request.RequestData["sourceName"];

	obs_frontend_open_projector(videoType.c_str(), monitor, geometry.c_str(), sourceName.c_str());

	return RequestResult::Success();
}

/**
 * Opens a projector window or creates a projector on a monitor for a specified source.
 *
 * @requestfield sourceName | String | The name of the source to create a projector for | None | N/A
 * @requestField ?projectorMonitor | Number | Monitor to open projector on | None | -1: Opens projector in a window
 * @requestField ?projectorGeometry | String | Size and position of projector window, encoded in Base64 using Qt's geometry encoding. Only applies if projectorMonitor is -1 | None | N/A
 * *
 * @requestType OpenSourceProjector
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category outputs
 */
RequestResult RequestHandler::OpenSourceProjector(const Request &request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;

	OBSSourceAutoRelease source = request.ValidateSource("sourceName", statusCode, comment);
	if (!source)
		return RequestResult::Error(statusCode, comment);

	bool hasMonitor = request.Contains("projectorMonitor");
	if (hasMonitor && !request.ValidateOptionalNumber("projectorMonitor", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	bool hasGeometry = request.Contains("projectorGeometry");
	if (hasGeometry && !request.ValidateOptionalNumber("projectorGeometry", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	std::string sourceName = request.RequestData["sourceName"];
	int monitor = -1;
	std::string geometry;

	if (hasMonitor)
		monitor = request.RequestData["projectorMonitor"];

	if (hasGeometry)
		geometry = request.RequestData["projectorGeometry"];

	obs_frontend_open_projector("source", monitor, geometry.c_str(), sourceName.c_str());

	return RequestResult::Success();
}
