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

RequestResult RequestHandler::GetInputDefaultSettings(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateString("inputKind", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	std::string inputKind = request.RequestData["inputKind"];

	OBSDataAutoRelease defaultSettings = obs_get_source_defaults(inputKind.c_str());
	if (!defaultSettings)
		return RequestResult::Error(RequestStatus::InvalidInputKind);

	json responseData;
	responseData["defaultInputSettings"] = Utils::Json::ObsDataToJson(defaultSettings, true);
	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::GetInputSettings(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease input = request.ValidateInput("inputName", statusCode, comment);
	if (!input)
		return RequestResult::Error(statusCode, comment);

	OBSDataAutoRelease inputSettings = obs_source_get_settings(input);

	json responseData;
	responseData["inputSettings"] = Utils::Json::ObsDataToJson(inputSettings);
	responseData["inputKind"] = obs_source_get_id(input);
	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetInputSettings(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease input = request.ValidateInput("inputName", statusCode, comment);
	if (!input)
		return RequestResult::Error(statusCode, comment);

	if (!request.ValidateObject("inputSettings", statusCode, comment, true))
		return RequestResult::Error(statusCode, comment);

	bool overlay = true;
	if (request.RequestData.contains("overlay") && !request.RequestData["overlay"].is_null()) {
		if (!request.ValidateBoolean("overlay", statusCode, comment)) {
			return RequestResult::Error(statusCode, comment);
		}

		overlay = request.RequestData["overlay"];
	}

	// Get the new settings and convert it to obs_data_t*
	OBSDataAutoRelease newSettings = Utils::Json::JsonToObsData(request.RequestData["inputSettings"]);
	if (!newSettings)
		// This should never happen
		return RequestResult::Error(RequestStatus::RequestProcessingFailed, "An internal data conversion operation failed. Please report this!");

	if (overlay)
		// Applies the new settings on top of the existing user settings
		obs_source_update(input, newSettings);
	else
		// Clears all user settings (leaving defaults) then applies the new settings
		obs_source_reset_settings(input, newSettings);

	// Tells any open source properties windows to perform a UI refresh
	obs_source_update_properties(input);

	return RequestResult::Success();
}
