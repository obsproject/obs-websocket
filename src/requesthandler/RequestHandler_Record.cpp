#include "RequestHandler.h"

RequestResult RequestHandler::GetRecordStatus(const Request& request)
{
	OBSOutputAutoRelease recordOutput = obs_frontend_get_streaming_output();

	json responseData;
	responseData["outputActive"] = obs_output_active(recordOutput);
	responseData["outputPaused"] = obs_output_paused(recordOutput);
	responseData["outputTimecode"] = Utils::Obs::StringHelper::GetOutputTimecodeString(recordOutput);
	responseData["outputDuration"] = Utils::Obs::NumberHelper::GetOutputDuration(recordOutput);
	responseData["outputBytes"] = (uint64_t)obs_output_get_total_bytes(recordOutput);

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::ToggleRecord(const Request& request)
{
	json responseData;
	if (obs_frontend_recording_active()) {
		obs_frontend_recording_stop();
		responseData["outputActive"] = false;
	} else {
		obs_frontend_recording_start();
		responseData["outputActive"] = true;
	}

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::StartRecord(const Request& request)
{
	if (obs_frontend_recording_active())
		return RequestResult::Error(RequestStatus::OutputRunning);

	// TODO: Call signal directly to perform blocking wait
	obs_frontend_recording_start();

	return RequestResult::Success();
}

RequestResult RequestHandler::StopRecord(const Request& request)
{
	if (!obs_frontend_recording_active())
		return RequestResult::Error(RequestStatus::OutputNotRunning);

	// TODO: Call signal directly to perform blocking wait
	obs_frontend_recording_stop();

	return RequestResult::Success();
}

RequestResult RequestHandler::ToggleRecordPause(const Request& request)
{
	json responseData;
	if (obs_frontend_recording_paused()) {
		obs_frontend_recording_pause(false);
		responseData["outputPaused"] = false;
	} else {
		obs_frontend_recording_pause(true);
		responseData["outputPaused"] = true;
	}

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::PauseRecord(const Request& request)
{
	if (obs_frontend_recording_paused())
		return RequestResult::Error(RequestStatus::OutputPaused);

	// TODO: Call signal directly to perform blocking wait
	obs_frontend_recording_pause(true);

	return RequestResult::Success();
}

RequestResult RequestHandler::ResumeRecord(const Request& request)
{
	if (!obs_frontend_recording_paused())
		return RequestResult::Error(RequestStatus::OutputNotPaused);

	// TODO: Call signal directly to perform blocking wait
	obs_frontend_recording_pause(false);

	return RequestResult::Success();
}

RequestResult RequestHandler::GetRecordDirectory(const Request& request)
{
	json responseData;
	responseData["recordDirectory"] = Utils::Obs::StringHelper::GetCurrentRecordOutputPath();

	return RequestResult::Success(responseData);
}
