#pragma once

#include <string>
#include <obs.hpp>

#include "Json.h"

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
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PAUSE,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PLAY,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_RESTART,
	OBS_WEBSOCKET_MEDIA_INPUT_ACTION_STOP,
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