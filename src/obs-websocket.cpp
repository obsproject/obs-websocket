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

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <QAction>
#include <QMainWindow>
#include <QTimer>

#include "obs-websocket.h"
#include "WSServer.h"
#include "WSEvents.h"
#include "Config.h"
#include "forms/settings-dialog.h"

void ___source_dummy_addref(obs_source_t*) {}
void ___sceneitem_dummy_addref(obs_sceneitem_t*) {}
void ___data_dummy_addref(obs_data_t*) {}
void ___data_array_dummy_addref(obs_data_array_t*) {}
void ___output_dummy_addref(obs_output_t*) {}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-websocket", "en-US")

SettingsDialog* settings_dialog;

bool obs_module_load(void) {
    blog(LOG_INFO, "you can haz websockets (version %s)", OBS_WEBSOCKET_VERSION);
    blog(LOG_INFO, "qt version (compile-time): %s ; qt version (run-time): %s",
        QT_VERSION_STR, qVersion());

    // Core setup
    Config* config = Config::Current();
    config->Load();

    WSServer::Instance = new WSServer();
    WSEvents::Instance = new WSEvents(WSServer::Instance);

    if (config->ServerEnabled)
        WSServer::Instance->Start(config->ServerPort);

    // UI setup
    QAction* menu_action = (QAction*)obs_frontend_add_tools_menu_qaction(
        obs_module_text("OBSWebsocket.Menu.SettingsItem"));

    obs_frontend_push_ui_translation(obs_module_get_string);
    QMainWindow* main_window = (QMainWindow*)obs_frontend_get_main_window();
    settings_dialog = new SettingsDialog(main_window);
    obs_frontend_pop_ui_translation();

    auto menu_cb = [] {
        settings_dialog->ToggleShowHide();
    };
    menu_action->connect(menu_action, &QAction::triggered, menu_cb);

    // Loading finished
    blog(LOG_INFO, "module loaded!");

    return true;
}

void obs_module_unload() {
    blog(LOG_INFO, "goodbye!");
}

