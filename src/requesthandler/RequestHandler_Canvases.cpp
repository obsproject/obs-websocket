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
 * @responseField canvases                  | Array<Object> | Array of canvases
 *
 * @requestType GetCanvasList
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category scenes
 */
RequestResult RequestHandler::GetCanvasList(const Request &request)
{
	json responseData;
	std::vector<json> ret;

	obs_enum_canvases(
		[](void *param, obs_canvas_t *canvas) {
			auto ret = static_cast<std::vector<json> *>(param);
			json canvasJson;
			canvasJson["canvasName"] = obs_canvas_get_name(canvas);
			canvasJson["canvasUuid"] = obs_canvas_get_uuid(canvas);
			struct obs_video_info ovi;
			if (obs_canvas_get_video_info(canvas, &ovi)) {
				canvasJson["fpsNumerator"] = ovi.fps_num;
				canvasJson["fpsDenominator"] = ovi.fps_den;
				canvasJson["baseWidth"] = ovi.base_width;
				canvasJson["baseHeight"] = ovi.base_height;
				canvasJson["outputWidth"] = ovi.output_width;
				canvasJson["outputHeight"] = ovi.output_height;
			}
			ret->push_back(canvasJson);
			return true;
		},
		&ret);
	responseData["canvases"] = ret;

	return RequestResult::Success(responseData);
}
