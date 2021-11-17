#include "RequestHandler.h"

RequestResult RequestHandler::GetMediaInputStatus(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease input = request.ValidateInput("inputName", statusCode, comment);
	if (!input)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["mediaState"] = Utils::Obs::StringHelper::GetMediaInputState(input);

	auto mediaState = obs_source_media_get_state(input);
	if (mediaState == OBS_MEDIA_STATE_PLAYING || mediaState == OBS_MEDIA_STATE_PAUSED) {
		responseData["mediaDuration"] = obs_source_media_get_duration(input);
		responseData["mediaCursor"] = obs_source_media_get_time(input);
	}

	return RequestResult::Success(responseData);
}
