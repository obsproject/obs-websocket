#include "RequestHandler.h"

#include "../plugin-macros.generated.h"

RequestResult RequestHandler::GetVersion(const Request& request)
{
	json ret{{"test", "pp"}};
	return RequestResult::Success(ret);
}
