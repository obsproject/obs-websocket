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

#include <obs-frontend-api.h>
#include <util/config-file.h>

#include "Config.h"
#include "Utils.h"

#define SECTION_NAME "WebsocketAPI"
#define PARAM_ENABLE "ServerEnabled"
#define PARAM_PORT "ServerPort"
#define PARAM_DEBUG "DebugEnabled"
#define PARAM_ALERT "AlertsEnabled"
#define PARAM_AUTHREQUIRED "AuthRequired"
#define PARAM_PASSWORD "AuthPassword"

#define QT_TO_UTF8(str) str.toUtf8().constData()

QSharedPointer<Config> Config::_instance = nullptr;

Config::Config() :
	ServerEnabled(true),
	ServerPort(4444),
	DebugEnabled(false),
	AlertsEnabled(true),
	AuthRequired(false),
	AuthPassword(""),
	SettingsLoaded(false)
{
	// OBS Config defaults
	config_t* obsConfig = obs_frontend_get_global_config();
	if (obsConfig) {
		config_set_default_bool(obsConfig,
			SECTION_NAME, PARAM_ENABLE, ServerEnabled);
		config_set_default_uint(obsConfig,
			SECTION_NAME, PARAM_PORT, ServerPort);

		config_set_default_bool(obsConfig,
			SECTION_NAME, PARAM_DEBUG, DebugEnabled);
		config_set_default_bool(obsConfig,
			SECTION_NAME, PARAM_ALERT, AlertsEnabled);

		config_set_default_bool(obsConfig,
			SECTION_NAME, PARAM_AUTHREQUIRED, AuthRequired);
		config_set_default_string(obsConfig,
			SECTION_NAME, PARAM_PASSWORD, QT_TO_UTF8(AuthPassword));
	}
}

Config::~Config() = default;

void Config::Load() {
	config_t* obsConfig = obs_frontend_get_global_config();

	ServerEnabled = config_get_bool(obsConfig, SECTION_NAME, PARAM_ENABLE);
	ServerPort = config_get_uint(obsConfig, SECTION_NAME, PARAM_PORT);

	DebugEnabled = config_get_bool(obsConfig, SECTION_NAME, PARAM_DEBUG);
	AlertsEnabled = config_get_bool(obsConfig, SECTION_NAME, PARAM_ALERT);

	AuthRequired = config_get_bool(obsConfig, SECTION_NAME, PARAM_AUTHREQUIRED);
	AuthPassword = config_get_string(obsConfig, SECTION_NAME, PARAM_PASSWORD);
}

void Config::Save() {
	config_t* obsConfig = obs_frontend_get_global_config();

	config_set_bool(obsConfig, SECTION_NAME, PARAM_ENABLE, ServerEnabled);
	config_set_uint(obsConfig, SECTION_NAME, PARAM_PORT, ServerPort);

	config_set_bool(obsConfig, SECTION_NAME, PARAM_DEBUG, DebugEnabled);
	config_set_bool(obsConfig, SECTION_NAME, PARAM_ALERT, AlertsEnabled);

	config_set_bool(obsConfig, SECTION_NAME, PARAM_AUTHREQUIRED, AuthRequired);
	config_set_string(obsConfig, SECTION_NAME, PARAM_PASSWORD, QT_TO_UTF8(AuthPassword));

	config_save(obsConfig);
}
