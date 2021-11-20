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

RequestResult RequestHandler::GetStreamStatus(const Request& request)
{
	OBSOutputAutoRelease streamOutput = obs_frontend_get_streaming_output();

	uint64_t outputDuration = Utils::Obs::NumberHelper::GetOutputDuration(streamOutput);

	json responseData;
	responseData["outputActive"] = obs_output_active(streamOutput);
	responseData["outputReconnecting"] = obs_output_reconnecting(streamOutput);
	responseData["outputTimecode"] = Utils::Obs::StringHelper::DurationToTimecode(outputDuration);
	responseData["outputDuration"] = outputDuration;
	responseData["outputBytes"] = (uint64_t)obs_output_get_total_bytes(streamOutput);
	responseData["outputSkippedFrames"] = obs_output_get_frames_dropped(streamOutput);
	responseData["outputTotalFrames"] = obs_output_get_total_frames(streamOutput);

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::ToggleStream(const Request& request)
{
	json responseData;
	if (obs_frontend_streaming_active()) {
		obs_frontend_streaming_stop();
		responseData["outputActive"] = false;
	} else {
		obs_frontend_streaming_start();
		responseData["outputActive"] = true;
	}

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::StartStream(const Request& request)
{
	if (obs_frontend_streaming_active())
		return RequestResult::Error(RequestStatus::OutputRunning);

	// TODO: Call signal directly to perform blocking wait
	obs_frontend_streaming_start();

	return RequestResult::Success();
}

RequestResult RequestHandler::StopStream(const Request& request)
{
	if (!obs_frontend_streaming_active())
		return RequestResult::Error(RequestStatus::OutputNotRunning);

	// TODO: Call signal directly to perform blocking wait
	obs_frontend_streaming_stop();

	return RequestResult::Success();
}
