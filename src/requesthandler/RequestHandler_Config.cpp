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
	if (!request.ValidateString("sceneCollectionName", statusCode, comment)) {
		return RequestResult::Error(statusCode, comment);
	}

	std::string currentSceneCollectionName = Utils::Obs::StringHelper::GetCurrentSceneCollection();
	std::string sceneCollectionName = request.RequestData["sceneCollectionName"];

	auto sceneCollections = Utils::Obs::ListHelper::GetSceneCollectionList();
	if (std::find(sceneCollections.begin(), sceneCollections.end(), sceneCollectionName) == sceneCollections.end())
		return RequestResult::Error(RequestStatus::SceneCollectionNotFound, "Your specified scene collection was not found.");

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
	if (!request.ValidateString("profileName", statusCode, comment)) {
		return RequestResult::Error(statusCode, comment);
	}

	std::string currentProfileName = Utils::Obs::StringHelper::GetCurrentProfile();
	std::string profileName = request.RequestData["profileName"];

	auto profiles = Utils::Obs::ListHelper::GetProfileList();
	if (std::find(profiles.begin(), profiles.end(), profileName) == profiles.end())
		return RequestResult::Error(RequestStatus::ProfileNotFound, "Your specified profile was not found.");

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
	if (!request.ValidateString("parameterCategory", statusCode, comment) || !request.ValidateString("parameterName", statusCode, comment)) {
		return RequestResult::Error(statusCode, comment);
	}

	std::string parameterCategory = request.RequestData["parameterCategory"];
	std::string parameterName = request.RequestData["parameterName"];

	config_t* profile = obs_frontend_get_profile_config();

	json responseData;
	if (config_has_default_value(profile, parameterCategory.c_str(), parameterName.c_str())) {
		responseData["parameterValue"] = 
		responseData["defaultParameterValue"] = config_get_default_string(profile, parameterCategory.c_str(), parameterName.c_str());
	} else {
		if (config_has_user_value(profile, parameterCategory.c_str(), parameterName.c_str()))
			responseData["parameterValue"] = config_get_string(profile, parameterCategory.c_str(), parameterName.c_str());
		else
			responseData["parameterValue"] = nullptr;
		responseData["defaultParameterValue"] = nullptr;
	}

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetProfileParameter(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateString("parameterCategory", statusCode, comment) ||
	!request.ValidateString("parameterName", statusCode, comment)) {
		return RequestResult::Error(statusCode, comment);
	}

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
