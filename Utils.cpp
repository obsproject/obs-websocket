#include "Utils.h"

obs_data_array_t* Utils::GetSceneItems(obs_source_t *source) {
	obs_data_array *items = obs_data_array_create();
	obs_scene *scene = obs_scene_from_source(source);

	/*obs_scene_item *currentItem = scene->first_item;
	while (currentItem != NULL) {
		obs_data_array_push_back(items, GetSceneItemData(currentItem));
		currentItem = currentItem->next;
	}*/

	return items;
}

obs_data_t* Utils::GetSceneItemData(obs_scene_item *item) {
	if (!item) {
		return NULL;
	}

	obs_data_t *data = obs_data_create();
	/*obs_data_set_string(data, "name", obs_source_get_name(item->source));
	obs_data_set_double(data, "x", item->pos.x);
	obs_data_set_double(data, "y", item->pos.y);
	obs_data_set_double(data, "cx", item->bounds.x);
	obs_data_set_double(data, "cy", item->bounds.y);
	obs_data_set_bool(data, "render", item->visible);*/

	return data;
}