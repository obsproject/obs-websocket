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

#include <QMainWindow>
#include <QDir>
#include <QUrl>
#include <obs-frontend-api.h>
#include <obs.hpp>
#include "obs-websocket.h"

#include "Utils.h"

Q_DECLARE_METATYPE(OBSScene);

obs_data_array_t* string_list_to_array(char** strings, char* key) {
    if (!strings)
        return obs_data_array_create();

    obs_data_array_t* list = obs_data_array_create();

    char* value = "";
    for (int i = 0; value != nullptr; i++) {
        value = strings[i];

        obs_data_t* item = obs_data_create();
        obs_data_set_string(item, key, value);

        if (value)
            obs_data_array_push_back(list, item);

        obs_data_release(item);
    }

    return list;
}

obs_data_array_t* Utils::GetSceneItems(obs_source_t* source) {
    obs_data_array_t* items = obs_data_array_create();
    obs_scene_t* scene = obs_scene_from_source(source);

    if (!scene)
        return nullptr;

    obs_scene_enum_items(scene, [](
            obs_scene_t* scene,
            obs_sceneitem_t* currentItem,
            void* param) {
        obs_data_array_t* data = static_cast<obs_data_array_t*>(param);

        obs_data_t* item_data = GetSceneItemData(currentItem);
        obs_data_array_insert(data, 0, item_data);

        obs_data_release(item_data);
        return true;
    }, items);

    return items;
}

obs_data_t* Utils::GetSceneItemData(obs_sceneitem_t* item) {
    if (!item)
        return nullptr;

    vec2 pos;
    obs_sceneitem_get_pos(item, &pos);

    vec2 scale;
    obs_sceneitem_get_scale(item, &scale);

    obs_source_t* item_source = obs_sceneitem_get_source(item);
    float item_width = float(obs_source_get_width(item_source));
    float item_height = float(obs_source_get_height(item_source));

    obs_data_t* data = obs_data_create();
    obs_data_set_string(data, "name",
        obs_source_get_name(obs_sceneitem_get_source(item)));
    obs_data_set_string(data, "type",
        obs_source_get_id(obs_sceneitem_get_source(item)));
    obs_data_set_double(data, "volume",
        obs_source_get_volume(obs_sceneitem_get_source(item)));
    obs_data_set_double(data, "x", pos.x);
    obs_data_set_double(data, "y", pos.y);
    obs_data_set_int(data, "source_cx", (int)item_width);
    obs_data_set_int(data, "source_cy", (int)item_height);
    obs_data_set_double(data, "cx", item_width*  scale.x);
    obs_data_set_double(data, "cy", item_height*  scale.y);
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
    search.result = nullptr;

    obs_scene_t* scene = obs_scene_from_source(source);
    if (scene == nullptr)
        return nullptr;

    obs_scene_enum_items(scene, [](
            obs_scene_t* scene,
            obs_sceneitem_t* currentItem,
            void* param) {
        current_search* search = static_cast<current_search*>(param);

        const char* currentItemName =
            obs_source_get_name(obs_sceneitem_get_source(currentItem));

        if (strcmp(currentItemName, search->query) == 0) {
            search->result = currentItem;
            obs_sceneitem_addref(search->result);
            return false;
        }

        return true;
    }, &search);

    return search.result;
}

obs_source_t* Utils::GetTransitionFromName(const char* search_name) {
    obs_source_t* found_transition = NULL;

    obs_frontend_source_list transition_list = {};
    obs_frontend_get_transitions(&transition_list);

    for (size_t i = 0; i < transition_list.sources.num; i++) {
        obs_source_t* transition = transition_list.sources.array[i];

        const char* transition_name = obs_source_get_name(transition);
        if (strcmp(transition_name, search_name) == 0)
        {
            found_transition = transition;
            obs_source_addref(found_transition);
            break;
        }
    }

    obs_frontend_source_list_free(&transition_list);

    return found_transition;
}

obs_source_t* Utils::GetSceneFromNameOrCurrent(const char* scene_name) {
    obs_source_t* scene = nullptr;

    if (!scene_name || !strlen(scene_name))
        scene = obs_frontend_get_current_scene();
    else
        scene = obs_get_source_by_name(scene_name);

    return scene;
}

obs_data_array_t* Utils::GetScenes() {
    obs_frontend_source_list sceneList = {};
    obs_frontend_get_scenes(&sceneList);

    obs_data_array_t* scenes = obs_data_array_create();
    for (size_t i = 0; i < sceneList.sources.num; i++) {
        obs_source_t* scene = sceneList.sources.array[i];

        obs_data_t* scene_data = GetSceneData(scene);
        obs_data_array_push_back(scenes, scene_data);

        obs_data_release(scene_data);
    }

    obs_frontend_source_list_free(&sceneList);

    return scenes;
}

obs_data_t* Utils::GetSceneData(obs_source* source) {
    obs_data_array_t* scene_items = GetSceneItems(source);

    obs_data_t* sceneData = obs_data_create();
    obs_data_set_string(sceneData, "name", obs_source_get_name(source));
    obs_data_set_array(sceneData, "sources", scene_items);

    obs_data_array_release(scene_items);
    return sceneData;
}

obs_data_array_t* Utils::GetSceneCollections() {
    char** scene_collections = obs_frontend_get_scene_collections();
    obs_data_array_t* list = string_list_to_array(scene_collections, "sc-name");

    bfree(scene_collections);
    return list;
}

obs_data_array_t* Utils::GetProfiles() {
    char** profiles = obs_frontend_get_profiles();
    obs_data_array_t* list = string_list_to_array(profiles, "profile-name");

    bfree(profiles);
    return list;
}

QSpinBox* Utils::GetTransitionDurationControl() {
    QMainWindow* window = (QMainWindow*)obs_frontend_get_main_window();
    return window->findChild<QSpinBox*>("transitionDuration");
}

int Utils::GetTransitionDuration() {
    QSpinBox* control = GetTransitionDurationControl();
    if (control)
        return control->value();
    else
        return -1;
}

void Utils::SetTransitionDuration(int ms) {
    QSpinBox* control = GetTransitionDurationControl();
    if (control && ms >= 0)
        control->setValue(ms);
}

bool Utils::SetTransitionByName(const char* transition_name) {
    obs_source_t* transition = GetTransitionFromName(transition_name);

    if (transition) {
        obs_frontend_set_current_transition(transition);
        obs_source_release(transition);
        return true;
    } else {
        return false;
    }
}

QPushButton* Utils::GetPreviewModeButtonControl() {
    QMainWindow* main = (QMainWindow*)obs_frontend_get_main_window();
    return main->findChild<QPushButton*>("modeSwitch");
}

QListWidget* Utils::GetSceneListControl() {
    QMainWindow* main = (QMainWindow*)obs_frontend_get_main_window();
    return main->findChild<QListWidget*>("scenes");
}

obs_scene_t* Utils::SceneListItemToScene(QListWidgetItem* item) {
    if (!item)
        return nullptr;

    QVariant item_data = item->data(static_cast<int>(Qt::UserRole));
    return item_data.value<OBSScene>();
}

QLayout* Utils::GetPreviewLayout() {
    QMainWindow* main = (QMainWindow*)obs_frontend_get_main_window();
    return main->findChild<QLayout*>("previewLayout");
}

bool Utils::IsPreviewModeActive() {
    QMainWindow* main = (QMainWindow*)obs_frontend_get_main_window();

    // Clue 1 : "Studio Mode" button is toggled on
    bool buttonToggledOn = GetPreviewModeButtonControl()->isChecked();

    // Clue 2 : Preview layout has more than one item
    int previewChildCount = GetPreviewLayout()->count();
    blog(LOG_INFO, "preview layout children count : %d", previewChildCount);

    return buttonToggledOn || (previewChildCount >= 2);
}

void Utils::EnablePreviewMode() {
    if (!IsPreviewModeActive())
        GetPreviewModeButtonControl()->click();
}

void Utils::DisablePreviewMode() {
    if (IsPreviewModeActive())
        GetPreviewModeButtonControl()->click();
}

void Utils::TogglePreviewMode() {
    GetPreviewModeButtonControl()->click();
}

obs_scene_t* Utils::GetPreviewScene() {
    if (IsPreviewModeActive()) {
        QListWidget* sceneList = GetSceneListControl();
        QList<QListWidgetItem*> selected = sceneList->selectedItems();

        // Qt::UserRole == QtUserRole::OBSRef
        obs_scene_t* scene = Utils::SceneListItemToScene(selected.first());

        obs_scene_addref(scene);
        return scene;
    }

    return nullptr;
}

bool Utils::SetPreviewScene(const char* name) {
    if (IsPreviewModeActive()) {
        QListWidget* sceneList = GetSceneListControl();
        QList<QListWidgetItem*> matchingItems =
            sceneList->findItems(name, Qt::MatchExactly);

        if (matchingItems.count() > 0) {
            sceneList->setCurrentItem(matchingItems.first());
            return true;
        } else {
            return false;
        }
    }

    return false;
}

void Utils::TransitionToProgram() {
    if (!IsPreviewModeActive())
        return;

    // WARNING : if the layout created in OBS' CreateProgramOptions() changes
    // then this won't work as expected

    QMainWindow* main = (QMainWindow*)obs_frontend_get_main_window();

    // The program options widget is the second item in the left-to-right layout
    QWidget* programOptions = GetPreviewLayout()->itemAt(1)->widget();

    // The "Transition" button lies in the mainButtonLayout
    // which is the first itemin the program options' layout
    QLayout* mainButtonLayout = programOptions->layout()->itemAt(1)->layout();
    QWidget* transitionBtnWidget = mainButtonLayout->itemAt(0)->widget();

    // Try to cast that widget into a button
    QPushButton* transitionBtn = qobject_cast<QPushButton*>(transitionBtnWidget);

    // Perform a click on that button
    transitionBtn->click();
}

const char* Utils::OBSVersionString() {
    uint32_t version = obs_get_version();

    uint8_t major, minor, patch;
    major = (version >> 24) & 0xFF;
    minor = (version >> 16) & 0xFF;
    patch = version & 0xFF;

    char* result = (char*)bmalloc(sizeof(char) * 12);
    sprintf(result, "%d.%d.%d", major, minor, patch);

    return result;
}

QSystemTrayIcon* Utils::GetTrayIcon() {
    QMainWindow* main = (QMainWindow*)obs_frontend_get_main_window();
    return main->findChildren<QSystemTrayIcon*>().first();
}

void Utils::SysTrayNotify(QString &text,
    QSystemTrayIcon::MessageIcon icon, QString title) {
    if (!QSystemTrayIcon::supportsMessages())
        return;

    QSystemTrayIcon* trayIcon = GetTrayIcon();
    if (trayIcon)
        trayIcon->showMessage(title, text, icon);
}

QString Utils::FormatIPAddress(QHostAddress &addr) {
    if (addr.protocol() == QAbstractSocket::IPv4Protocol)
        QString v4addr = addr.toString().replace("::fff:", "");

    return addr.toString();
}

const char* Utils::GetRecordingFolder() {
    config_t* profile = obs_frontend_get_profile_config();
    const char* outputMode = config_get_string(profile, "Output", "Mode");

    if (strcmp(outputMode, "Advanced") == 0) {
        // Advanced mode
        return config_get_string(profile, "AdvOut", "RecFilePath");
    } else {
        // Simple mode
        return config_get_string(profile, "SimpleOutput", "FilePath");
    }
}

bool Utils::SetRecordingFolder(const char* path) {
    if (!QDir(path).exists())
        return false;

    config_t* profile = obs_frontend_get_profile_config();
    const char* outputMode = config_get_string(profile, "Output", "Mode");

    if (strcmp(outputMode, "Advanced") == 0) {
        config_set_string(profile, "AdvOut", "RecFilePath", path);
    } else {
        config_set_string(profile, "SimpleOutput", "FilePath", path);
    }

    config_save(profile);
    return true;
}

QString* Utils::ParseDataToQueryString(obs_data_t* data) {
    QString* query = nullptr;
    if (data) {
        obs_data_item_t* item = obs_data_first(data);
        if (item) {
            query = new QString();
            bool isFirst = true;
            do {
                if (!obs_data_item_has_user_value(item))
                    continue;
                
                if (!isFirst)
                    query->append('&');
                else
                    isFirst = false;
                
                const char* attrName = obs_data_item_get_name(item);
                query->append(attrName).append("=");

                switch (obs_data_item_gettype(item)) {
                    case OBS_DATA_BOOLEAN:
                        query->append(obs_data_item_get_bool(item)?"true":"false");
                        break;
                    case OBS_DATA_NUMBER:
                        switch (obs_data_item_numtype(item))
                        {
                            case OBS_DATA_NUM_DOUBLE:
                                query->append(
                                    QString::number(obs_data_item_get_double(item)));
                                break;
                            case OBS_DATA_NUM_INT:
                                query->append(
                                    QString::number(obs_data_item_get_int(item)));
                                break;
                            case OBS_DATA_NUM_INVALID:
                                break;
                        }
                        break;
                    case OBS_DATA_STRING:
                        query->append(QUrl::toPercentEncoding(
                            QString(obs_data_item_get_string(item))));
                        break;
                    default:
                        //other types are not supported
                        break;
                }
            } while (obs_data_item_next(&item));
        }
    }
    return query;
}
