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

#include "RequestHandler.h"

/**
 * Gets an array of canvases in OBS.
 * 
 * @responseField canvases | Array<Object> | Array of canvases
 *
 * @requestType GetCanvasList
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.7.0
 * @api requests
 * @category canvases
 */
RequestResult RequestHandler::GetCanvasList(const Request &)
{
	json responseData;
	std::vector<json> canvases;

	obs_enum_canvases(
		[](void *param, obs_canvas_t *canvas) {
			auto canvases = static_cast<std::vector<json> *>(param);

			json canvasJson;
			canvasJson["canvasName"] = obs_canvas_get_name(canvas);
			canvasJson["canvasUuid"] = obs_canvas_get_uuid(canvas);

			auto flags = obs_canvas_get_flags(canvas);
			json canvasFlags;
			canvasFlags["MAIN"] = !!(flags & MAIN);
			canvasFlags["ACTIVATE"] = !!(flags & ACTIVATE);
			canvasFlags["MIX_AUDIO"] = !!(flags & MIX_AUDIO);
			canvasFlags["SCENE_REF"] = !!(flags & SCENE_REF);
			canvasFlags["EPHEMERAL"] = !!(flags & EPHEMERAL);
			canvasJson["canvasFlags"] = canvasFlags;

			canvasJson["canvasVideoSettings"] = Utils::Obs::ObjectHelper::GetCanvasVideoSettings(canvas);
			canvases->push_back(canvasJson);
			return true;
		},
		&canvases);
	responseData["canvases"] = canvases;

	return RequestResult::Success(responseData);
}
