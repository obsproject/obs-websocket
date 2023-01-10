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

/**
 * @typedef Filter
 * @property {Boolean} filterEnabled
 * @property {Number} filterIndex
 * @property {String} filterKind
 * @property {String} filterName
 * @property {Object} filterSettings
 * @api typedefs
 */

/**
 * @typedef MeterData
 * @property {String} inputName
 * @property {Array<Array<Number>>} inputLevelsMul
 * @api typedefs
 */

/**
 * @typedef SceneItemTransform
 * @property {Number} sourceWidth
 * @property {Number} sourceHeight
 * @property {Number} positionX
 * @property {Number} positionY
 * @property {Number} rotation
 * @property {Number} scaleX
 * @property {Number} scaleY
 * @property {Number} width
 * @property {Number} height
 * @property {Number} alignment
 * @property {Number} boundsType
 * @property {Number} boundsAlignment
 * @property {Number} boundsWith
 * @property {Number} boundsHeight
 * @property {Number} cropLeft
 * @property {Number} cropRight
 * @property {Number} cropTop
 * @property {Number} cropBottom
 * @api typedefs
 */

/**
 * @typedef SceneItem
 * @property {Number} sceneItemId
 * @property {Number} sceneItemIndex
 * @property {Boolean} sceneItemEnabled
 * @property {Boolean} sceneItemLocked
 * @property {SceneItemTransform} sceneItemTransform
 * @property {Number} sceneItemBlendMode
 * @property {String} sourceName
 * @property {Number} sourceType
 * @property {String|undefined} inputKind
 * @property {Boolean|undefined} isGroup
 * @api typedefs
 */

/**
 * @typedef Scene
 * @property {String} sceneName
 * @property {Number} sceneIndex
 * @api typedefs
 */

/**
 * @typedef Input
 * @property {String} inputName
 * @property {String} inputKind
 * @property {String} unversionedInputKind
 * @api typedefs
 */

/**
 * @typedef Output
 * @property {String} outputName
 * @property {String} outputKind
 * @property {Number} outputWidth
 * @property {Number} outputHeight
 * @property {Boolean} outputActive
 * @property {Number} outputFlags
 * @api typedefs
 */

/**
 * @typedef ListPropertyItem
 * @property {String} itemName
 * @property {Boolean} itemEnabled
 * @property {Number|String|undefined} itemValue
 * @api typedefs
 */

/**
 * @typedef Transition
 * @property {String} transitionName
 * @property {String} transitionKind
 * @property {Boolean} transitionFixed
 * @property {Boolean} transitionConfigurable
 * @api typedefs
 */

/**
 * @typedef Monitor
 * @property {String} monitorName
 * @property {Number} monitorIndex
 * @property {Number} monitorWidth
 * @property {Number} monitorHeight
 * @property {Number} monitorPositionX
 * @property {Number} monitorPositionY
 * @api typedefs
 */

#include <memory>
#include <obs.hpp>
#include <util/platform.h>

#include "utils/Obs.h"
#include "plugin-macros.generated.h"

struct Config;
typedef std::shared_ptr<Config> ConfigPtr;

class EventHandler;
typedef std::shared_ptr<EventHandler> EventHandlerPtr;

class WebSocketApi;
typedef std::shared_ptr<WebSocketApi> WebSocketApiPtr;

class WebSocketServer;
typedef std::shared_ptr<WebSocketServer> WebSocketServerPtr;

os_cpu_usage_info_t *GetCpuUsageInfo();

ConfigPtr GetConfig();

EventHandlerPtr GetEventHandler();

WebSocketApiPtr GetWebSocketApi();

WebSocketServerPtr GetWebSocketServer();

bool IsDebugEnabled();
