#pragma once

#include "../../utils/Utils.h"

struct Request
{
	Request(uint8_t rpcVersion, bool ignoreNonFatalRequestChecks, std::string requestType, json requestData = nullptr);

	uint8_t RpcVersion;
	bool IgnoreNonFatalRequestChecks;
	std::string RequestType;
	json RequestData;
};