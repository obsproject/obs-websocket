#pragma once

#include "RequestStatus.h"
#include "../../utils/Utils.h"

struct Request
{
	Request(const uint8_t rpcVersion, const bool ignoreNonFatalRequestChecks, const std::string requestType, const json requestData = nullptr);

	const bool HasRequestData() const
	{
		return RequestData.is_object() && !RequestData.empty();
	}

	const bool ValidateBasic(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const;
	const bool ValidateNumber(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const double minValue = -INFINITY, const double maxValue = INFINITY) const;
	const bool ValidateString(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty = false) const;
	const bool ValidateBoolean(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const;
	const bool ValidateObject(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty = false) const;
	const bool ValidateArray(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty = false) const;

	obs_source_t *ValidateScene(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const;
	obs_source_t *ValidateInput(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const;

	const uint8_t RpcVersion;
	const bool IgnoreNonFatalRequestChecks;
	const std::string RequestType;
	const json RequestData;
};