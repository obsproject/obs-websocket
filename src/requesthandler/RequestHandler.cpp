#include <obs.hpp>
#include <obs-frontend-api.h>

#include "RequestHandler.h"

#include "../plugin-macros.generated.h"

RequestHandler::RequestHandler(bool ignoreNonFatalRequestChecks, uint8_t rpcVersion) :
	_ignoreNonFatalRequestChecks(ignoreNonFatalRequestChecks),
	_rpcVersion(rpcVersion)
{
}

RequestHandler::RequestHandler(SessionPtr session) :
	_ignoreNonFatalRequestChecks(session->IgnoreNonFatalRequestChecks()),
	_rpcVersion(session->RpcVersion())
{
}

RequestHandler::RequestResult RequestHandler::ProcessRequest(std::string requestType, json requestData)
{
	RequestHandler::RequestResult ret;

	ret.statusCode = RequestHandler::RequestStatus::Success;

	return ret;
}