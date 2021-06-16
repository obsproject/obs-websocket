#include "Request.h"

#include "../../plugin-macros.generated.h"

json GetDefaultJsonObject(json requestData)
{
	if (!requestData.is_object())
		return json::object();
	else
		return requestData;
}

Request::Request(uint8_t rpcVersion, bool ignoreNonFatalRequestChecks, std::string requestType, json requestData) :
	RpcVersion(rpcVersion),
	IgnoreNonFatalRequestChecks(ignoreNonFatalRequestChecks),
	RequestData(GetDefaultJsonObject(requestData)),
	RequestType(requestType)
{
}

const bool Request::ValidateBasic(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const
{
	if (!HasRequestData()) {
		statusCode = RequestStatus::MissingRequestData;
		comment = "Your request data is missing or invalid (non-object)";
		return false;
	}

	if (!RequestData.contains(keyName)) {
		statusCode = RequestStatus::MissingRequestParameter;
		comment = std::string("Your request is missing the `") + keyName + "` parameter.";
		return false;
	}

	return true;
}

const bool Request::ValidateNumber(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const double minValue, const double maxValue) const
{
	if (!ValidateBasic(keyName, statusCode, comment))
		return false;

	if (!RequestData[keyName].is_number()) {
		statusCode = RequestStatus::InvalidRequestParameterDataType;
		comment = std::string("The parameter `") + keyName + "` must be a number.";
		return false;
	}

	double value = RequestData[keyName];
	if (value < minValue) {
		statusCode = RequestStatus::RequestParameterOutOfRange;
		comment = std::string("The parameter `") + keyName + "` is below the minimum of `" + std::to_string(minValue) + "`";
		return false;
	}
	if (value > maxValue) {
		statusCode = RequestStatus::RequestParameterOutOfRange;
		comment = std::string("The parameter `") + keyName + "` is above the maximum of `" + std::to_string(maxValue) + "`";
		return false;
	}

	return true;
}

const bool Request::ValidateString(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty) const
{
	if (!ValidateBasic(keyName, statusCode, comment))
		return false;

	if (!RequestData[keyName].is_string()) {
		statusCode = RequestStatus::InvalidRequestParameterDataType;
		comment = std::string("The parameter `") + keyName + "` must be a string.";
		return false;
	}

	if (RequestData[keyName].get<std::string>().empty() && !allowEmpty) {
		statusCode = RequestStatus::RequestParameterEmpty;
		comment = std::string("The parameter `") + keyName + "` must not be empty.";
		return false;
	}

	return true;
}

const bool Request::ValidateBoolean(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const
{
	if (!ValidateBasic(keyName, statusCode, comment))
		return false;

	if (!RequestData[keyName].is_boolean()) {
		statusCode = RequestStatus::InvalidRequestParameterDataType;
		comment = std::string("The parameter `") + keyName + "` must be boolean.";
		return false;
	}

	return true;
}

const bool Request::ValidateObject(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty) const
{
	if (!ValidateBasic(keyName, statusCode, comment))
		return false;

	if (!RequestData[keyName].is_object()) {
		statusCode = RequestStatus::InvalidRequestParameterDataType;
		comment = std::string("The parameter `") + keyName + "` must be an object.";
		return false;
	}

	if (RequestData[keyName].empty() && !allowEmpty) {
		statusCode = RequestStatus::RequestParameterEmpty;
		comment = std::string("The parameter `") + keyName + "` must not be empty.";
		return false;
	}

	return true;
}

const bool Request::ValidateArray(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty) const
{
	if (!ValidateBasic(keyName, statusCode, comment))
		return false;

	if (!RequestData[keyName].is_array()) {
		statusCode = RequestStatus::InvalidRequestParameterDataType;
		comment = std::string("The parameter `") + keyName + "` must be an array.";
		return false;
	}

	if (RequestData[keyName].empty() && !allowEmpty) {
		statusCode = RequestStatus::RequestParameterEmpty;
		comment = std::string("The parameter `") + keyName + "` must not be empty.";
		return false;
	}

	return true;
}

obs_source_t *Request::ValidateInput(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const
{
	if (!ValidateString(keyName, statusCode, comment))
		return nullptr;

	std::string inputName = RequestData[keyName];

	obs_source_t *ret = obs_get_source_by_name(inputName.c_str());
	if (!ret) {
		statusCode = RequestStatus::InputNotFound;
		comment = std::string("No input was found by the name of `") + inputName + "`.";
		return nullptr;
	}

	if (obs_source_get_type(ret) != OBS_SOURCE_TYPE_INPUT) {
		statusCode = RequestStatus::InvalidSourceType;
		comment = "The specified source is not an input.";
		return nullptr;
	}

	return ret;
}
