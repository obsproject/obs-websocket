/*
obs-websocket
Copyright (C) 2016-2021 Stephane Lepin <stephane.lepin@gmail.com>
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

#pragma once

#include <string>
#include <obs.hpp>

#include "Json.h"

#if !defined(OBSSourceAutoRelease)
#define OBS_HAS_AUTORELEASE
// Autorelease object definitions
void ___source_dummy_addref(obs_source_t*);
void ___weak_source_dummy_addref(obs_weak_source_t*);
void ___scene_dummy_addref(obs_scene_t*);
void ___sceneitem_dummy_addref(obs_sceneitem_t*);
void ___data_dummy_addref(obs_data_t*);
void ___data_array_dummy_addref(obs_data_array_t*);
void ___output_dummy_addref(obs_output_t*);
void ___data_item_dummy_addref(obs_data_item_t*);
void ___data_item_release(obs_data_item_t*);
void ___properties_dummy_addref(obs_properties_t*);

using OBSSourceAutoRelease = OBSRef<obs_source_t*, ___source_dummy_addref, obs_source_release>;
using OBSWeakSourceAutoRelease = OBSRef<obs_weak_source_t*, ___weak_source_dummy_addref, obs_weak_source_release>;
using OBSSceneAutoRelease = OBSRef<obs_scene_t*, ___scene_dummy_addref, obs_scene_release>;
using OBSSceneItemAutoRelease = OBSRef<obs_sceneitem_t*, ___sceneitem_dummy_addref, obs_sceneitem_release>;
using OBSDataAutoRelease = OBSRef<obs_data_t*, ___data_dummy_addref, obs_data_release>;
using OBSDataArrayAutoRelease = OBSRef<obs_data_array_t*, ___data_array_dummy_addref, obs_data_array_release>;
using OBSOutputAutoRelease = OBSRef<obs_output_t*, ___output_dummy_addref, obs_output_release>;
using OBSDataItemAutoRelease = OBSRef<obs_data_item_t*, ___data_item_dummy_addref, ___data_item_release>;
using OBSPropertiesAutoDestroy = OBSRef<obs_properties_t*, ___properties_dummy_addref, obs_properties_destroy>;
#endif

template <typename T> T* GetCalldataPointer(const calldata_t *data, const char* name) {
	void *ptr = nullptr;
	calldata_get_ptr(data, name, &ptr);
	return reinterpret_cast<T*>(ptr);
}

enum ObsOutputState {
	OBS_WEBSOCKET_OUTPUT_STARTING,
	OBS_WEBSOCKET_OUTPUT_STARTED,
	OBS_WEBSOCKET_OUTPUT_STOPPING,
	OBS_WEBSOCKET_OUTPUT_STOPPED,
	OBS_WEBSOCKET_OUTPUT_RECONNECTING,
	OBS_WEBSOCKET_OUTPUT_PAUSED,
	OBS_WEBSOCKET_OUTPUT_RESUMED
};

enum ObsMediaInputAction {
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_NONE,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PLAY,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PAUSE,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_STOP,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_RESTART,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_NEXT,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PREVIOUS
};

namespace Utils {
	namespace Obs {
		namespace StringHelper {
			std::string GetObsVersion();
			std::string GetCurrentSceneCollection();
			std::string GetCurrentProfile();
			std::string GetCurrentProfilePath();
			std::string GetCurrentRecordOutputPath();
			std::string GetSourceType(obs_source_t *source);
			std::string GetInputMonitorType(obs_source_t *input);
			std::string GetMediaInputState(obs_source_t *input);
			std::string GetLastReplayBufferFilePath();
			std::string GetSceneItemBoundsType(enum obs_bounds_type type);
			std::string DurationToTimecode(uint64_t);
		}

		namespace EnumHelper {
			enum obs_bounds_type GetSceneItemBoundsType(std::string boundsType);
			enum ObsMediaInputAction GetMediaInputAction(std::string mediaAction);
		}

		namespace NumberHelper {
			uint64_t GetOutputDuration(obs_output_t *output);
			size_t GetSceneCount();
		}

		namespace ListHelper {
			std::vector<std::string> GetSceneCollectionList();
			std::vector<std::string> GetProfileList();
			std::vector<obs_hotkey_t *> GetHotkeyList();
			std::vector<std::string> GetHotkeyNameList();
			std::vector<json> GetSceneList();
			std::vector<json> GetSceneItemList(obs_scene_t *scene, bool basic = false);
			std::vector<json> GetTransitionList();
			std::vector<json> GetInputList(std::string inputKind = "");
			std::vector<std::string> GetInputKindList(bool unversioned = false, bool includeDisabled = false);
		}

		namespace DataHelper {
			json GetStats();
			json GetSceneItemTransform(obs_sceneitem_t *item);
		}

		namespace SearchHelper {
			obs_hotkey_t *GetHotkeyByName(std::string name);
			obs_sceneitem_t *GetSceneItemByName(obs_scene_t *scene, std::string name); // Increments ref. Use OBSSceneItemAutoRelease
		}

		namespace ActionHelper {
			obs_sceneitem_t *CreateSceneItem(obs_source_t *source, obs_scene_t *scene, bool sceneItemEnabled = true, obs_transform_info *sceneItemTransform = nullptr, obs_sceneitem_crop *sceneItemCrop = nullptr); // Increments ref. Use OBSSceneItemAutoRelease
			obs_sceneitem_t *CreateInput(std::string inputName, std::string inputKind, obs_data_t *inputSettings, obs_scene_t *scene, bool sceneItemEnabled = true); // Increments ref. Use OBSSceneItemAutoRelease
		}
	}
}
