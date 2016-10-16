#include "Utils.h"

bool enum_scene_items(obs_scene_t *scene, obs_sceneitem_t *currentItem, void *param) {
	obs_data_array_t *data = static_cast<obs_data_array *>(param);
	obs_data_array_push_back(data, Utils::GetSceneItemData(currentItem));
	return true;
}

obs_data_array_t* Utils::GetSceneItems(obs_source_t *source) {
	obs_data_array_t *items = obs_data_array_create();
	obs_scene_t *scene = obs_scene_from_source(source);

	obs_scene_enum_items(scene, enum_scene_items, items);

	return items;
}

obs_data_t* Utils::GetSceneItemData(obs_sceneitem_t *item) {
	if (!item) {
		return NULL;
	}

	vec2 pos;
	obs_sceneitem_get_pos(item, &pos);

	vec2 bounds;
	obs_sceneitem_get_bounds(item, &bounds);

	obs_data_t *data = obs_data_create();
	obs_data_set_string(data, "name", obs_source_get_name(obs_sceneitem_get_source(item)));
	obs_data_set_string(data, "type", obs_source_get_id(obs_sceneitem_get_source(item)));
	obs_data_set_double(data, "volume", obs_source_get_volume(obs_sceneitem_get_source(item)));
	obs_data_set_double(data, "x", pos.x);
	obs_data_set_double(data, "y", pos.y);
	obs_data_set_double(data, "cx", bounds.x);
	obs_data_set_double(data, "cy", bounds.y);
	obs_data_set_bool(data, "render", obs_sceneitem_visible(item));

	return data;
}

obs_data_array_t* Utils::GetScenes() {
	obs_frontend_source_list sceneList = {};
	obs_frontend_get_scenes(&sceneList);

	obs_data_array_t* scenes = obs_data_array_create();
	for (size_t i = 0; i < (&sceneList)->sources.num; i++) {
		obs_source_t* scene = (&sceneList)->sources.array[i];
		obs_data_array_push_back(scenes, GetSceneData(scene));
	}

	obs_frontend_source_list_free(&sceneList);

	return scenes;
}

obs_data_t* Utils::GetSceneData(obs_source *source) {
	obs_data_t* sceneData = obs_data_create();
	obs_data_set_string(sceneData, "name", obs_source_get_name(source));
	obs_data_set_array(sceneData, "sources", GetSceneItems(source));
	
	return sceneData;
}