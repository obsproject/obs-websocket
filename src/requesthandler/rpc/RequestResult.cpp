#include "RequestResult.h"

RequestResult::RequestResult(RequestStatus::RequestStatus statusCode, json responseData, std::string comment) :
	StatusCode(statusCode),
	ResponseData(responseData),
	Comment(comment),
	SleepFrames(0)
{
}

RequestResult RequestResult::Success(json responseData)
{
	return RequestResult(RequestStatus::Success, responseData, "");
}

RequestResult RequestResult::Error(RequestStatus::RequestStatus statusCode, std::string comment)
{
	return RequestResult(statusCode, nullptr, comment);
}
