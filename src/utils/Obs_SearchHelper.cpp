/*
obs-websocket
Copyright (C) 2020-2021 Kyle Manning <tt2468@gmail.com>

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

#include "Obs.h"
#include "../plugin-macros.generated.h"

obs_hotkey_t *Utils::Obs::SearchHelper::GetHotkeyByName(std::string name)
{
	if (name.empty())
		return nullptr;

	auto hotkeys = ArrayHelper::GetHotkeyList();

	for (auto hotkey : hotkeys) {
		if (obs_hotkey_get_name(hotkey) == name)
			return hotkey;
	}

	return nullptr;
}

// Increments source ref. Use OBSSourceAutoRelease
obs_source_t *Utils::Obs::SearchHelper::GetSceneTransitionByName(std::string name)
{
	obs_frontend_source_list transitionList = {};
	obs_frontend_get_transitions(&transitionList);

	obs_source_t *ret = nullptr;
	for (size_t i = 0; i < transitionList.sources.num; i++) {
		obs_source_t *transition = transitionList.sources.array[i];
		if (obs_source_get_name(transition) == name) {
			ret = obs_source_get_ref(transition);
			break;
		}
	}

	obs_frontend_source_list_free(&transitionList);

	return ret;
}

// Increments item ref. Use OBSSceneItemAutoRelease
obs_sceneitem_t *Utils::Obs::SearchHelper::GetSceneItemByName(obs_scene_t *scene, std::string name)
{
	if (name.empty())
		return nullptr;

	// Finds first matching scene item in scene, search starts at index 0
	OBSSceneItem ret = obs_scene_find_source(scene, name.c_str());
	obs_sceneitem_addref(ret);

	return ret;
}
