#include <util/config-file.h>

#include "RequestHandler.h"
#include "../plugin-macros.generated.h"

RequestResult RequestHandler::GetSceneCollectionList(const Request& request)
{
	json responseData;
	responseData["currentSceneCollectionName"] = Utils::Obs::StringHelper::GetCurrentSceneCollection();
	responseData["sceneCollections"] = Utils::Obs::ListHelper::GetSceneCollectionList();
	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetCurrentSceneCollection(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateString("sceneCollectionName", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	std::string sceneCollectionName = request.RequestData["sceneCollectionName"];

	auto sceneCollections = Utils::Obs::ListHelper::GetSceneCollectionList();
	if (std::find(sceneCollections.begin(), sceneCollections.end(), sceneCollectionName) == sceneCollections.end())
		return RequestResult::Error(RequestStatus::SceneCollectionNotFound);

	std::string currentSceneCollectionName = Utils::Obs::StringHelper::GetCurrentSceneCollection();
	// Avoid queueing tasks if nothing will change
	if (currentSceneCollectionName != sceneCollectionName) {
		obs_queue_task(OBS_TASK_UI, [](void* param) {
			obs_frontend_set_current_scene_collection(reinterpret_cast<const char*>(param));
		}, (void*)sceneCollectionName.c_str(), true);
	}

	return RequestResult::Success();
}

RequestResult RequestHandler::GetProfileList(const Request& request)
{
	json responseData;
	responseData["currentProfileName"] = Utils::Obs::StringHelper::GetCurrentProfile();
	responseData["profiles"] = Utils::Obs::ListHelper::GetProfileList();
	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetCurrentProfile(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateString("profileName", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	std::string profileName = request.RequestData["profileName"];

	auto profiles = Utils::Obs::ListHelper::GetProfileList();
	if (std::find(profiles.begin(), profiles.end(), profileName) == profiles.end())
		return RequestResult::Error(RequestStatus::ProfileNotFound);

	std::string currentProfileName = Utils::Obs::StringHelper::GetCurrentProfile();
	// Avoid queueing tasks if nothing will change
	if (currentProfileName != profileName) {
		obs_queue_task(OBS_TASK_UI, [](void* param) {
			obs_frontend_set_current_profile(reinterpret_cast<const char*>(param));
		}, (void*)profileName.c_str(), true);
	}

	return RequestResult::Success();
}

RequestResult RequestHandler::GetProfileParameter(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!(request.ValidateString("parameterCategory", statusCode, comment) && request.ValidateString("parameterName", statusCode, comment)))
		return RequestResult::Error(statusCode, comment);

	std::string parameterCategory = request.RequestData["parameterCategory"];
	std::string parameterName = request.RequestData["parameterName"];

	config_t* profile = obs_frontend_get_profile_config();

	json responseData;
	responseData["parameterValue"] = config_get_string(profile, parameterCategory.c_str(), parameterName.c_str());
	responseData["defaultParameterValue"] = config_get_default_string(profile, parameterCategory.c_str(), parameterName.c_str());

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetProfileParameter(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!(request.ValidateString("parameterCategory", statusCode, comment) &&
	request.ValidateString("parameterName", statusCode, comment)))
		return RequestResult::Error(statusCode, comment);

	std::string parameterCategory = request.RequestData["parameterCategory"];
	std::string parameterName = request.RequestData["parameterName"];

	config_t* profile = obs_frontend_get_profile_config();

	// Using check helpers here would just make the logic more complicated
	if (!request.RequestData.contains("parameterValue") || request.RequestData["parameterValue"].is_null()) {
		if (!config_remove_value(profile, parameterCategory.c_str(), parameterName.c_str()))
			return RequestResult::Error(RequestStatus::ConfigParameterNotFound);
	} else if (request.RequestData["parameterValue"].is_string()) {
		std::string parameterValue = request.RequestData["parameterValue"];
		config_set_string(profile, parameterCategory.c_str(), parameterName.c_str(), parameterValue.c_str());
	} else {
		return RequestResult::Error(RequestStatus::InvalidRequestParameterDataType, "The parameter `parameterValue` must be a string.");
	}
	
	return RequestResult::Success();
}

RequestResult RequestHandler::GetVideoSettings(const Request& request)
{
	struct obs_video_info ovi;
	if (!obs_get_video_info(&ovi))
		return RequestResult::Error(RequestStatus::RequestProcessingFailed, "Unable to get internal OBS video info.");

	json responseData;
	responseData["fpsNumerator"] = ovi.fps_num;
	responseData["fpsDenominator"] = ovi.fps_den;
	responseData["baseWidth"] = ovi.base_width;
	responseData["baseHeight"] = ovi.base_height;
	responseData["outputWidth"] = ovi.output_width;
	responseData["outputHeight"] = ovi.output_height;

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetVideoSettings(const Request& request)
{
	if (obs_video_active())
		return RequestResult::Error(RequestStatus::OutputRunning, "Video settings cannot be changed while an output is active.");

	RequestStatus::RequestStatus statusCode = RequestStatus::NoError;
	std::string comment;
	bool changeFps = (request.ValidateNumber("fpsNumerator", statusCode, comment, 1) && request.ValidateNumber("fpsDenominator", statusCode, comment, 1));
	if (!changeFps && statusCode != RequestStatus::MissingRequestParameter)
		return RequestResult::Error(statusCode, comment);

	bool changeBaseRes = (request.ValidateNumber("baseWidth", statusCode, comment, 8, 4096) && request.ValidateNumber("baseHeight", statusCode, comment, 8, 4096));
	if (!changeBaseRes && statusCode != RequestStatus::MissingRequestParameter)
		return RequestResult::Error(statusCode, comment);

	bool changeOutputRes = (request.ValidateNumber("outputWidth", statusCode, comment, 8, 4096) && request.ValidateNumber("outputHeight", statusCode, comment, 8, 4096));
	if (!changeOutputRes && statusCode != RequestStatus::MissingRequestParameter)
		return RequestResult::Error(statusCode, comment);

	config_t *config = obs_frontend_get_profile_config();

	if (changeFps) {
		config_set_uint(config, "Video", "FPSType", 2);
		config_set_uint(config, "Video", "FPSNum", request.RequestData["fpsNumerator"]);
		config_set_uint(config, "Video", "FPSDen", request.RequestData["fpsDenominator"]);
	}

	if (changeBaseRes) {
		config_set_uint(config, "Video", "BaseCX", request.RequestData["baseWidth"]);
		config_set_uint(config, "Video", "BaseCY", request.RequestData["baseHeight"]);
	}

	if (changeOutputRes) {
		config_set_uint(config, "Video", "OutputCX", request.RequestData["outputWidth"]);
		config_set_uint(config, "Video", "OutputCY", request.RequestData["outputHeight"]);
	}

	if (changeFps || changeBaseRes || changeOutputRes) {
		config_save_safe(config, "tmp", nullptr);
		obs_frontend_reset_video();
		return RequestResult::Success();
	}

	return RequestResult::Error(RequestStatus::MissingRequestParameter, "You must specify at least one video-changing pair.");
}
