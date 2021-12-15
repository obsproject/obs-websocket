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

#include <inttypes.h>
#include <QString>
#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <util/util_uint64.h>

#include "Obs.h"
#include "../obs-websocket.h"
#include "../plugin-macros.generated.h"

#define CASE(x) case x: return #x;

#define RET_COMPARE(str, x) if (str == #x) return x;

std::vector<std::string> ConvertStringArray(char **array)
{
	std::vector<std::string> ret;
	if (!array)
		return ret;

	size_t index = 0;
	char* value = nullptr;
	do {
		value = array[index];
		if (value)
			ret.push_back(value);
		index++;
	} while (value);

	return ret;
}

std::string Utils::Obs::StringHelper::GetObsVersion()
{
	uint32_t version = obs_get_version();

	uint8_t major, minor, patch;
	major = (version >> 24) & 0xFF;
	minor = (version >> 16) & 0xFF;
	patch = version & 0xFF;

	QString combined = QString("%1.%2.%3").arg(major).arg(minor).arg(patch);
	return combined.toStdString();
}

std::string Utils::Obs::StringHelper::GetCurrentSceneCollection()
{
	char *sceneCollectionName = obs_frontend_get_current_scene_collection();
	std::string ret = sceneCollectionName;
	bfree(sceneCollectionName);
	return ret;
}

std::string Utils::Obs::StringHelper::GetCurrentProfile()
{
	char *profileName = obs_frontend_get_current_profile();
	std::string ret = profileName;
	bfree(profileName);
	return ret;
}

std::string Utils::Obs::StringHelper::GetCurrentProfilePath()
{
	char *profilePath = obs_frontend_get_current_profile_path();
	std::string ret = profilePath;
	bfree(profilePath);
	return ret;
}

std::string Utils::Obs::StringHelper::GetCurrentRecordOutputPath()
{
	//char *recordOutputPath = obs_frontend_get_current_record_output_path();
	//std::string ret = recordOutputPath;
	//bfree(recordOutputPath);
	//return ret;

	return "";
}

std::string Utils::Obs::StringHelper::GetSourceType(obs_source_t *source)
{
	obs_source_type sourceType = obs_source_get_type(source);

	switch (sourceType) {
		default:
		CASE(OBS_SOURCE_TYPE_INPUT)
		CASE(OBS_SOURCE_TYPE_FILTER)
		CASE(OBS_SOURCE_TYPE_TRANSITION)
		CASE(OBS_SOURCE_TYPE_SCENE)
	}
}

std::string Utils::Obs::StringHelper::GetInputMonitorType(obs_source_t *input)
{
	obs_monitoring_type monitorType = obs_source_get_monitoring_type(input);

	switch (monitorType) {
		default:
		CASE(OBS_MONITORING_TYPE_NONE)
		CASE(OBS_MONITORING_TYPE_MONITOR_ONLY)
		CASE(OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT)
	}
}

std::string Utils::Obs::StringHelper::GetMediaInputState(obs_source_t *input)
{
	obs_media_state mediaState = obs_source_media_get_state(input);

	switch (mediaState) {
		default:
		CASE(OBS_MEDIA_STATE_NONE)
		CASE(OBS_MEDIA_STATE_PLAYING)
		CASE(OBS_MEDIA_STATE_OPENING)
		CASE(OBS_MEDIA_STATE_BUFFERING)
		CASE(OBS_MEDIA_STATE_PAUSED)
		CASE(OBS_MEDIA_STATE_STOPPED)
		CASE(OBS_MEDIA_STATE_ENDED)
		CASE(OBS_MEDIA_STATE_ERROR)
	}
}

std::string Utils::Obs::StringHelper::GetLastReplayBufferFilePath()
{
	OBSOutputAutoRelease output = obs_frontend_get_replay_buffer_output();
	calldata_t cd = {0};
	proc_handler_t *ph = obs_output_get_proc_handler(output);
	proc_handler_call(ph, "get_last_replay", &cd);
	auto ret = calldata_string(&cd, "path");
	calldata_free(&cd);
	return ret;
}

std::string Utils::Obs::StringHelper::GetSceneItemBoundsType(enum obs_bounds_type type)
{
	switch (type) {
		default:
		CASE(OBS_BOUNDS_NONE)
		CASE(OBS_BOUNDS_STRETCH)
		CASE(OBS_BOUNDS_SCALE_INNER)
		CASE(OBS_BOUNDS_SCALE_OUTER)
		CASE(OBS_BOUNDS_SCALE_TO_WIDTH)
		CASE(OBS_BOUNDS_SCALE_TO_HEIGHT)
		CASE(OBS_BOUNDS_MAX_ONLY)
	}
}

std::string Utils::Obs::StringHelper::DurationToTimecode(uint64_t ms)
{
	uint64_t secs = ms / 1000ULL;
	uint64_t minutes = secs / 60ULL;

	uint64_t hoursPart = minutes / 60ULL;
	uint64_t minutesPart = minutes % 60ULL;
	uint64_t secsPart = secs % 60ULL;
	uint64_t msPart = ms % 1000ULL;

	QString formatted = QString::asprintf("%02" PRIu64 ":%02" PRIu64 ":%02" PRIu64 ".%03" PRIu64, hoursPart, minutesPart, secsPart, msPart);
	return formatted.toStdString();
}

enum obs_bounds_type Utils::Obs::EnumHelper::GetSceneItemBoundsType(std::string boundsType)
{
	RET_COMPARE(boundsType, OBS_BOUNDS_NONE);
	RET_COMPARE(boundsType, OBS_BOUNDS_STRETCH);
	RET_COMPARE(boundsType, OBS_BOUNDS_SCALE_INNER);
	RET_COMPARE(boundsType, OBS_BOUNDS_SCALE_OUTER);
	RET_COMPARE(boundsType, OBS_BOUNDS_SCALE_TO_WIDTH);
	RET_COMPARE(boundsType, OBS_BOUNDS_SCALE_TO_HEIGHT);
	RET_COMPARE(boundsType, OBS_BOUNDS_MAX_ONLY);

	return OBS_BOUNDS_NONE;
}

enum ObsMediaInputAction Utils::Obs::EnumHelper::GetMediaInputAction(std::string mediaAction)
{
	RET_COMPARE(mediaAction, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PLAY);
	RET_COMPARE(mediaAction, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PAUSE);
	RET_COMPARE(mediaAction, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_STOP);
	RET_COMPARE(mediaAction, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_RESTART);
	RET_COMPARE(mediaAction, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_NEXT);
	RET_COMPARE(mediaAction, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PREVIOUS);

	return OBS_WEBSOCKET_MEDIA_INPUT_ACTION_NONE;
}

uint64_t Utils::Obs::NumberHelper::GetOutputDuration(obs_output_t *output)
{
	if (!output || !obs_output_active(output))
		return 0;

	video_t* video = obs_output_video(output);
	uint64_t frameTimeNs = video_output_get_frame_time(video);
	int totalFrames = obs_output_get_total_frames(output);

	return util_mul_div64(totalFrames, frameTimeNs, 1000000ULL);
}

size_t Utils::Obs::NumberHelper::GetSceneCount()
{
	size_t ret;
	auto sceneEnumProc = [](void *param, obs_source_t *scene) {
		auto ret = reinterpret_cast<size_t*>(param);

		if (obs_source_is_group(scene))
			return true;

		(*ret)++;
		return true;
	};

	obs_enum_scenes(sceneEnumProc, &ret);

	return ret;
}

std::vector<std::string> Utils::Obs::ListHelper::GetSceneCollectionList()
{
	char** sceneCollections = obs_frontend_get_scene_collections();
	auto ret = ConvertStringArray(sceneCollections);
	bfree(sceneCollections);
	return ret;
}

std::vector<std::string> Utils::Obs::ListHelper::GetProfileList()
{
	char** profiles = obs_frontend_get_profiles();
	auto ret = ConvertStringArray(profiles);
	bfree(profiles);
	return ret;
}

std::vector<obs_hotkey_t *> Utils::Obs::ListHelper::GetHotkeyList()
{
	std::vector<obs_hotkey_t *> ret;

	obs_enum_hotkeys([](void* data, obs_hotkey_id, obs_hotkey_t* hotkey) {
		auto ret = reinterpret_cast<std::vector<obs_hotkey_t *> *>(data);

		ret->push_back(hotkey);

		return true;
	}, &ret);

	return ret;
}

std::vector<std::string> Utils::Obs::ListHelper::GetHotkeyNameList()
{
	auto hotkeys = GetHotkeyList();

	std::vector<std::string> ret;
	for (auto hotkey : hotkeys) {
		ret.emplace_back(obs_hotkey_get_name(hotkey));
	}

	return ret;
}

std::vector<json> Utils::Obs::ListHelper::GetSceneList()
{
	obs_frontend_source_list sceneList = {};
	obs_frontend_get_scenes(&sceneList);

	std::vector<json> ret;
	for (size_t i = 0; i < sceneList.sources.num; i++) {
		obs_source_t *scene = sceneList.sources.array[i];

		if (obs_source_is_group(scene))
			continue;

		json sceneJson;
		sceneJson["sceneName"] = obs_source_get_name(scene);
		sceneJson["sceneIndex"] = sceneList.sources.num - i - 1;

		ret.push_back(sceneJson);
	}

	obs_frontend_source_list_free(&sceneList);

	// Reverse the vector order to match other array returns
	std::reverse(ret.begin(), ret.end());

	return ret;
}

std::vector<json> Utils::Obs::ListHelper::GetSceneItemList(obs_scene_t *scene, bool basic)
{
	std::pair<std::vector<json>, bool> enumData;
	enumData.second = basic;

	obs_scene_enum_items(scene, [](obs_scene_t*, obs_sceneitem_t* sceneItem, void* param) {
		auto enumData = reinterpret_cast<std::pair<std::vector<json>, bool>*>(param);

		json item;
		item["sceneItemId"] = obs_sceneitem_get_id(sceneItem);
		// Should be slightly faster than calling obs_sceneitem_get_order_position()
		item["sceneItemIndex"] = enumData->first.size();
		if (!enumData->second) {
			OBSSource itemSource = obs_sceneitem_get_source(sceneItem);
			item["sourceName"] = obs_source_get_name(itemSource);
			item["sourceType"] = StringHelper::GetSourceType(itemSource);
			if (obs_source_get_type(itemSource) == OBS_SOURCE_TYPE_INPUT)
				item["inputKind"] = obs_source_get_id(itemSource);
			else
				item["inputKind"] = nullptr;
			if (obs_source_get_type(itemSource) == OBS_SOURCE_TYPE_SCENE)
				item["isGroup"] = obs_source_is_group(itemSource);
			else
				item["isGroup"] = nullptr;
		}

		enumData->first.push_back(item);

		return true;
	}, &enumData);

	return enumData.first;
}

std::vector<json> Utils::Obs::ListHelper::GetTransitionList()
{
	obs_frontend_source_list transitionList = {};
	obs_frontend_get_transitions(&transitionList);

	std::vector<json> ret;
	for (size_t i = 0; i < transitionList.sources.num; i++) {
		obs_source_t *transition = transitionList.sources.array[i];
		json transitionJson;
		transitionJson["transitionName"] = obs_source_get_name(transition);
		transitionJson["transitionKind"] = obs_source_get_id(transition);
		transitionJson["transitionFixed"] = obs_transition_fixed(transition);
		ret.push_back(transitionJson);
	}

	obs_frontend_source_list_free(&transitionList);

	return ret;
}

struct EnumInputInfo {
	std::string inputKind; // For searching by input kind
	std::vector<json> inputs;
};

std::vector<json> Utils::Obs::ListHelper::GetInputList(std::string inputKind)
{
	EnumInputInfo inputInfo;
	inputInfo.inputKind = inputKind;

	auto inputEnumProc = [](void *param, obs_source_t *input) {
		// Sanity check in case the API changes
		if (obs_source_get_type(input) != OBS_SOURCE_TYPE_INPUT)
			return true;

		auto inputInfo = reinterpret_cast<EnumInputInfo*>(param);

		std::string inputKind = obs_source_get_id(input);

		if (!inputInfo->inputKind.empty() && inputInfo->inputKind != inputKind)
			return true;

		json inputJson;
		inputJson["inputName"] = obs_source_get_name(input);
		inputJson["inputKind"] = inputKind;
		inputJson["unversionedInputKind"] = obs_source_get_unversioned_id(input);

		inputInfo->inputs.push_back(inputJson);
		return true;
	};
	// Actually enumerates only public inputs, despite the name
	obs_enum_sources(inputEnumProc, &inputInfo);

	return inputInfo.inputs;
}

std::vector<std::string> Utils::Obs::ListHelper::GetInputKindList(bool unversioned, bool includeDisabled)
{
	std::vector<std::string> ret;

	size_t idx = 0;
	const char *kind;
	const char *unversioned_kind;
	while (obs_enum_input_types2(idx++, &kind, &unversioned_kind)) {
		uint32_t caps = obs_get_source_output_flags(kind);

		if (!includeDisabled && (caps & OBS_SOURCE_CAP_DISABLED) != 0)
			continue;

		if (unversioned)
			ret.push_back(unversioned_kind);
		else
			ret.push_back(kind);
	}

	return ret;
}

json Utils::Obs::DataHelper::GetStats()
{
	json ret;

	config_t* currentProfile = obs_frontend_get_profile_config();
	const char* outputMode = config_get_string(currentProfile, "Output", "Mode");
	const char* recordPath = strcmp(outputMode, "Advanced") ? config_get_string(currentProfile, "SimpleOutput", "FilePath") : config_get_string(currentProfile, "AdvOut", "RecFilePath");

	video_t* video = obs_get_video();

	ret["cpuUsage"] = os_cpu_usage_info_query(GetCpuUsageInfo());
	ret["memoryUsage"] = (double)os_get_proc_resident_size() / (1024.0 * 1024.0);
	ret["availableDiskSpace"] = (double)os_get_free_disk_space(recordPath) / (1024.0 * 1024.0);
	ret["activeFps"] = obs_get_active_fps();
	ret["averageFrameRenderTime"] = (double)obs_get_average_frame_time_ns() / 1000000.0;
	ret["renderSkippedFrames"] = obs_get_lagged_frames();
	ret["renderTotalFrames"] = obs_get_total_frames();
	ret["outputSkippedFrames"] = video_output_get_skipped_frames(video);
	ret["outputTotalFrames"] = video_output_get_total_frames(video);

	return ret;
}

json Utils::Obs::DataHelper::GetSceneItemTransform(obs_sceneitem_t *item)
{
	json ret;

	obs_transform_info osi;
	obs_sceneitem_crop crop;
	obs_sceneitem_get_info(item, &osi);
	obs_sceneitem_get_crop(item, &crop);

	OBSSource source = obs_sceneitem_get_source(item);
	float sourceWidth = float(obs_source_get_width(source));
	float sourceHeight = float(obs_source_get_height(source));

	ret["sourceWidth"] = sourceWidth;
	ret["sourceHeight"] = sourceHeight;

	ret["positionX"] = osi.pos.x;
	ret["positionY"] = osi.pos.y;

	ret["rotation"] = osi.rot;

	ret["scaleX"] = osi.scale.x;
	ret["scaleY"] = osi.scale.y;

	ret["width"] = osi.scale.x * sourceWidth;
	ret["height"] = osi.scale.y * sourceHeight;

	ret["alignment"] = osi.alignment;

	ret["boundsType"] = StringHelper::GetSceneItemBoundsType(osi.bounds_type);
	ret["boundsAlignment"] = osi.bounds_alignment;
	ret["boundsWidth"] = osi.bounds.x;
	ret["boundsHeight"] = osi.bounds.y;

	ret["cropLeft"] = int(crop.left);
	ret["cropRight"] = int(crop.right);
	ret["cropTop"] = int(crop.top);
	ret["cropBottom"] = int(crop.bottom);

	return ret;
}

obs_hotkey_t *Utils::Obs::SearchHelper::GetHotkeyByName(std::string name)
{
	if (name.empty())
		return nullptr;

	auto hotkeys = ListHelper::GetHotkeyList();

	for (auto hotkey : hotkeys) {
		if (obs_hotkey_get_name(hotkey) == name)
			return hotkey;
	}

	return nullptr;
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

struct CreateSceneItemData {
	obs_source_t *source; // In
	bool sceneItemEnabled; // In
	obs_transform_info *sceneItemTransform = nullptr; // In
	obs_sceneitem_crop *sceneItemCrop = nullptr; // In
	OBSSceneItem sceneItem; // Out
};

void CreateSceneItemHelper(void *_data, obs_scene_t *scene)
{
	auto *data = reinterpret_cast<CreateSceneItemData*>(_data);
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
