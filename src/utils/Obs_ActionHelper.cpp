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

struct CreateSceneItemData {
	obs_source_t *source; // In
	bool sceneItemEnabled; // In
	obs_transform_info *sceneItemTransform = nullptr; // In
	obs_sceneitem_crop *sceneItemCrop = nullptr; // In
	OBSSceneItem sceneItem; // Out
};

void CreateSceneItemHelper(void *_data, obs_scene_t *scene)
{
	auto *data = static_cast<CreateSceneItemData*>(_data);
	data->sceneItem = obs_scene_add(scene, data->source);

	if (data->sceneItemTransform)
		obs_sceneitem_set_info(data->sceneItem, data->sceneItemTransform);

	if (data->sceneItemCrop)
		obs_sceneitem_set_crop(data->sceneItem, data->sceneItemCrop);

	obs_sceneitem_set_visible(data->sceneItem, data->sceneItemEnabled);
}

obs_sceneitem_t *Utils::Obs::ActionHelper::CreateSceneItem(obs_source_t *source, obs_scene_t *scene, bool sceneItemEnabled, obs_transform_info *sceneItemTransform, obs_sceneitem_crop *sceneItemCrop)
{
	// Sanity check for valid scene
	if (!(source && scene))
		return nullptr;

	// Create data struct and populate for scene item creation
	CreateSceneItemData data;
	data.source = source;
	data.sceneItemEnabled = sceneItemEnabled;
	data.sceneItemTransform = sceneItemTransform;
	data.sceneItemCrop = sceneItemCrop;

	// Enter graphics context and create the scene item
	obs_enter_graphics();
	obs_scene_atomic_update(scene, CreateSceneItemHelper, &data);
	obs_leave_graphics();

	obs_sceneitem_addref(data.sceneItem);

	return data.sceneItem;
}

obs_sceneitem_t *Utils::Obs::ActionHelper::CreateInput(std::string inputName, std::string inputKind, obs_data_t *inputSettings, obs_scene_t *scene, bool sceneItemEnabled)
{
	// Create the input
	OBSSourceAutoRelease input = obs_source_create(inputKind.c_str(), inputName.c_str(), inputSettings, nullptr);

	// Check that everything was created properly
	if (!input)
		return nullptr;

	// Apparently not all default input properties actually get applied on creation (smh)
	uint32_t flags = obs_source_get_output_flags(input);
	if ((flags & OBS_SOURCE_MONITOR_BY_DEFAULT) != 0)
		obs_source_set_monitoring_type(input, OBS_MONITORING_TYPE_MONITOR_ONLY);

	// Create a scene item for the input
	obs_sceneitem_t *ret = CreateSceneItem(input, scene, sceneItemEnabled);

	// If creation failed, remove the input
	if (!ret)
		obs_source_remove(input);

	return ret;
}
