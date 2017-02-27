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

#ifndef UTILS_H
#define UTILS_H

#include <QSpinBox>
#include <stdio.h>
#include <obs-module.h>

class Utils
{
	public:
		static obs_data_array_t* GetSceneItems(obs_source_t *source);
		static obs_data_t* GetSceneItemData(obs_scene_item *item);
		static obs_sceneitem_t* GetSceneItemFromName(obs_source_t *source, const char *name);
		static obs_source_t* GetTransitionFromName(const char *search_name);
		static obs_source_t* GetSceneFromNameOrCurrent(const char *scene_name);

		static obs_data_array_t* GetScenes();
		static obs_data_t* GetSceneData(obs_source *source);

		static obs_data_array_t* GetSceneCollections();
		static obs_data_array_t* GetProfiles();

		static QSpinBox* GetTransitionDurationControl();
		static int GetTransitionDuration();
		static void SetTransitionDuration(int ms);

		static const char* OBSVersionString();
};

#endif // UTILS_H
