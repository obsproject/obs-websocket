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
