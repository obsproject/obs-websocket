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
#include <obs-frontend-api.h>

#include "Json.h"

// Autorelease object definitions
inline void ___properties_dummy_addref(obs_properties_t*){}
using OBSPropertiesAutoDestroy = OBSRef<obs_properties_t*, ___properties_dummy_addref, obs_properties_destroy>;

#if !defined(OBS_AUTORELEASE)
inline void ___source_dummy_addref(obs_source_t*){}
inline void ___scene_dummy_addref(obs_scene_t*){}
inline void ___sceneitem_dummy_addref(obs_sceneitem_t*){}
inline void ___data_dummy_addref(obs_data_t*){}
inline void ___data_array_dummy_addref(obs_data_array_t*){}
inline void ___output_dummy_addref(obs_output_t*){}
inline void ___encoder_dummy_addref(obs_encoder_t *){}
inline void ___service_dummy_addref(obs_service_t *){}

inline void ___weak_source_dummy_addref(obs_weak_source_t*){}
inline void ___weak_output_dummy_addref(obs_weak_output_t *){}
inline void ___weak_encoder_dummy_addref(obs_weak_encoder_t *){}
inline void ___weak_service_dummy_addref(obs_weak_service_t *){}

using OBSSourceAutoRelease = OBSRef<obs_source_t*, ___source_dummy_addref, obs_source_release>;
using OBSSceneAutoRelease = OBSRef<obs_scene_t*, ___scene_dummy_addref, obs_scene_release>;
using OBSSceneItemAutoRelease = OBSRef<obs_sceneitem_t*, ___sceneitem_dummy_addref, obs_sceneitem_release>;
using OBSDataAutoRelease = OBSRef<obs_data_t*, ___data_dummy_addref, obs_data_release>;
using OBSDataArrayAutoRelease = OBSRef<obs_data_array_t*, ___data_array_dummy_addref, obs_data_array_release>;
using OBSOutputAutoRelease = OBSRef<obs_output_t*, ___output_dummy_addref, obs_output_release>;
using OBSEncoderAutoRelease = OBSRef<obs_encoder_t *, ___encoder_dummy_addref, obs_encoder_release>;
using OBSServiceAutoRelease = OBSRef<obs_service_t *, ___service_dummy_addref, obs_service_release>;

using OBSWeakSourceAutoRelease = OBSRef<obs_weak_source_t*, ___weak_source_dummy_addref, obs_weak_source_release>;
using OBSWeakOutputAutoRelease = OBSRef<obs_weak_output_t *, ___weak_output_dummy_addref, obs_weak_output_release>;
using OBSWeakEncoderAutoRelease = OBSRef<obs_weak_encoder_t *, ___weak_encoder_dummy_addref, obs_weak_encoder_release>;
using OBSWeakServiceAutoRelease = OBSRef<obs_weak_service_t *, ___weak_service_dummy_addref, obs_weak_service_release>;
#endif

template <typename T> T* GetCalldataPointer(const calldata_t *data, const char* name) {
	void *ptr = nullptr;
	calldata_get_ptr(data, name, &ptr);
	return static_cast<T*>(ptr);
}

enum ObsOutputState {
	OBS_WEBSOCKET_OUTPUT_UNKNOWN,
	OBS_WEBSOCKET_OUTPUT_STARTING,
	OBS_WEBSOCKET_OUTPUT_STARTED,
	OBS_WEBSOCKET_OUTPUT_STOPPING,
	OBS_WEBSOCKET_OUTPUT_STOPPED,
	OBS_WEBSOCKET_OUTPUT_RECONNECTING,
	OBS_WEBSOCKET_OUTPUT_PAUSED,
	OBS_WEBSOCKET_OUTPUT_RESUMED,
};

enum ObsMediaInputAction {
	/**
	* No action.
	*
	* @enumIdentifier OBS_WEBSOCKET_MEDIA_INPUT_ACTION_NONE
	* @enumType ObsMediaInputAction
	* @rpcVersion 1
	* @initialVersion 5.0.0
	* @api enums
	*/
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_NONE,
	/**
	* Play the media input.
	*
	* @enumIdentifier OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PLAY
	* @enumType ObsMediaInputAction
	* @rpcVersion 1
	* @initialVersion 5.0.0
	* @api enums
	*/
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PLAY,
	/**
	* Pause the media input.
	*
	* @enumIdentifier OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PAUSE
	* @enumType ObsMediaInputAction
	* @rpcVersion 1
	* @initialVersion 5.0.0
	* @api enums
	*/
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PAUSE,
	/**
	* Stop the media input.
	*
	* @enumIdentifier OBS_WEBSOCKET_MEDIA_INPUT_ACTION_STOP
	* @enumType ObsMediaInputAction
	* @rpcVersion 1
	* @initialVersion 5.0.0
	* @api enums
	*/
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_STOP,
	/**
	* Restart the media input.
	*
	* @enumIdentifier OBS_WEBSOCKET_MEDIA_INPUT_ACTION_RESTART
	* @enumType ObsMediaInputAction
	* @rpcVersion 1
	* @initialVersion 5.0.0
	* @api enums
	*/
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_RESTART,
	/**
	* Go to the next playlist item.
	*
	* @enumIdentifier OBS_WEBSOCKET_MEDIA_INPUT_ACTION_NEXT
	* @enumType ObsMediaInputAction
	* @rpcVersion 1
	* @initialVersion 5.0.0
	* @api enums
	*/
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_NEXT,
	/**
	* Go to the previous playlist item.
	*
	* @enumIdentifier OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PREVIOUS
	* @enumType ObsMediaInputAction
	* @rpcVersion 1
	* @initialVersion 5.0.0
	* @api enums
	*/
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PREVIOUS,
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
			std::string GetInputMonitorType(enum obs_monitoring_type monitorType);
			std::string GetInputMonitorType(obs_source_t *input);
			std::string GetMediaInputState(obs_source_t *input);
			std::string GetLastReplayBufferFilePath();
			std::string GetSceneItemBoundsType(enum obs_bounds_type type);
			std::string GetSceneItemBlendMode(enum obs_blending_type mode);
			std::string DurationToTimecode(uint64_t);
			std::string GetOutputState(ObsOutputState state);
		}

		namespace EnumHelper {
			enum obs_bounds_type GetSceneItemBoundsType(std::string boundsType);
			enum ObsMediaInputAction GetMediaInputAction(std::string mediaAction);
			enum obs_blending_type GetSceneItemBlendMode(std::string mode);
		}

		namespace NumberHelper {
			uint64_t GetOutputDuration(obs_output_t *output);
			size_t GetSceneCount();
			size_t GetSourceFilterIndex(obs_source_t *source, obs_source_t *filter);
		}

		namespace ArrayHelper {
			std::vector<std::string> GetSceneCollectionList();
			std::vector<std::string> GetProfileList();
			std::vector<obs_hotkey_t *> GetHotkeyList();
			std::vector<std::string> GetHotkeyNameList();
			std::vector<json> GetSceneList();
			std::vector<std::string> GetGroupList();
			std::vector<json> GetSceneItemList(obs_scene_t *scene, bool basic = false);
			std::vector<json> GetInputList(std::string inputKind = "");
			std::vector<std::string> GetInputKindList(bool unversioned = false, bool includeDisabled = false);
			std::vector<json> GetListPropertyItems(obs_property_t *property);
			std::vector<std::string> GetTransitionKindList();
			std::vector<json> GetSceneTransitionList();
		}

		namespace ObjectHelper {
			json GetStats();
			json GetSceneItemTransform(obs_sceneitem_t *item);
		}

		namespace SearchHelper {
			obs_hotkey_t *GetHotkeyByName(std::string name);
			obs_source_t *GetSceneTransitionByName(std::string name); // Increments source ref. Use OBSSourceAutoRelease
			obs_sceneitem_t *GetSceneItemByName(obs_scene_t *scene, std::string name); // Increments ref. Use OBSSceneItemAutoRelease
		}

		namespace ActionHelper {
			obs_sceneitem_t *CreateSceneItem(obs_source_t *source, obs_scene_t *scene, bool sceneItemEnabled = true, obs_transform_info *sceneItemTransform = nullptr, obs_sceneitem_crop *sceneItemCrop = nullptr); // Increments ref. Use OBSSceneItemAutoRelease
			obs_sceneitem_t *CreateInput(std::string inputName, std::string inputKind, obs_data_t *inputSettings, obs_scene_t *scene, bool sceneItemEnabled = true); // Increments ref. Use OBSSceneItemAutoRelease
		}
	}
}
