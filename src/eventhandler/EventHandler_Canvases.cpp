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

#include "EventHandler.h"

/**
 * A new canvas has been created.
 *
 * @dataField canvasName | String  | Name of the new canvas
 * @dataField canvasUuid | String  | UUID of the new canvas
 *
 * @eventType CanvasCreated
 * @eventSubscription Canvases
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.7.0
 * @api events
 * @category canvas
 */
void EventHandler::HandleCanvasCreated(obs_canvas_t *canvas)
{
	json eventData;
	eventData["canvasName"] = obs_canvas_get_name(canvas);
	eventData["canvasUuid"] = obs_canvas_get_uuid(canvas);
	BroadcastEvent(EventSubscription::Canvases, "CanvasCreated", eventData);
}

/**
 * A canvas has been removed.
 *
 * @dataField canvasName | String  | Name of the removed canvas
 * @dataField canvasUuid | String  | UUID of the removed canvas
 *
 * @eventType CanvasRemoved
 * @eventSubscription Canvases
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.7.0
 * @api events
 * @category canvas
 */
void EventHandler::HandleCanvasRemoved(obs_canvas_t *canvas)
{
	json eventData;
	eventData["canvasName"] = obs_canvas_get_name(canvas);
	eventData["canvasUuid"] = obs_canvas_get_uuid(canvas);
	BroadcastEvent(EventSubscription::Canvases, "CanvasRemoved", eventData);
}

/**
 * The name of a canvas has changed.
 *
 * @dataField canvasUuid    | String | UUID of the canvas
 * @dataField oldCanvasName | String | Old name of the canvas
 * @dataField canvasName    | String | New name of the canvas
 *
 * @eventType CanvasNameChanged
 * @eventSubscription Canvases
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.7.0
 * @api events
 * @category canvas
 */
void EventHandler::HandleCanvasNameChanged(obs_canvas_t *canvas, std::string oldCanvasName, std::string canvasName)
{
	json eventData;
	eventData["canvasUuid"] = obs_canvas_get_uuid(canvas);
	eventData["oldCanvasName"] = oldCanvasName;
	eventData["canvasName"] = canvasName;
	BroadcastEvent(EventSubscription::Canvases, "CanvasNameChanged", eventData);
}
