/*
obs-websocket
Copyright (C) 2016	Stéphane Lepin <stephane.lepin@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include "Utils.h"

obs_data_array_t* Utils::GetSceneItems(obs_source_t *source) {
	obs_data_array_t *items = obs_data_array_create();
	obs_scene_t *scene = obs_scene_from_source(source);
	if (scene == NULL) {
		return NULL;
	}

	obs_scene_enum_items(scene, [](obs_scene_t *scene, obs_sceneitem_t *currentItem, void *param) {
		obs_data_array_t *data = static_cast<obs_data_array_t *>(param);
		
		obs_data_t *item_data = GetSceneItemData(currentItem);
		obs_data_array_insert(data, 0, item_data);

		obs_data_release(item_data);
		return true;
	}, items);

	return items;
}

obs_data_t* Utils::GetSceneItemData(obs_sceneitem_t *item) {
	if (!item) {
		return NULL;
	}

	vec2 pos;
	obs_sceneitem_get_pos(item, &pos);

	vec2 scale;
	obs_sceneitem_get_scale(item, &scale);

	obs_source_t* item_source = obs_sceneitem_get_source(item);
	float item_width = float(obs_source_get_width(item_source));
	float item_height = float(obs_source_get_height(item_source));

	obs_data_t *data = obs_data_create();
	obs_data_set_string(data, "name", obs_source_get_name(obs_sceneitem_get_source(item)));
	obs_data_set_string(data, "type", obs_source_get_id(obs_sceneitem_get_source(item)));
	obs_data_set_double(data, "volume", obs_source_get_volume(obs_sceneitem_get_source(item)));
	obs_data_set_double(data, "x", pos.x);
	obs_data_set_double(data, "y", pos.y);
	obs_data_set_double(data, "cx", item_width * scale.x);
	obs_data_set_double(data, "cy", item_height * scale.y);
	obs_data_set_bool(data, "render", obs_sceneitem_visible(item));

	return data;
}

obs_sceneitem_t* Utils::GetSceneItemFromName(obs_source_t* source, const char* name) {
	struct current_search {
		const char* query;
		obs_sceneitem_t* result;
	};

	current_search search;
	search.query = name;
	search.result = NULL;

	obs_scene_t *scene = obs_scene_from_source(source);
	if (scene == NULL) {
		return NULL;
	}

	obs_scene_enum_items(scene, [](obs_scene_t *scene, obs_sceneitem_t *currentItem, void *param) {
		current_search *search = static_cast<current_search *>(param);
		
		const char* currentItemName = obs_source_get_name(obs_sceneitem_get_source(currentItem));
		if (strcmp(currentItemName, search->query) == 0) {
			search->result = currentItem;
			obs_sceneitem_addref(search->result);
			return false;
		}

		return true;
	}, &search);

	return search.result;
}

obs_source_t* Utils::GetTransitionFromName(const char *search_name) {
	obs_source_t *found_transition = NULL;

	obs_frontend_source_list transition_list = {};
	obs_frontend_get_transitions(&transition_list);

	for (size_t i = 0; i < transition_list.sources.num; i++) {
		obs_source_t *transition = transition_list.sources.array[i];

		const char *transition_name = obs_source_get_name(transition);
		if (strcmp(transition_name, search_name) == 0) {
			found_transition = transition;
			obs_source_addref(found_transition);
			break;
		}
	}

	obs_frontend_source_list_free(&transition_list);

	return found_transition;
}

obs_data_array_t* Utils::GetScenes() {
	obs_frontend_source_list sceneList = {};
	obs_frontend_get_scenes(&sceneList);

	obs_data_array_t* scenes = obs_data_array_create();
	for (size_t i = 0; i < sceneList.sources.num; i++) {
		obs_source_t *scene = sceneList.sources.array[i];
		
		obs_data_t *scene_data = GetSceneData(scene);
		obs_data_array_push_back(scenes, scene_data);

		obs_data_release(scene_data);
	}

	obs_frontend_source_list_free(&sceneList); 

	return scenes;
}

obs_data_t* Utils::GetSceneData(obs_source *source) {
	obs_data_array_t *scene_items = GetSceneItems(source);

	obs_data_t* sceneData = obs_data_create();
	obs_data_set_string(sceneData, "name", obs_source_get_name(source));
	obs_data_set_array(sceneData, "sources", scene_items);
	
	obs_data_array_release(scene_items);
	return sceneData;
}
