#include "RequestHandler.h"
#include "../plugin-macros.generated.h"

RequestResult RequestHandler::GetStreamStatus(const Request& request)
{
	json responseData;

	OBSOutputAutoRelease streamOutput = obs_frontend_get_streaming_output();
	responseData["outputActive"] = obs_output_active(streamOutput);
	responseData["outputTimecode"] = Utils::Obs::StringHelper::GetOutputTimecodeString(streamOutput);
	responseData["outputDuration"] = Utils::Obs::NumberHelper::GetOutputDuration(streamOutput);

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::StartStream(const Request& request)
{
	if (obs_frontend_streaming_active())
		return RequestResult::Error(RequestStatus::StreamRunning);

	// TODO: Call signal directly to perform blocking wait
	obs_frontend_streaming_start();

	return RequestResult::Success();
}

RequestResult RequestHandler::StopStream(const Request& request)
{
	if (!obs_frontend_streaming_active())
		return RequestResult::Error(RequestStatus::StreamNotRunning);

	// TODO: Call signal directly to perform blocking wait
	obs_frontend_streaming_stop();

	return RequestResult::Success();
}
