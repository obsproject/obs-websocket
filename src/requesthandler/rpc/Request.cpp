#include "Request.h"

#include "../../plugin-macros.generated.h"

Request::Request(uint8_t rpcVersion, bool ignoreNonFatalRequestChecks, std::string requestType, json requestData) :
	RpcVersion(rpcVersion),
	IgnoreNonFatalRequestChecks(ignoreNonFatalRequestChecks),
	RequestType(requestType)
{
	if (!requestData.is_object())
		RequestData = json::object();
	else
		RequestData = requestData;
}