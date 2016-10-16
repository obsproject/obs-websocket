#ifndef UTILS_H
#define UTILS_H

#include <obs-module.h>

class Utils
{
	public:
		static obs_data_array_t* GetSceneItems(obs_source_t* source);
		static obs_data_t* GetSceneItemData(obs_scene_item *item);
};

#endif // UTILS_H