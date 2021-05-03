#pragma once

#include "RequestStatus.h"
#include "../../utils/Utils.h"

struct RequestResult
{
	RequestResult(RequestStatus statusCode = RequestStatus::Success, json responseData = nullptr, std::string comment = "");
	static RequestResult Success(json responseData = nullptr);
	static RequestResult Error(RequestStatus statusCode, std::string comment = "");
	RequestStatus StatusCode;
	json ResponseData;
	std::string Comment;
};
