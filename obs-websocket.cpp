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

#include "obs-websocket.h"
#include "WSEvents.h"
#include "WSServer.h"
#include "Config.h"
#include "forms/settings-dialog.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-websocket", "en-US")

WSEvents *eventHandler;
WSServer *server;
SettingsDialog *settings_dialog;

bool obs_module_load(void) 
{
	blog(LOG_INFO, "[obs-websockets] you can haz websockets (version %s)", OBS_WEBSOCKET_VERSION);
	
	server = new WSServer(4444);
	eventHandler = new WSEvents(server);
	
	obs_frontend_add_save_callback(Config::OBSSaveCallback, Config::Current());

	QAction *menu_action = (QAction*)obs_frontend_add_tools_menu_qaction(obs_module_text("Menu.SettingsItem"));

	obs_frontend_push_ui_translation(obs_module_get_string);
	settings_dialog = new SettingsDialog();
	obs_frontend_pop_ui_translation();

	auto menu_cb = [] {
		settings_dialog->ToggleShowHide();
	};
	menu_action->connect(menu_action, &QAction::triggered, menu_cb);

	return true;
}

void obs_module_unload()
{
	blog(LOG_INFO, "[obs-websockets] goodbye !");
}

