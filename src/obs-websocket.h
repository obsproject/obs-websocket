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

#include <memory>
#include <obs.hpp>
#ifdef _MSC_VER
    #pragma push_macro("strtoll")
#endif
#include <util/platform.h>
#ifdef _MSC_VER
    #pragma pop_macro("strtoll")
#endif

#include "plugin-macros.generated.h"

// Autorelease object definitions
void ___source_dummy_addref(obs_source_t*);
void ___scene_dummy_addref(obs_scene_t*);
void ___sceneitem_dummy_addref(obs_sceneitem_t*);
void ___data_dummy_addref(obs_data_t*);
void ___data_array_dummy_addref(obs_data_array_t*);
void ___output_dummy_addref(obs_output_t*);
void ___data_item_dummy_addref(obs_data_item_t*);
void ___data_item_release(obs_data_item_t*);
void ___properties_dummy_addref(obs_properties_t*);

using OBSSourceAutoRelease = OBSRef<obs_source_t*, ___source_dummy_addref, obs_source_release>;
using OBSSceneAutoRelease = OBSRef<obs_scene_t*, ___scene_dummy_addref, obs_scene_release>;
using OBSSceneItemAutoRelease = OBSRef<obs_sceneitem_t*, ___sceneitem_dummy_addref, obs_sceneitem_release>;
using OBSDataAutoRelease = OBSRef<obs_data_t*, ___data_dummy_addref, obs_data_release>;
using OBSDataArrayAutoRelease = OBSRef<obs_data_array_t*, ___data_array_dummy_addref, obs_data_array_release>;
using OBSOutputAutoRelease = OBSRef<obs_output_t*, ___output_dummy_addref, obs_output_release>;
using OBSDataItemAutoRelease = OBSRef<obs_data_item_t*, ___data_item_dummy_addref, ___data_item_release>;
using OBSPropertiesAutoDestroy = OBSRef<obs_properties_t*, ___properties_dummy_addref, obs_properties_destroy>;

class Config;
typedef std::shared_ptr<Config> ConfigPtr;

class WebSocketApi;
typedef std::shared_ptr<WebSocketApi> WebSocketApiPtr;

class WebSocketServer;
typedef std::shared_ptr<WebSocketServer> WebSocketServerPtr;

class EventHandler;
typedef std::shared_ptr<EventHandler> EventHandlerPtr;

ConfigPtr GetConfig();

WebSocketApiPtr GetWebSocketApi();

WebSocketServerPtr GetWebSocketServer();

EventHandlerPtr GetEventHandler();

os_cpu_usage_info_t* GetCpuUsageInfo();

bool IsDebugEnabled();
