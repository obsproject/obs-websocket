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

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#include <QSpinBox>
#include <QPushButton>
#include <QLayout>
#include <QListWidget>
#include <QSystemTrayIcon>
#include <QHostAddress>

#include <obs.hpp>
#include <obs-module.h>
#include <util/config-file.h>

class Utils {
  public:
    static obs_data_array_t* StringListToArray(char** strings, char* key);
    static obs_data_array_t* GetSceneItems(obs_source_t* source);
    static obs_data_t* GetSceneItemData(obs_sceneitem_t* item);
    static obs_sceneitem_t* GetSceneItemFromName(
        obs_source_t* source, QString name);
    static obs_sceneitem_t* GetSceneItemFromId(obs_source_t* source, size_t id);
    static obs_sceneitem_t* GetSceneItemFromItem(obs_source_t* source, obs_data_t* item);
    static obs_source_t* GetTransitionFromName(QString transitionName);
    static obs_source_t* GetSceneFromNameOrCurrent(QString sceneName);

    static bool IsValidAlignment(const uint32_t alignment);

    static obs_data_array_t* GetScenes();
    static obs_data_t* GetSceneData(obs_source_t* source);

    static QSpinBox* GetTransitionDurationControl();
    static int GetTransitionDuration();
    static void SetTransitionDuration(int ms);

    static bool SetTransitionByName(QString transitionName);

    static QPushButton* GetPreviewModeButtonControl();
    static QLayout* GetPreviewLayout();
    static QListWidget* GetSceneListControl();
    static obs_scene_t* SceneListItemToScene(QListWidgetItem* item);

    static void TransitionToProgram();

    static QString OBSVersionString();

    static QSystemTrayIcon* GetTrayIcon();
    static void SysTrayNotify(
        QString &text,
        QSystemTrayIcon::MessageIcon n,
        QString title = QString("obs-websocket"));

    static QString FormatIPAddress(QHostAddress &addr);

    static const char* GetRecordingFolder();
    static bool SetRecordingFolder(const char* path);

    static QString ParseDataToQueryString(obs_data_t* data);
    static obs_hotkey_t* FindHotkeyByName(QString name);
    static bool ReplayBufferEnabled();
    static void StartReplayBuffer();
    static bool IsRPHotkeySet();
    static const char* GetFilenameFormatting();
    static bool SetFilenameFormatting(const char* filenameFormatting);
};

#endif // UTILS_H
