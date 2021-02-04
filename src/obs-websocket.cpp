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
#include <obs-data.h>

#include <QtCore/QTimer>
#include <QtWidgets/QAction>
#include <QtWidgets/QMainWindow>

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

void ___data_item_dummy_addref(obs_data_item_t*) {}
void ___data_item_release(obs_data_item_t* dataItem) {
	obs_data_item_release(&dataItem);
}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-websocket", "en-US")

ConfigPtr _config;
WSServerPtr _server;
WSEventsPtr _eventsSystem;
SettingsDialog* settingsDialog = nullptr;

bool obs_module_load(void) {
	blog(LOG_INFO, "you can haz websockets (version %s)", OBS_WEBSOCKET_VERSION);
	blog(LOG_INFO, "qt version (compile-time): %s ; qt version (run-time): %s",
		QT_VERSION_STR, qVersion());

	// Core setup
	_config = ConfigPtr(new Config());
	_config->MigrateFromGlobalSettings(); // TODO remove this on the next minor jump
	_config->Load();

	_server = WSServerPtr(new WSServer());
	_eventsSystem = WSEventsPtr(new WSEvents(_server));

	// UI setup
	obs_frontend_push_ui_translation(obs_module_get_string);
	QMainWindow* mainWindow = (QMainWindow*)obs_frontend_get_main_window();
	settingsDialog = new SettingsDialog(mainWindow);
	obs_frontend_pop_ui_translation();

	const char* menuActionText =
		obs_module_text("OBSWebsocket.Settings.DialogTitle");
	QAction* menuAction =
		(QAction*)obs_frontend_add_tools_menu_qaction(menuActionText);
	QObject::connect(menuAction, &QAction::triggered, [] {
		// The settings dialog belongs to the main window. Should be ok
		// to pass the pointer to this QAction belonging to the main window
		settingsDialog->ToggleShowHide();
	});

	// Setup event handler to start the server once OBS is ready
	auto eventCallback = [](enum obs_frontend_event event, void *param) {
		if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
			if (_config->ServerEnabled) {
				_server->start(_config->ServerPort, _config->LockToIPv4);
			}
			obs_frontend_remove_event_callback((obs_frontend_event_cb)param, nullptr);
		}
	};
	obs_frontend_add_event_callback(eventCallback, (void*)(obs_frontend_event_cb)eventCallback);

	// Loading finished
	blog(LOG_INFO, "module loaded!");

	return true;
}

void obs_module_unload() {
	_server->stop();

	_eventsSystem.reset();
	_server.reset();
	_config.reset();

	blog(LOG_INFO, "goodbye!");
}

ConfigPtr GetConfig() {
	return _config;
}

WSServerPtr GetServer() {
	return _server;
}

WSEventsPtr GetEventsSystem() {
	return _eventsSystem;
}

void ShowPasswordSetting() {
	if (settingsDialog) {
		settingsDialog->PreparePasswordEntry();
		settingsDialog->setVisible(true);
	}
}
