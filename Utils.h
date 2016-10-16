#ifndef UTILS_H
#define UTILS_H

#include <obs-module.h>
#include <obs-frontend-api.h>

class Utils
{
	public:
		static obs_data_array_t* GetSceneItems(obs_source_t* source);
		static obs_data_t* GetSceneItemData(obs_scene_item *item);

		static obs_data_array_t* GetScenes();
		static obs_data_t* GetSceneData(obs_source *source);
};

#endif // UTILS_H