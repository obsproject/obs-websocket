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

#pragma once

#include <atomic>
#include <QString>
#include <util/config-file.h>

#include "plugin-macros.generated.h"

struct Config {
	Config();
	void Load();
	void Save();
	void SetDefaultsToGlobalStore();
	config_t* GetConfigStore();

	std::atomic<bool> PortOverridden;
	std::atomic<bool> PasswordOverridden;

	std::atomic<bool> FirstLoad;
	std::atomic<bool> ServerEnabled;
	std::atomic<uint16_t> ServerPort;
	std::atomic<bool> DebugEnabled;
	std::atomic<bool> AlertsEnabled;
	std::atomic<bool> AuthRequired;
	QString ServerPassword;
};
