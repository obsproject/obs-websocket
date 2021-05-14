#include <obs.hpp>
#include <obs-frontend-api.h>

#include "Utils.h"

#include "../plugin-macros.generated.h"

#define CASE(x) case x: return #x;

std::vector<std::string> ConvertStringArray(char **array)
{
	std::vector<std::string> ret;
	if (!array)
		return ret;

	size_t index = 0;
	char* value = nullptr;
	do {
		value = array[index];
		if (value)
			ret.push_back(value);
		index++;
	} while (value);

	return ret;
}

std::string Utils::Obs::DataHelper::GetSourceTypeString(obs_source_t *source)
{
	obs_source_type sourceType = obs_source_get_type(source);

	switch (sourceType) {
		default:
		CASE(OBS_SOURCE_TYPE_INPUT)
		CASE(OBS_SOURCE_TYPE_FILTER)
		CASE(OBS_SOURCE_TYPE_TRANSITION)
		CASE(OBS_SOURCE_TYPE_SCENE)
	}
}

std::string Utils::Obs::DataHelper::GetSourceMonitorTypeString(obs_source_t *source)
{
	obs_monitoring_type monitorType = obs_source_get_monitoring_type(source);

	switch (monitorType) {
		default:
		CASE(OBS_MONITORING_TYPE_NONE)
		CASE(OBS_MONITORING_TYPE_MONITOR_ONLY)
		CASE(OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT)
	}
}

std::string Utils::Obs::DataHelper::GetSourceMediaStateString(obs_source_t *source)
{
	obs_media_state mediaState = obs_source_media_get_state(source);

	switch (mediaState) {
		default:
		CASE(OBS_MEDIA_STATE_NONE)
		CASE(OBS_MEDIA_STATE_PLAYING)
		CASE(OBS_MEDIA_STATE_OPENING)
		CASE(OBS_MEDIA_STATE_BUFFERING)
		CASE(OBS_MEDIA_STATE_PAUSED)
		CASE(OBS_MEDIA_STATE_STOPPED)
		CASE(OBS_MEDIA_STATE_ENDED)
		CASE(OBS_MEDIA_STATE_ERROR)
	}
}

std::vector<std::string> Utils::Obs::ListHelper::GetSceneCollectionList()
{
	char** sceneCollections = obs_frontend_get_scene_collections();
	auto ret = ConvertStringArray(sceneCollections);
	bfree(sceneCollections);
	return ret;
}

std::vector<std::string> Utils::Obs::ListHelper::GetProfileList()
{
	char** profiles = obs_frontend_get_profiles();
	auto ret = ConvertStringArray(profiles);
	bfree(profiles);
	return ret;
}

std::vector<json> Utils::Obs::ListHelper::GetSceneList()
{
	obs_frontend_source_list sceneList = {};
	obs_frontend_get_scenes(&sceneList);

	std::vector<json> ret;
	for (size_t i = 0; i < sceneList.sources.num; i++) {
		json sceneJson;
		obs_source_t *scene = sceneList.sources.array[i];
		sceneJson["sceneName"] = obs_source_get_name(scene);
		sceneJson["sceneIndex"] = i;
		ret.push_back(sceneJson);
	}

	obs_frontend_source_list_free(&sceneList);

	return ret;
}
