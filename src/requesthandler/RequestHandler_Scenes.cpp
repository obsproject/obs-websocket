#include "RequestHandler.h"

#include "../plugin-macros.generated.h"

RequestResult RequestHandler::GetSceneList(const Request& request)
{
	json responseData;

	OBSSourceAutoRelease currentProgramScene = obs_frontend_get_current_scene();
	responseData["currentProgramSceneName"] = obs_source_get_name(currentProgramScene);


	OBSSourceAutoRelease currentPreviewScene = obs_frontend_get_current_preview_scene();
	if (currentPreviewScene)
		responseData["currentPreviewSceneName"] = obs_source_get_name(currentPreviewScene);
	else
		responseData["currentPreviewSceneName"] = nullptr;

	responseData["scenes"] = Utils::Obs::ListHelper::GetSceneList();

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::GetCurrentProgramScene(const Request& request)
{
	json responseData;

	OBSSourceAutoRelease currentProgramScene = obs_frontend_get_current_scene();
	responseData["currentProgramSceneName"] = obs_source_get_name(currentProgramScene);

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetCurrentProgramScene(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateString("sceneName", statusCode, comment)) {
		return RequestResult::Error(statusCode, comment);
	}

	std::string sceneName = request.RequestData["sceneName"];

	OBSSourceAutoRelease scene = obs_get_source_by_name(sceneName.c_str());
	if (!scene)
		return RequestResult::Error(RequestStatus::SceneNotFound);

	if (obs_source_get_type(scene) != OBS_SOURCE_TYPE_SCENE)
		return RequestResult::Error(RequestStatus::InvalidSourceType, "The specified source is not a scene.");

	obs_frontend_set_current_scene(scene);

	return RequestResult::Success();
}

RequestResult RequestHandler::GetCurrentPreviewScene(const Request& request)
{
	json responseData;

	OBSSourceAutoRelease currentPreviewScene = obs_frontend_get_current_preview_scene();
	if (currentPreviewScene) {
		responseData["currentPreviewSceneName"] = obs_source_get_name(currentPreviewScene);
	} else if (request.IgnoreNonFatalRequestChecks) {
		responseData["currentPreviewSceneName"] = nullptr;
	} else {
		return RequestResult::Error(RequestStatus::StudioModeNotActive);
	}

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetCurrentPreviewScene(const Request& request)
{
	if (!obs_frontend_preview_program_mode_active())
		return RequestResult::Error(RequestStatus::StudioModeNotActive);

	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateString("sceneName", statusCode, comment)) {
		return RequestResult::Error(statusCode, comment);
	}

	std::string sceneName = request.RequestData["sceneName"];

	OBSSourceAutoRelease scene = obs_get_source_by_name(sceneName.c_str());
	if (!scene)
		return RequestResult::Error(RequestStatus::SceneNotFound);

	if (obs_source_get_type(scene) != OBS_SOURCE_TYPE_SCENE)
		return RequestResult::Error(RequestStatus::InvalidSourceType, "The specified source is not a scene.");

	obs_frontend_set_current_preview_scene(scene);

	return RequestResult::Success();
}
