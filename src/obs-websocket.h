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

#include <obs.hpp>
#include <memory>

void ___source_dummy_addref(obs_source_t*);
void ___sceneitem_dummy_addref(obs_sceneitem_t*);
void ___data_dummy_addref(obs_data_t*);
void ___data_array_dummy_addref(obs_data_array_t*);
void ___output_dummy_addref(obs_output_t*);

using OBSSourceAutoRelease =
	OBSRef<obs_source_t*, ___source_dummy_addref, obs_source_release>;
using OBSSceneItemAutoRelease =
	OBSRef<obs_sceneitem_t*, ___sceneitem_dummy_addref, obs_sceneitem_release>;
using OBSDataAutoRelease =
	OBSRef<obs_data_t*, ___data_dummy_addref, obs_data_release>;
using OBSDataArrayAutoRelease =
	OBSRef<obs_data_array_t*, ___data_array_dummy_addref, obs_data_array_release>;
using OBSOutputAutoRelease =
	OBSRef<obs_output_t*, ___output_dummy_addref, obs_output_release>;

void ___data_item_dummy_addref(obs_data_item_t*);
void ___data_item_release(obs_data_item_t*);
using OBSDataItemAutoRelease =
	OBSRef<obs_data_item_t*, ___data_item_dummy_addref, ___data_item_release>;

class Config;
typedef std::shared_ptr<Config> ConfigPtr;

class WSServer;
typedef std::shared_ptr<WSServer> WSServerPtr;

class WSEvents;
typedef std::shared_ptr<WSEvents> WSEventsPtr;

ConfigPtr GetConfig();
WSServerPtr GetServer();
WSEventsPtr GetEventsSystem();
void ShowPasswordSetting();

#define OBS_WEBSOCKET_VERSION "4.9.1"

#define blog(level, msg, ...) blog(level, "[obs-websocket] " msg, ##__VA_ARGS__)
