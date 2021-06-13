#include "RequestHandler.h"

#include "../plugin-macros.generated.h"

RequestResult RequestHandler::GetSourceActive(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateString("sourceName", statusCode, comment)) {
		return RequestResult::Error(statusCode, comment);
	}

	std::string sourceName = request.RequestData["sourceName"];

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.c_str());
	if (!source)
		return RequestResult::Error(RequestStatus::SourceNotFound);

	json responseData;
	responseData["videoActive"] = obs_source_active(source);
	responseData["videoShowing"] = obs_source_showing(source);
	return RequestResult::Success(responseData);
}
