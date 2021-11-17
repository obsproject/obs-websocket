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
