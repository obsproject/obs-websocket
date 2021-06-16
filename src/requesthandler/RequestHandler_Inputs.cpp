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
	if (!input || !request.ValidateObject("inputSettings", statusCode, comment, true))
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

RequestResult RequestHandler::GetInputMute(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease input = request.ValidateInput("inputName", statusCode, comment);
	if (!input)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["inputMuted"] = obs_source_muted(input);
	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetInputMute(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease input = request.ValidateInput("inputName", statusCode, comment);
	if (!input || !request.ValidateBoolean("inputMuted", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	obs_source_set_muted(input, request.RequestData["inputMuted"]);

	return RequestResult::Success();
}

RequestResult RequestHandler::ToggleInputMute(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease input = request.ValidateInput("inputName", statusCode, comment);
	if (!input)
		return RequestResult::Error(statusCode, comment);

	bool inputMuted = !obs_source_muted(input);
	obs_source_set_muted(input, inputMuted);

	json responseData;
	responseData["inputMuted"] = inputMuted;
	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::GetInputVolume(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease input = request.ValidateInput("inputName", statusCode, comment);
	if (!input)
		return RequestResult::Error(statusCode, comment);

	float inputVolumeMul = obs_source_get_volume(input);
	float inputVolumeDb = obs_mul_to_db(inputVolumeMul);
	if (inputVolumeDb == -INFINITY)
		inputVolumeDb = -100.0;

	json responseData;
	responseData["inputVolumeMul"] = inputVolumeMul;
	responseData["inputVolumeDb"] = inputVolumeDb;
	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetInputVolume(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	OBSSourceAutoRelease input = request.ValidateInput("inputName", statusCode, comment);
	if (!input)
		return RequestResult::Error(statusCode, comment);

	bool hasMul = request.ValidateNumber("inputVolumeMul", statusCode, comment, 0, 20);
	if (!hasMul && statusCode != RequestStatus::MissingRequestParameter)
		return RequestResult::Error(statusCode, comment);

	bool hasDb = request.ValidateNumber("inputVolumeDb", statusCode, comment, -100, 26);
	if (!hasDb && statusCode != RequestStatus::MissingRequestParameter)
		return RequestResult::Error(statusCode, comment);

	if (hasMul && hasDb)
		return RequestResult::Error(RequestStatus::TooManyRequestParameters, "You may only specify one volume parameter.");

	if (!hasMul && !hasDb)
		return RequestResult::Error(RequestStatus::MissingRequestParameter, "You must specify one volume parameter.");

	float inputVolumeMul = 0.0;
	if (hasMul)
		inputVolumeMul = request.RequestData["inputVolumeMul"];
	else
		inputVolumeMul = obs_db_to_mul(request.RequestData["inputVolumeDb"]);

	obs_source_set_volume(input, inputVolumeMul);

	return RequestResult::Success();
}
