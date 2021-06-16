#include "RequestHandler.h"

#include "../plugin-macros.generated.h"

RequestResult RequestHandler::GetInputList(const Request& request)
{
	std::string inputKind;

	if (request.RequestData.contains("inputKind") && !request.RequestData["inputKind"].is_null()) {
		RequestStatus::RequestStatus statusCode;
		std::string comment;
		if (!request.ValidateString("inputKind", statusCode, comment)) {
			return RequestResult::Error(statusCode, comment);
		}

		inputKind = request.RequestData["inputKind"];
	}

	json responseData;
	responseData["inputs"] = Utils::Obs::ListHelper::GetInputList(inputKind);
	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::GetInputKindList(const Request& request)
{
	bool unversioned = false;

	if (request.RequestData.contains("unversioned") && !request.RequestData["unversioned"].is_null()) {
		RequestStatus::RequestStatus statusCode;
		std::string comment;
		if (!request.ValidateBoolean("unversioned", statusCode, comment)) {
			return RequestResult::Error(statusCode, comment);
		}

		unversioned = request.RequestData["unversioned"];
	}

	json responseData;
	responseData["inputKinds"] = Utils::Obs::ListHelper::GetInputKindList(unversioned);
	return RequestResult::Success(responseData);
}
