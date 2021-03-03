/*
obs-websocket
Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>

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
#include <QtWidgets/QMainWindow>
#include <QtCore/QDir>
#include <QtCore/QUrl>

#include <obs-frontend-api.h>
#include <obs.hpp>
#include <util/platform.h>
#include <obs-data.h>

#include "obs-websocket.h"

#include "Utils.h"
#include "Config.h"

Q_DECLARE_METATYPE(OBSScene);

const QHash<obs_bounds_type, QString> boundTypeNames = {
	{ OBS_BOUNDS_STRETCH, "OBS_BOUNDS_STRETCH" },
	{ OBS_BOUNDS_SCALE_INNER, "OBS_BOUNDS_SCALE_INNER" },
	{ OBS_BOUNDS_SCALE_OUTER, "OBS_BOUNDS_SCALE_OUTER" },
	{ OBS_BOUNDS_SCALE_TO_WIDTH, "OBS_BOUNDS_SCALE_TO_WIDTH" },
	{ OBS_BOUNDS_SCALE_TO_HEIGHT, "OBS_BOUNDS_SCALE_TO_HEIGHT" },
	{ OBS_BOUNDS_MAX_ONLY, "OBS_BOUNDS_MAX_ONLY" },
	{ OBS_BOUNDS_NONE, "OBS_BOUNDS_NONE" },
};

QString getBoundsNameFromType(obs_bounds_type type) {
	QString fallback = boundTypeNames.value(OBS_BOUNDS_NONE);
	return boundTypeNames.value(type, fallback);
}

obs_bounds_type getBoundsTypeFromName(QString name) {
	return boundTypeNames.key(name);
}

const QHash<obs_scale_type, QString> scaleTypeNames = {
	{ OBS_SCALE_DISABLE, "OBS_SCALE_DISABLE" },
	{ OBS_SCALE_POINT, "OBS_SCALE_POINT" },
	{ OBS_SCALE_BICUBIC, "OBS_SCALE_BICUBIC" },
	{ OBS_SCALE_BILINEAR, "OBS_SCALE_BILINEAR" },
	{ OBS_SCALE_LANCZOS, "OBS_SCALE_LANCZOS" },
	{ OBS_SCALE_AREA, "OBS_SCALE_AREA" },
};

QString getScaleNameFromType(obs_scale_type type) {
	QString fallback = scaleTypeNames.value(OBS_SCALE_DISABLE);
	return scaleTypeNames.value(type, fallback);
}

obs_scale_type getFilterTypeFromName(QString name) {
	return scaleTypeNames.key(name);
}

bool Utils::StringInStringList(char** strings, const char* string) {
	if (!strings) {
		return false;
	}

	size_t index = 0;
	while (strings[index] != NULL) {
		char* value = strings[index];

		if (strcmp(value, string) == 0) {
			return true;
		}

		index++;
	}

	return false;
}

obs_data_array_t* Utils::StringListToArray(char** strings, const char* key) {
	obs_data_array_t* list = obs_data_array_create();

	if (!strings || !key) {
		return list; // empty list
	}

	size_t index = 0;
	char* value = nullptr;

	do {
		value = strings[index];

		OBSDataAutoRelease item = obs_data_create();
		obs_data_set_string(item, key, value);

		if (value) {
			obs_data_array_push_back(list, item);
		}

		index++;
	} while (value != nullptr);

	return list;
}

obs_data_array_t* Utils::GetSceneItems(obs_source_t* source) {
	obs_data_array_t* items = obs_data_array_create();
	OBSScene scene = obs_scene_from_source(source);

	if (!scene) {
		return nullptr;
	}

	obs_scene_enum_items(scene, [](
			obs_scene_t* scene,
			obs_sceneitem_t* currentItem,
			void* param)
	{
		obs_data_array_t* data = reinterpret_cast<obs_data_array_t*>(param);

		OBSDataAutoRelease itemData = GetSceneItemData(currentItem);
		obs_data_array_insert(data, 0, itemData);
		return true;
	}, items);

	return items;
}

/**
 * @typedef {Object} `SceneItem` An OBS Scene Item.
 * @property {Number} `cy`
 * @property {Number} `cx`
 * @property {Number} `alignment` The point on the source that the item is manipulated from. The sum of 1=Left or 2=Right, and 4=Top or 8=Bottom, or omit to center on that axis.
 * @property {String} `name` The name of this Scene Item.
 * @property {int} `id` Scene item ID
 * @property {Boolean} `render` Whether or not this Scene Item is set to "visible".
 * @property {Boolean} `muted` Whether or not this Scene Item is muted.
 * @property {Boolean} `locked` Whether or not this Scene Item is locked and can't be moved around
 * @property {Number} `source_cx`
 * @property {Number} `source_cy`
 * @property {String} `type` Source type. Value is one of the following: "input", "filter", "transition", "scene" or "unknown"
 * @property {Number} `volume`
 * @property {Number} `x`
 * @property {Number} `y`
 * @property {String (optional)} `parentGroupName` Name of the item's parent (if this item belongs to a group)
 * @property {Array<SceneItem> (optional)} `groupChildren` List of children (if this item is a group)
 */
obs_data_t* Utils::GetSceneItemData(obs_sceneitem_t* item) {
	if (!item) {
		return nullptr;
	}

	vec2 pos;
	obs_sceneitem_get_pos(item, &pos);

	vec2 scale;
	obs_sceneitem_get_scale(item, &scale);

	// obs_sceneitem_get_source doesn't increase the refcount
	OBSSource itemSource = obs_sceneitem_get_source(item);
	float item_width = float(obs_source_get_width(itemSource));
	float item_height = float(obs_source_get_height(itemSource));

	obs_data_t* data = obs_data_create();
	obs_data_set_string(data, "name",
		obs_source_get_name(itemSource));
	obs_data_set_int(data, "id",
		obs_sceneitem_get_id(item));
	obs_data_set_string(data, "type",
		obs_source_get_id(itemSource));
	obs_data_set_double(data, "volume",
		obs_source_get_volume(itemSource));
	obs_data_set_double(data, "x", pos.x);
	obs_data_set_double(data, "y", pos.y);
	obs_data_set_int(data, "source_cx", (int)item_width);
	obs_data_set_int(data, "source_cy", (int)item_height);
	obs_data_set_bool(data, "muted", obs_source_muted(itemSource));
	obs_data_set_int(data, "alignment", (int)obs_sceneitem_get_alignment(item));
	obs_data_set_double(data, "cx", item_width * scale.x);
	obs_data_set_double(data, "cy", item_height * scale.y);
	obs_data_set_bool(data, "render", obs_sceneitem_visible(item));
	obs_data_set_bool(data, "locked", obs_sceneitem_locked(item));

	obs_scene_t* parent = obs_sceneitem_get_scene(item);
	if (parent) {
		OBSSource parentSource = obs_scene_get_source(parent);
		QString parentKind = obs_source_get_id(parentSource);
		if (parentKind == "group") {
			obs_data_set_string(data, "parentGroupName", obs_source_get_name(parentSource));
		}
	}

	if (obs_sceneitem_is_group(item)) {
		OBSDataArrayAutoRelease children = obs_data_array_create();
		obs_sceneitem_group_enum_items(item, [](obs_scene_t*, obs_sceneitem_t* currentItem, void* param) {
			obs_data_array_t* items = reinterpret_cast<obs_data_array_t*>(param);

			OBSDataAutoRelease itemData = GetSceneItemData(currentItem);
			obs_data_array_push_back(items, itemData);

			return true;
		}, children);
		obs_data_set_array(data, "groupChildren", children);
	}

	return data;
}

obs_sceneitem_t* Utils::GetSceneItemFromName(obs_scene_t* scene, QString name) {
	if (!scene) {
		return nullptr;
	}

	struct current_search {
		QString query;
		obs_sceneitem_t* result;
		bool (*enumCallback)(obs_scene_t*, obs_sceneitem_t*, void*);
	};

	current_search search;
	search.query = name;
	search.result = nullptr;

	search.enumCallback = [](
			obs_scene_t* scene,
			obs_sceneitem_t* currentItem,
			void* param)
	{
		current_search* search = reinterpret_cast<current_search*>(param);

		if (obs_sceneitem_is_group(currentItem)) {
			obs_sceneitem_group_enum_items(currentItem, search->enumCallback, search);
			if (search->result) {
				return false;
			}
		}

		QString currentItemName =
			obs_source_get_name(obs_sceneitem_get_source(currentItem));

		if (currentItemName == search->query) {
			search->result = currentItem;
			obs_sceneitem_addref(search->result);
			return false;
		}

		return true;
	};

	obs_scene_enum_items(scene, search.enumCallback, &search);

	return search.result;
}

obs_sceneitem_t* Utils::GetSceneItemFromId(obs_scene_t* scene, int64_t id) {
	if (!scene) {
		return nullptr;
	}

	struct current_search {
		int query;
		obs_sceneitem_t* result;
		bool (*enumCallback)(obs_scene_t*, obs_sceneitem_t*, void*);
	};

	current_search search;
	search.query = id;
	search.result = nullptr;

	search.enumCallback = [](
		obs_scene_t* scene,
		obs_sceneitem_t* currentItem,
		void* param)
	{
		current_search* search = reinterpret_cast<current_search*>(param);

		if (obs_sceneitem_is_group(currentItem)) {
			obs_sceneitem_group_enum_items(currentItem, search->enumCallback, search);
			if (search->result) {
				return false;
			}
		}

		if (obs_sceneitem_get_id(currentItem) == search->query) {
			search->result = currentItem;
			obs_sceneitem_addref(search->result);
			return false;
		}

		return true;
	};

	obs_scene_enum_items(scene, search.enumCallback, &search);

	return search.result;
}

obs_sceneitem_t* Utils::GetSceneItemFromItem(obs_scene_t* scene, obs_data_t* itemInfo) {
	if (!scene) {
		return nullptr;
	}

	OBSDataItemAutoRelease idInfoItem = obs_data_item_byname(itemInfo, "id");
	int id = obs_data_item_get_int(idInfoItem);

	OBSDataItemAutoRelease nameInfoItem = obs_data_item_byname(itemInfo, "name");
	const char* name = obs_data_item_get_string(nameInfoItem);

	if (idInfoItem) {
		obs_sceneitem_t* sceneItem = GetSceneItemFromId(scene, id);
		obs_source_t* sceneItemSource = obs_sceneitem_get_source(sceneItem);

		QString sceneItemName = obs_source_get_name(sceneItemSource);
		if (nameInfoItem && (QString(name) != sceneItemName)) {
			return nullptr;
		}

		return sceneItem;
	} else if (nameInfoItem) {
		return GetSceneItemFromName(scene, name);
	}

	return nullptr;
}

obs_sceneitem_t* Utils::GetSceneItemFromRequestField(obs_scene_t* scene, obs_data_item_t* dataItem)
{
	enum obs_data_type dataType = obs_data_item_gettype(dataItem);

	if (dataType == OBS_DATA_OBJECT) {
		OBSDataAutoRelease itemData = obs_data_item_get_obj(dataItem);
		return GetSceneItemFromItem(scene, itemData);
	} else if (dataType == OBS_DATA_STRING) {
		QString name = obs_data_item_get_string(dataItem);
		return GetSceneItemFromName(scene, name);
	}

	return nullptr;
}

bool Utils::IsValidAlignment(const uint32_t alignment) {
	switch (alignment) {
		case OBS_ALIGN_CENTER:
		case OBS_ALIGN_LEFT:
		case OBS_ALIGN_RIGHT:
		case OBS_ALIGN_TOP:
		case OBS_ALIGN_BOTTOM:
		case OBS_ALIGN_TOP | OBS_ALIGN_LEFT:
		case OBS_ALIGN_TOP | OBS_ALIGN_RIGHT:
		case OBS_ALIGN_BOTTOM | OBS_ALIGN_LEFT:
		case OBS_ALIGN_BOTTOM | OBS_ALIGN_RIGHT: {
			return true;
		}
	}
	return false;
}

obs_source_t* Utils::GetTransitionFromName(QString searchName) {
	obs_source_t* foundTransition = nullptr;

	obs_frontend_source_list transition_list = {};
	obs_frontend_get_transitions(&transition_list);

	for (size_t i = 0; i < transition_list.sources.num; i++) {
		obs_source_t* transition = transition_list.sources.array[i];
		QString transitionName = obs_source_get_name(transition);

		if (transitionName == searchName) {
			foundTransition = transition;
			obs_source_addref(foundTransition);
			break;
		}
	}

	obs_frontend_source_list_free(&transition_list);
	return foundTransition;
}

obs_scene_t* Utils::GetSceneFromNameOrCurrent(QString sceneName) {
	// Both obs_frontend_get_current_scene() and obs_get_source_by_name()
	// increase the returned source's refcount
	OBSSourceAutoRelease sceneSource = nullptr;

	if (sceneName.isEmpty() || sceneName.isNull()) {
		sceneSource = obs_frontend_get_current_scene();
	}
	else {
		sceneSource = obs_get_source_by_name(sceneName.toUtf8());
	}

	return obs_scene_from_source(sceneSource);
}

obs_data_array_t* Utils::GetScenes() {
	obs_frontend_source_list sceneList = {};
	obs_frontend_get_scenes(&sceneList);

	obs_data_array_t* scenes = obs_data_array_create();
	for (size_t i = 0; i < sceneList.sources.num; i++) {
		obs_source_t* scene = sceneList.sources.array[i];
		OBSDataAutoRelease sceneData = GetSceneData(scene);
		obs_data_array_push_back(scenes, sceneData);
	}

	obs_frontend_source_list_free(&sceneList);
	return scenes;
}

obs_data_t* Utils::GetSceneData(obs_source_t* source) {
	OBSDataArrayAutoRelease sceneItems = GetSceneItems(source);

	obs_data_t* sceneData = obs_data_create();
	obs_data_set_string(sceneData, "name", obs_source_get_name(source));
	obs_data_set_array(sceneData, "sources", sceneItems);

	return sceneData;
}

QSpinBox* Utils::GetTransitionDurationControl() {
	QMainWindow* window = (QMainWindow*)obs_frontend_get_main_window();
	return window->findChild<QSpinBox*>("transitionDuration");
}

int Utils::GetTransitionDuration(obs_source_t* transition) {
	if (!transition || obs_source_get_type(transition) != OBS_SOURCE_TYPE_TRANSITION) {
		return -1;
	}

	QString transitionKind = obs_source_get_id(transition);
	if (transitionKind == "cut_transition") {
		// If this is a Cut transition, return 0
		return 0;
	}

	if (obs_transition_fixed(transition)) {
		// If this transition has a fixed duration (such as a Stinger),
		// we don't currently have a way of retrieving that number.
		// For now, return -1 to indicate that we don't know the actual duration.
		return -1;
	}

	OBSSourceAutoRelease destinationScene = obs_transition_get_active_source(transition);
	OBSDataAutoRelease destinationSettings = obs_source_get_private_settings(destinationScene);

	// Detect if transition is the global transition or a transition override.
	// Fetching the duration is different depending on the case.
	obs_data_item_t* transitionDurationItem = obs_data_item_byname(destinationSettings, "transition_duration");
	int duration = (
		transitionDurationItem
		? obs_data_item_get_int(transitionDurationItem)
		: obs_frontend_get_transition_duration()
	);

	return duration;
}

bool Utils::SetTransitionByName(QString transitionName) {
	OBSSourceAutoRelease transition = GetTransitionFromName(transitionName);

	if (transition) {
		obs_frontend_set_current_transition(transition);
		return true;
	} else {
		return false;
	}
}

obs_data_t* Utils::GetTransitionData(obs_source_t* transition) {
	int duration = Utils::GetTransitionDuration(transition);
	if (duration < 0) {
		blog(LOG_WARNING, "GetTransitionData: duration is negative !");
	}

	OBSSourceAutoRelease sourceScene = obs_transition_get_source(transition, OBS_TRANSITION_SOURCE_A);
	OBSSourceAutoRelease destinationScene = obs_transition_get_active_source(transition);

	obs_data_t* transitionData = obs_data_create();
	obs_data_set_string(transitionData, "name", obs_source_get_name(transition));
	obs_data_set_string(transitionData, "type", obs_source_get_id(transition));
	obs_data_set_int(transitionData, "duration", duration);

	// When a transition starts and while it is running, SOURCE_A is the source scene
	// and SOURCE_B is the destination scene.
	// Before the transition_end event is triggered on a transition, the destination scene
	// goes into SOURCE_A and SOURCE_B becomes null. This means that, in transition_stop
	// we don't know what was the source scene
	// TODO fix this in libobs

	bool isTransitionEndEvent = (sourceScene == destinationScene);
	if (!isTransitionEndEvent) {
		obs_data_set_string(transitionData, "from-scene", obs_source_get_name(sourceScene));
	}
	
	obs_data_set_string(transitionData, "to-scene", obs_source_get_name(destinationScene));

	return transitionData;
}

QString Utils::OBSVersionString() {
	uint32_t version = obs_get_version();

	uint8_t major, minor, patch;
	major = (version >> 24) & 0xFF;
	minor = (version >> 16) & 0xFF;
	patch = version & 0xFF;

	QString result = QString("%1.%2.%3")
		.arg(major).arg(minor).arg(patch);

	return result;
}

QSystemTrayIcon* Utils::GetTrayIcon() {
	void* systemTray = obs_frontend_get_system_tray();
	return reinterpret_cast<QSystemTrayIcon*>(systemTray);
}

void Utils::SysTrayNotify(QString text, QSystemTrayIcon::MessageIcon icon, QString title) {
	auto config = GetConfig();
	if ((config && !config->AlertsEnabled) ||
		!QSystemTrayIcon::isSystemTrayAvailable() ||
		!QSystemTrayIcon::supportsMessages())
	{
		return;
	}

	QSystemTrayIcon* trayIcon = GetTrayIcon();
	if (trayIcon)
		trayIcon->showMessage(title, text, icon);
}

const char* Utils::GetRecordingFolder() {
	config_t* profile = obs_frontend_get_profile_config();
	QString outputMode = config_get_string(profile, "Output", "Mode");

	if (outputMode == "Advanced") {
		// Advanced mode
		return config_get_string(profile, "AdvOut", "RecFilePath");
	} else {
		// Simple mode
		return config_get_string(profile, "SimpleOutput", "FilePath");
	}
}

bool Utils::SetRecordingFolder(const char* path) {
	QDir dir(path);
	if (!dir.exists()) {
		dir.mkpath(".");
	}

	config_t* profile = obs_frontend_get_profile_config();
	config_set_string(profile, "AdvOut", "RecFilePath", path);
	config_set_string(profile, "SimpleOutput", "FilePath", path);

	config_save(profile);
	return true;
}

QString Utils::ParseDataToQueryString(obs_data_t* data) {
	if (!data)
		return QString();

	QString query;

	obs_data_item_t* item = obs_data_first(data);
	if (item) {
		bool isFirst = true;
		do {
			if (!obs_data_item_has_user_value(item))
				continue;

			if (!isFirst)
				query += "&";
			else
				isFirst = false;

			QString attrName = obs_data_item_get_name(item);
			query += (attrName + "=");

			switch (obs_data_item_gettype(item)) {
				case OBS_DATA_BOOLEAN:
					query += (obs_data_item_get_bool(item) ? "true" : "false");
					break;

				case OBS_DATA_NUMBER:
					switch (obs_data_item_numtype(item)) {
						case OBS_DATA_NUM_DOUBLE:
							query +=
								QString::number(obs_data_item_get_double(item));
							break;
						case OBS_DATA_NUM_INT:
							query +=
								QString::number(obs_data_item_get_int(item));
							break;
						case OBS_DATA_NUM_INVALID:
							break;
					}
					break;

				case OBS_DATA_STRING:
					query +=
						QUrl::toPercentEncoding(
							QString(obs_data_item_get_string(item)));
					break;

				default:
					//other types are not supported
					break;
			}
		} while (obs_data_item_next(&item));
	}

	return query;
}

obs_hotkey_t* Utils::FindHotkeyByName(QString name) {
	struct current_search {
		QString query;
		obs_hotkey_t* result;
	};

	current_search search;
	search.query = name;
	search.result = nullptr;

	obs_enum_hotkeys([](void* data, obs_hotkey_id id, obs_hotkey_t* hotkey) {
		current_search* search = reinterpret_cast<current_search*>(data);

		const char* hk_name = obs_hotkey_get_name(hotkey);
		if (hk_name == search->query) {
			search->result = hotkey;
			return false;
		}

		return true;
	}, &search);

	return search.result;
}

bool Utils::ReplayBufferEnabled() {
	config_t* profile = obs_frontend_get_profile_config();
	QString outputMode = config_get_string(profile, "Output", "Mode");

	if (outputMode == "Simple") {
		return config_get_bool(profile, "SimpleOutput", "RecRB");
	}
	else if (outputMode == "Advanced") {
		return config_get_bool(profile, "AdvOut", "RecRB");
	}

	return false;
}

void Utils::StartReplayBuffer() {
	if (obs_frontend_replay_buffer_active())
		return;

	if (!IsRPHotkeySet()) {
		obs_output_t* rpOutput = obs_frontend_get_replay_buffer_output();
		OBSData outputHotkeys = obs_hotkeys_save_output(rpOutput);

		OBSDataAutoRelease dummyBinding = obs_data_create();
		obs_data_set_bool(dummyBinding, "control", true);
		obs_data_set_bool(dummyBinding, "alt", true);
		obs_data_set_bool(dummyBinding, "shift", true);
		obs_data_set_bool(dummyBinding, "command", true);
		obs_data_set_string(dummyBinding, "key", "OBS_KEY_0");

		OBSDataArray rpSaveHotkey = obs_data_get_array(
			outputHotkeys, "ReplayBuffer.Save");
		obs_data_array_push_back(rpSaveHotkey, dummyBinding);

		obs_hotkeys_load_output(rpOutput, outputHotkeys);
		obs_frontend_replay_buffer_start();

		obs_output_release(rpOutput);
	}
	else {
		obs_frontend_replay_buffer_start();
	}
}

bool Utils::IsRPHotkeySet() {
	OBSOutputAutoRelease rpOutput = obs_frontend_get_replay_buffer_output();
	OBSDataAutoRelease hotkeys = obs_hotkeys_save_output(rpOutput);
	OBSDataArrayAutoRelease bindings = obs_data_get_array(hotkeys,
		"ReplayBuffer.Save");

	size_t count = obs_data_array_count(bindings);
	return (count > 0);
}

const char* Utils::GetFilenameFormatting() {
	config_t* profile = obs_frontend_get_profile_config();
	return config_get_string(profile, "Output", "FilenameFormatting");
}

bool Utils::SetFilenameFormatting(const char* filenameFormatting) {
	config_t* profile = obs_frontend_get_profile_config();
	config_set_string(profile, "Output", "FilenameFormatting", filenameFormatting);
	config_save(profile);
	return true;
}

const char* Utils::GetCurrentRecordingFilename()
{
	OBSOutputAutoRelease recordingOutput = obs_frontend_get_recording_output();
	if (!recordingOutput) {
		return nullptr;
	}

	OBSDataAutoRelease settings = obs_output_get_settings(recordingOutput);

	// mimicks the behavior of BasicOutputHandler::GetRecordingFilename :
	// try to fetch the path from the "url" property, then try "path" if the first one
	// didn't yield any result
	OBSDataItemAutoRelease item = obs_data_item_byname(settings, "url");
	if (!item) {
		item = obs_data_item_byname(settings, "path");
		if (!item) {
			return nullptr;
		}
	}

	return obs_data_item_get_string(item);
}

// Transform properties copy-pasted from WSRequestHandler_SceneItems.cpp because typedefs can't be extended yet

/**
 * @typedef {Object} `SceneItemTransform`
 * @property {double} `position.x` The x position of the scene item from the left.
 * @property {double} `position.y` The y position of the scene item from the top.
 * @property {int} `position.alignment` The point on the scene item that the item is manipulated from.
 * @property {double} `rotation` The clockwise rotation of the scene item in degrees around the point of alignment.
 * @property {double} `scale.x` The x-scale factor of the scene item.
 * @property {double} `scale.y` The y-scale factor of the scene item.
 * @property {String} `scale.filter` The scale filter of the source. Can be "OBS_SCALE_DISABLE", "OBS_SCALE_POINT", "OBS_SCALE_BICUBIC", "OBS_SCALE_BILINEAR", "OBS_SCALE_LANCZOS" or "OBS_SCALE_AREA".
 * @property {int} `crop.top` The number of pixels cropped off the top of the scene item before scaling.
 * @property {int} `crop.right` The number of pixels cropped off the right of the scene item before scaling.
 * @property {int} `crop.bottom` The number of pixels cropped off the bottom of the scene item before scaling.
 * @property {int} `crop.left` The number of pixels cropped off the left of the scene item before scaling.
 * @property {bool} `visible` If the scene item is visible.
 * @property {bool} `locked` If the scene item is locked in position.
 * @property {String} `bounds.type` Type of bounding box. Can be "OBS_BOUNDS_STRETCH", "OBS_BOUNDS_SCALE_INNER", "OBS_BOUNDS_SCALE_OUTER", "OBS_BOUNDS_SCALE_TO_WIDTH", "OBS_BOUNDS_SCALE_TO_HEIGHT", "OBS_BOUNDS_MAX_ONLY" or "OBS_BOUNDS_NONE".
 * @property {int} `bounds.alignment` Alignment of the bounding box.
 * @property {double} `bounds.x` Width of the bounding box.
 * @property {double} `bounds.y` Height of the bounding box.
 * @property {int} `sourceWidth` Base width (without scaling) of the source
 * @property {int} `sourceHeight` Base source (without scaling) of the source
 * @property {double} `width` Scene item width (base source width multiplied by the horizontal scaling factor)
 * @property {double} `height` Scene item height (base source height multiplied by the vertical scaling factor)
 * @property {String (optional)} `parentGroupName` Name of the item's parent (if this item belongs to a group)
 * @property {Array<SceneItemTransform> (optional)} `groupChildren` List of children (if this item is a group) 
 */
obs_data_t* Utils::GetSceneItemPropertiesData(obs_sceneitem_t* sceneItem) {
	if (!sceneItem) {
		return nullptr;
	}

	OBSSource source = obs_sceneitem_get_source(sceneItem);
	uint32_t baseSourceWidth = obs_source_get_width(source);
	uint32_t baseSourceHeight = obs_source_get_height(source);

	vec2 pos, scale, bounds;
	obs_sceneitem_crop crop;

	obs_sceneitem_get_pos(sceneItem, &pos);
	obs_sceneitem_get_scale(sceneItem, &scale);
	obs_sceneitem_get_crop(sceneItem, &crop);
	obs_sceneitem_get_bounds(sceneItem, &bounds);

	uint32_t alignment = obs_sceneitem_get_alignment(sceneItem);
	float rotation = obs_sceneitem_get_rot(sceneItem);
	bool isVisible = obs_sceneitem_visible(sceneItem);
	bool isLocked = obs_sceneitem_locked(sceneItem);

	obs_bounds_type boundsType = obs_sceneitem_get_bounds_type(sceneItem);
	uint32_t boundsAlignment = obs_sceneitem_get_bounds_alignment(sceneItem);
	QString boundsTypeName = getBoundsNameFromType(boundsType);

	obs_scale_type scaleFilter = obs_sceneitem_get_scale_filter(sceneItem);
	QString scaleFilterName = getScaleNameFromType(scaleFilter);

	OBSDataAutoRelease posData = obs_data_create();
	obs_data_set_double(posData, "x", pos.x);
	obs_data_set_double(posData, "y", pos.y);
	obs_data_set_int(posData, "alignment", alignment);

	OBSDataAutoRelease scaleData = obs_data_create();
	obs_data_set_string(scaleData, "filter", scaleFilterName.toUtf8());
	obs_data_set_double(scaleData, "x", scale.x);
	obs_data_set_double(scaleData, "y", scale.y);

	OBSDataAutoRelease cropData = obs_data_create();
	obs_data_set_int(cropData, "left", crop.left);
	obs_data_set_int(cropData, "top", crop.top);
	obs_data_set_int(cropData, "right", crop.right);
	obs_data_set_int(cropData, "bottom", crop.bottom);

	OBSDataAutoRelease boundsData = obs_data_create();
	obs_data_set_string(boundsData, "type", boundsTypeName.toUtf8());
	obs_data_set_int(boundsData, "alignment", boundsAlignment);
	obs_data_set_double(boundsData, "x", bounds.x);
	obs_data_set_double(boundsData, "y", bounds.y);

	obs_data_t* data = obs_data_create();
	obs_data_set_obj(data, "position", posData);
	obs_data_set_double(data, "rotation", rotation);
	obs_data_set_obj(data, "scale", scaleData);
	obs_data_set_obj(data, "crop", cropData);
	obs_data_set_bool(data, "visible", isVisible);
	obs_data_set_bool(data, "locked", isLocked);
	obs_data_set_obj(data, "bounds", boundsData);

	obs_data_set_int(data, "sourceWidth", baseSourceWidth);
	obs_data_set_int(data, "sourceHeight", baseSourceHeight);
	obs_data_set_double(data, "width", baseSourceWidth * scale.x);
	obs_data_set_double(data, "height", baseSourceHeight * scale.y);

	obs_scene_t* parent = obs_sceneitem_get_scene(sceneItem);
	if (parent) {
		OBSSource parentSource = obs_scene_get_source(parent);
		QString parentKind = obs_source_get_id(parentSource);
		if (parentKind == "group") {
			obs_data_set_string(data, "parentGroupName", obs_source_get_name(parentSource));
		}
	}

	if (obs_sceneitem_is_group(sceneItem)) {
		OBSDataArrayAutoRelease children = obs_data_array_create();
		obs_sceneitem_group_enum_items(sceneItem, [](obs_scene_t*, obs_sceneitem_t* subItem, void* param) {
			obs_data_array_t* items = reinterpret_cast<obs_data_array_t*>(param);

			OBSDataAutoRelease itemData = GetSceneItemPropertiesData(subItem);
			obs_data_array_push_back(items, itemData);

			return true;
		}, children);
		obs_data_set_array(data, "groupChildren", children);
	}

	return data;
}

obs_data_t* Utils::GetSourceFilterInfo(obs_source_t* filter, bool includeSettings)
{
	obs_data_t* data = obs_data_create();
	obs_data_set_bool(data, "enabled", obs_source_enabled(filter));
	obs_data_set_string(data, "type", obs_source_get_id(filter));
	obs_data_set_string(data, "name", obs_source_get_name(filter));
	if (includeSettings) {
		OBSDataAutoRelease settings = obs_source_get_settings(filter);
		obs_data_set_obj(data, "settings", settings);
	}
	return data;
}

obs_data_array_t* Utils::GetSourceFiltersList(obs_source_t* source, bool includeSettings)
{
	struct enum_params {
		obs_data_array_t* filters;
		bool includeSettings;
	};

	if (!source) {
		return nullptr;
	}

	struct enum_params enumParams;

	enumParams.filters = obs_data_array_create();
	enumParams.includeSettings = includeSettings;

	obs_source_enum_filters(source, [](obs_source_t* parent, obs_source_t* child, void* param)
	{
		auto enumParams = reinterpret_cast<struct enum_params*>(param);

		OBSDataAutoRelease filterData = Utils::GetSourceFilterInfo(child, enumParams->includeSettings);
		obs_data_array_push_back(enumParams->filters, filterData);
	}, &enumParams);

	return enumParams.filters;
}

void getPauseRecordingFunctions(RecordingPausedFunction* recPausedFuncPtr, PauseRecordingFunction* pauseRecFuncPtr)
{
	void* frontendApi = os_dlopen("obs-frontend-api");

	if (recPausedFuncPtr) {
		*recPausedFuncPtr = (RecordingPausedFunction)os_dlsym(frontendApi, "obs_frontend_recording_paused");
	}

	if (pauseRecFuncPtr) {
		*pauseRecFuncPtr = (PauseRecordingFunction)os_dlsym(frontendApi, "obs_frontend_recording_pause");
	}
}

QString Utils::nsToTimestamp(uint64_t ns)
{
	uint64_t ms = ns / 1000000ULL;
	uint64_t secs = ms / 1000ULL;
	uint64_t minutes = secs / 60ULL;

	uint64_t hoursPart = minutes / 60ULL;
	uint64_t minutesPart = minutes % 60ULL;
	uint64_t secsPart = secs % 60ULL;
	uint64_t msPart = ms % 1000ULL;

	return QString::asprintf("%02" PRIu64 ":%02" PRIu64 ":%02" PRIu64 ".%03" PRIu64, hoursPart, minutesPart, secsPart, msPart);
}

void Utils::AddSourceHelper(void *_data, obs_scene_t *scene)
{
	auto *data = reinterpret_cast<AddSourceData*>(_data);
	data->sceneItem = obs_scene_add(scene, data->source);
	obs_sceneitem_set_visible(data->sceneItem, data->setVisible);
}

obs_data_t *Utils::OBSDataGetDefaults(obs_data_t *data)
{
	obs_data_t *returnData = obs_data_create();
	obs_data_item_t *item = NULL;

	for (item = obs_data_first(data); item; obs_data_item_next(&item)) {
		enum obs_data_type type = obs_data_item_gettype(item);
		const char *name = obs_data_item_get_name(item);

		if (type == OBS_DATA_STRING) {
			const char *val = obs_data_item_get_string(item);
			obs_data_set_string(returnData, name, val);
		} else if (type == OBS_DATA_NUMBER) {
			enum obs_data_number_type type = obs_data_item_numtype(item);
			if (type == OBS_DATA_NUM_INT) {
				long long val = obs_data_item_get_int(item);
				obs_data_set_int(returnData, name, val);
			} else {
				double val = obs_data_item_get_double(item);
				obs_data_set_double(returnData, name, val);
			}
		} else if (type == OBS_DATA_BOOLEAN) {
			bool val = obs_data_item_get_bool(item);
			obs_data_set_bool(returnData, name, val);
		} else if (type == OBS_DATA_OBJECT) {
			OBSDataAutoRelease obj = obs_data_item_get_obj(item);
			obs_data_set_obj(returnData, name, obj);
		} else if (type == OBS_DATA_ARRAY) {
			OBSDataArrayAutoRelease array = obs_data_item_get_array(item);
			obs_data_set_array(returnData, name, array);
		}
	}
	return returnData;
}
