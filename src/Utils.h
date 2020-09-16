/*
obs-websocket
Copyright (C) 2016-2019	Stéphane Lepin <stephane.lepin@gmail.com>

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

#include <stdio.h>

#include <QtCore/QString>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QSystemTrayIcon>

#include <obs.hpp>
#include <obs-module.h>
#include <util/config-file.h>

typedef void(*PauseRecordingFunction)(bool);
typedef bool(*RecordingPausedFunction)();

namespace Utils {
	bool StringInStringList(char** strings, const char* string);
	obs_data_array_t* StringListToArray(char** strings, const char* key);
	obs_data_array_t* GetSceneItems(obs_source_t* source);
	obs_data_t* GetSceneItemData(obs_sceneitem_t* item);

	// These functions support nested lookup into groups
	obs_sceneitem_t* GetSceneItemFromName(obs_scene_t* scene, QString name);
	obs_sceneitem_t* GetSceneItemFromId(obs_scene_t* scene, int64_t id);
	obs_sceneitem_t* GetSceneItemFromItem(obs_scene_t* scene, obs_data_t* item);
	obs_sceneitem_t* GetSceneItemFromRequestField(obs_scene_t* scene, obs_data_item_t* dataItem);

	obs_scene_t* GetSceneFromNameOrCurrent(QString sceneName);
	obs_data_t* GetSceneItemPropertiesData(obs_sceneitem_t* item);

	obs_data_t* GetSourceFilterInfo(obs_source_t* filter, bool includeSettings);
	obs_data_array_t* GetSourceFiltersList(obs_source_t* source, bool includeSettings);

	bool IsValidAlignment(const uint32_t alignment);

	obs_data_array_t* GetScenes();
	obs_data_t* GetSceneData(obs_source_t* source);

	// TODO contribute a proper frontend API method for this to OBS and remove this hack
	QSpinBox* GetTransitionDurationControl();
	int GetTransitionDuration(obs_source_t* transition);
	obs_source_t* GetTransitionFromName(QString transitionName);
	bool SetTransitionByName(QString transitionName);
	obs_data_t* GetTransitionData(obs_source_t* transition);

	QString OBSVersionString();

	QSystemTrayIcon* GetTrayIcon();
	void SysTrayNotify(
		QString text,
		QSystemTrayIcon::MessageIcon n,
		QString title = QString("obs-websocket"));

	const char* GetRecordingFolder();
	bool SetRecordingFolder(const char* path);

	QString ParseDataToQueryString(obs_data_t* data);
	obs_hotkey_t* FindHotkeyByName(QString name);

	bool ReplayBufferEnabled();
	void StartReplayBuffer();
	bool IsRPHotkeySet();

	const char* GetFilenameFormatting();
	bool SetFilenameFormatting(const char* filenameFormatting);

	QString nsToTimestamp(uint64_t ns);
};
