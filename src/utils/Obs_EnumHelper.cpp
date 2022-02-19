/*
obs-websocket
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

#include "Obs.h"
#include "../plugin-macros.generated.h"

#define RET_COMPARE(str, x) if (str == #x) return x;

enum obs_bounds_type Utils::Obs::EnumHelper::GetSceneItemBoundsType(std::string boundsType)
{
	RET_COMPARE(boundsType, OBS_BOUNDS_NONE)
	RET_COMPARE(boundsType, OBS_BOUNDS_STRETCH)
	RET_COMPARE(boundsType, OBS_BOUNDS_SCALE_INNER)
	RET_COMPARE(boundsType, OBS_BOUNDS_SCALE_OUTER)
	RET_COMPARE(boundsType, OBS_BOUNDS_SCALE_TO_WIDTH)
	RET_COMPARE(boundsType, OBS_BOUNDS_SCALE_TO_HEIGHT)
	RET_COMPARE(boundsType, OBS_BOUNDS_MAX_ONLY)

	return OBS_BOUNDS_NONE;
}

enum ObsMediaInputAction Utils::Obs::EnumHelper::GetMediaInputAction(std::string mediaAction)
{
	RET_COMPARE(mediaAction, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PLAY)
	RET_COMPARE(mediaAction, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PAUSE)
	RET_COMPARE(mediaAction, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_STOP)
	RET_COMPARE(mediaAction, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_RESTART)
	RET_COMPARE(mediaAction, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_NEXT)
	RET_COMPARE(mediaAction, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PREVIOUS)

	return OBS_WEBSOCKET_MEDIA_INPUT_ACTION_NONE;
}

enum obs_blending_type Utils::Obs::EnumHelper::GetSceneItemBlendMode(std::string mode)
{
	RET_COMPARE(mode, OBS_BLEND_NORMAL)
	RET_COMPARE(mode, OBS_BLEND_ADDITIVE)
	RET_COMPARE(mode, OBS_BLEND_SUBTRACT)
	RET_COMPARE(mode, OBS_BLEND_SCREEN)
	RET_COMPARE(mode, OBS_BLEND_MULTIPLY)
	RET_COMPARE(mode, OBS_BLEND_LIGHTEN)
	RET_COMPARE(mode, OBS_BLEND_DARKEN)

	return OBS_BLEND_NORMAL;
}
