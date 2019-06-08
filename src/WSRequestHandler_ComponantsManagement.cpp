/**
* obs-websocket
* Copyright (C) 2019	Ilyes KAANICH <https://github.com/ilyes64>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <https://www.gnu.org/licenses/>
*/
#include <QtCore/QString>
#include <QtCore/QBuffer>
#include <QtCore/QFileInfo>
#include <QtGui/QImage>
#include <QtGui/QImageWriter>

#include "Utils.h"

#include "WSRequestHandler.h"
/**
* Create a new source
*
* @param {String} `sourceName` Name of the source to be created
* @param {String} `typeIDName` type of the source to be created
* @param {Object} `sourceSettings` New settings. These will be merged to the new created source.
* @param {Object} `hotkeyData` hotkeyData. These will be merged to the new created source.
*
* @api requests
* @name AddNewSource
* @category sources
*/
HandlerResponse WSRequestHandler::HandleCreateNewSource(WSRequestHandler* req)
{
	if (!req->hasField("sourceName") || !req->hasField("typeIDName") || !req->hasField("sourceSettings")) {
		return req->SendErrorResponse("missing request parameters");
	}
	const char* sourceName = obs_data_get_string(req->data, "sourceName");
	const char* typeIDName = obs_data_get_string(req->data, "typeIDName");
	
	OBSDataAutoRelease newsourceSettings = obs_data_get_obj(req->data, "sourceSettings");
	OBSDataAutoRelease hotkeyData = obs_data_get_obj(req->data, "hotkeyData");
	
	obs_source_t* source = obs_source_create(typeIDName, sourceName, newsourceSettings, hotkeyData);
	obs_source_set_enabled(source, true);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "sourceName", obs_source_get_name(source));
	obs_data_set_string(response, "sourceType", obs_source_get_id(source));
	obs_data_set_obj(response, "sourceSettings", newsourceSettings);
	obs_data_set_obj(response, "hotkeyData", hotkeyData);
	return req->SendOKResponse(response);
}

/**
* Add New Scene
*
* @param {String} `sceneName` Name of the source to be created
*
* @api requests
* @name AddNewScene
* @category sources
*/
HandlerResponse WSRequestHandler::HandleAddNewScene(WSRequestHandler* req)
{
	if (!req->hasField("sourceName") || !req->hasField("typeIDName") || !req->hasField("sourceSettings")) {
		return req->SendErrorResponse("missing request parameters");
	}
	const char* sceneName = obs_data_get_string(req->data, "sceneName");
	obs_scene_create(sceneName);
	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "sceneName", sceneName);
	return req->SendOKResponse(response);
}

/**
* Add New Scene
*
* @param {String} `sourceParentName` Name of the parent source
* @param {String} `sourceChildName` Name of the child source
*
* @api requests
* @name AddActiveChild
* @category sources
*/
HandlerResponse WSRequestHandler::HandleAddActiveChild(WSRequestHandler* req)
{
	if (!req->hasField("sourceParentName") || !req->hasField("sourceChildName")) {
		return req->SendErrorResponse("missing request parameters");
	}
	const char* sourceParentName = obs_data_get_string(req->data, "sourceParentName");
	const char* sourceChildName = obs_data_get_string(req->data, "sourceChildName");
	obs_source_t* sourceParent= obs_get_source_by_name(sourceParentName);
	obs_source_t* sourceChild = obs_get_source_by_name(sourceChildName);
	
	obs_source_add_active_child(sourceParent, sourceChild);
	return req->SendOKResponse();
}