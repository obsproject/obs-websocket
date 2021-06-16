#include "RequestHandler.h"

#include "../plugin-macros.generated.h"

struct EnumInputInfo {
	std::string inputKind;
	std::vector<json> inputs;
};

RequestResult RequestHandler::GetInputList(const Request& request)
{
	EnumInputInfo inputInfo;

	if (request.RequestData.contains("inputKind") && !request.RequestData["inputKind"].is_null()) {
		RequestStatus::RequestStatus statusCode;
		std::string comment;
		if (!request.ValidateString("inputKind", statusCode, comment)) {
			return RequestResult::Error(statusCode, comment);
		}

		inputInfo.inputKind = request.RequestData["inputKind"];
	}

	auto inputEnumProc = [](void *param, obs_source_t *input) {
		// Sanity check in case the API changes
		if (obs_source_get_type(input) != OBS_SOURCE_TYPE_INPUT)
			return true;

		auto inputInfo = reinterpret_cast<EnumInputInfo*>(param);

		std::string inputKind = obs_source_get_id(input);

		if (!inputInfo->inputKind.empty() && inputInfo->inputKind != inputKind)
			return true;

		json inputJson;
		inputJson["inputName"] = obs_source_get_name(input);
		inputJson["inputKind"] = inputKind;
		inputJson["unversionedInputKind"] = obs_source_get_unversioned_id(input);

		inputInfo->inputs.push_back(inputJson);
		return true;
	};
	// Actually enumerates only public inputs, despite the name
	obs_enum_sources(inputEnumProc, &inputInfo);

	json responseData;
	responseData["inputs"] = inputInfo.inputs;
	return RequestResult::Success(responseData);
}
