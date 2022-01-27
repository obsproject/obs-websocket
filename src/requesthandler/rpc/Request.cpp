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

#include "Request.h"
#include "../../obs-websocket.h"

json GetDefaultJsonObject(const json &requestData)
{
	// Always provide an object to prevent exceptions while running checks in requests
	if (!requestData.is_object())
		return json::object();
	else
		return requestData;
}

Request::Request(const std::string &requestType, const json &requestData, const RequestBatchExecutionType::RequestBatchExecutionType executionType) :
	RequestType(requestType),
	HasRequestData(requestData.is_object()),
	RequestData(GetDefaultJsonObject(requestData)),
	ExecutionType(executionType)
{
}

bool Request::Contains(const std::string &keyName) const
{
	return (RequestData.contains(keyName) && !RequestData[keyName].is_null());
}

bool Request::ValidateBasic(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const
{
	if (!HasRequestData) {
		statusCode = RequestStatus::MissingRequestData;
		comment = "Your request data is missing or invalid (non-object)";
		return false;
	}

	if (!RequestData.contains(keyName) || RequestData[keyName].is_null()) {
		statusCode = RequestStatus::MissingRequestField;
		comment = std::string("Your request is missing the `") + keyName + "` field.";
		return false;
	}

	return true;
}

bool Request::ValidateOptionalNumber(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const double minValue, const double maxValue) const
{
	if (!RequestData[keyName].is_number()) {
		statusCode = RequestStatus::InvalidRequestFieldType;
		comment = std::string("The field value of `") + keyName + "` must be a number.";
		return false;
	}

	double value = RequestData[keyName];
	if (value < minValue) {
		statusCode = RequestStatus::RequestFieldOutOfRange;
		comment = std::string("The field value of `") + keyName + "` is below the minimum of `" + std::to_string(minValue) + "`";
		return false;
	}
	if (value > maxValue) {
		statusCode = RequestStatus::RequestFieldOutOfRange;
		comment = std::string("The field value of `") + keyName + "` is above the maximum of `" + std::to_string(maxValue) + "`";
		return false;
	}

	return true;
}

bool Request::ValidateNumber(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const double minValue, const double maxValue) const
{
	if (!ValidateBasic(keyName, statusCode, comment))
		return false;

	if (!ValidateOptionalNumber(keyName, statusCode, comment, minValue, maxValue))
		return false;

	return true;
}

bool Request::ValidateOptionalString(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty) const
{
	if (!RequestData[keyName].is_string()) {
		statusCode = RequestStatus::InvalidRequestFieldType;
		comment = std::string("The field value of `") + keyName + "` must be a string.";
		return false;
	}

	if (RequestData[keyName].get<std::string>().empty() && !allowEmpty) {
		statusCode = RequestStatus::RequestFieldEmpty;
		comment = std::string("The field value of `") + keyName + "` must not be empty.";
		return false;
	}

	return true;
}

bool Request::ValidateString(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty) const
{
	if (!ValidateBasic(keyName, statusCode, comment))
		return false;

	if (!ValidateOptionalString(keyName, statusCode, comment, allowEmpty))
		return false;

	return true;
}

bool Request::ValidateOptionalBoolean(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const
{
	if (!RequestData[keyName].is_boolean()) {
		statusCode = RequestStatus::InvalidRequestFieldType;
		comment = std::string("The field value of `") + keyName + "` must be boolean.";
		return false;
	}

	return true;
}

bool Request::ValidateBoolean(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const
{
	if (!ValidateBasic(keyName, statusCode, comment))
		return false;

	if (!ValidateOptionalBoolean(keyName, statusCode, comment))
		return false;

	return true;
}

bool Request::ValidateOptionalObject(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty) const
{
	if (!RequestData[keyName].is_object()) {
		statusCode = RequestStatus::InvalidRequestFieldType;
		comment = std::string("The field value of `") + keyName + "` must be an object.";
		return false;
	}

	if (RequestData[keyName].empty() && !allowEmpty) {
		statusCode = RequestStatus::RequestFieldEmpty;
		comment = std::string("The field value of `") + keyName + "` must not be empty.";
		return false;
	}

	return true;
}

bool Request::ValidateObject(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty) const
{
	if (!ValidateBasic(keyName, statusCode, comment))
		return false;

	if (!ValidateOptionalObject(keyName, statusCode, comment, allowEmpty))
		return false;

	return true;
}

bool Request::ValidateOptionalArray(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty) const
{
	if (!RequestData[keyName].is_array()) {
		statusCode = RequestStatus::InvalidRequestFieldType;
		comment = std::string("The field value of `") + keyName + "` must be an array.";
		return false;
	}

	if (RequestData[keyName].empty() && !allowEmpty) {
		statusCode = RequestStatus::RequestFieldEmpty;
		comment = std::string("The field value of `") + keyName + "` must not be empty.";
		return false;
	}

	return true;
}

bool Request::ValidateArray(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty) const
{
	if (!ValidateBasic(keyName, statusCode, comment))
		return false;

	if (!ValidateOptionalArray(keyName, statusCode, comment, allowEmpty))
		return false;

	return true;
}

obs_source_t *Request::ValidateSource(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const
{
	if (!ValidateString(keyName, statusCode, comment))
		return nullptr;

	std::string sourceName = RequestData[keyName];

	obs_source_t *ret = obs_get_source_by_name(sourceName.c_str());
	if (!ret) {
		statusCode = RequestStatus::ResourceNotFound;
		comment = std::string("No source was found by the name of `") + sourceName + "`.";
		return nullptr;
	}

	return ret;
}

obs_source_t *Request::ValidateScene(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const ObsWebSocketSceneFilter filter) const
{
	obs_source_t *ret = ValidateSource(keyName, statusCode, comment);
	if (!ret)
		return nullptr;

	if (obs_source_get_type(ret) != OBS_SOURCE_TYPE_SCENE) {
		obs_source_release(ret);
		statusCode = RequestStatus::InvalidResourceType;
		comment = "The specified source is not a scene.";
		return nullptr;
	}

	bool isGroup = obs_source_is_group(ret);
	if (filter == OBS_WEBSOCKET_SCENE_FILTER_SCENE_ONLY && isGroup) {
		obs_source_release(ret);
		statusCode = RequestStatus::InvalidResourceType;
		comment = "The specified source is not a scene. (Is group)";
		return nullptr;
	} else if (filter == OBS_WEBSOCKET_SCENE_FILTER_GROUP_ONLY && !isGroup) {
		obs_source_release(ret);
		statusCode = RequestStatus::InvalidResourceType;
		comment = "The specified source is not a group. (Is scene)";
		return nullptr;
	}

	return ret;
}

obs_scene_t *Request::ValidateScene2(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const ObsWebSocketSceneFilter filter) const
{
	OBSSourceAutoRelease sceneSource = ValidateSource(keyName, statusCode, comment);
	if (!sceneSource)
		return nullptr;

	if (obs_source_get_type(sceneSource) != OBS_SOURCE_TYPE_SCENE) {
		statusCode = RequestStatus::InvalidResourceType;
		comment = "The specified source is not a scene.";
		return nullptr;
	}

	bool isGroup = obs_source_is_group(sceneSource);
	if (isGroup) {
		if (filter == OBS_WEBSOCKET_SCENE_FILTER_SCENE_ONLY) {
			statusCode = RequestStatus::InvalidResourceType;
			comment = "The specified source is not a scene. (Is group)";
			return nullptr;
		}
		return obs_scene_get_ref(obs_group_from_source(sceneSource));
	} else {
		if (filter == OBS_WEBSOCKET_SCENE_FILTER_GROUP_ONLY) {
			statusCode = RequestStatus::InvalidResourceType;
			comment = "The specified source is not a group. (Is scene)";
			return nullptr;
		}
		return obs_scene_get_ref(obs_scene_from_source(sceneSource));
	}
}

obs_source_t *Request::ValidateInput(const std::string &keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const
{
	obs_source_t *ret = ValidateSource(keyName, statusCode, comment);
	if (!ret)
		return nullptr;

	if (obs_source_get_type(ret) != OBS_SOURCE_TYPE_INPUT) {
		obs_source_release(ret);
		statusCode = RequestStatus::InvalidResourceType;
		comment = "The specified source is not an input.";
		return nullptr;
	}

	return ret;
}

FilterPair Request::ValidateFilter(const std::string &sourceKeyName, const std::string &filterKeyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const
{
	obs_source_t *source = ValidateSource(sourceKeyName, statusCode, comment);
	if (!source)
		return FilterPair{source, nullptr};

	if (!ValidateString(filterKeyName, statusCode, comment))
		return FilterPair{source, nullptr};

	std::string filterName = RequestData[filterKeyName];

	obs_source_t *filter = obs_source_get_filter_by_name(source, filterName.c_str());
	if (!filter) {
		statusCode = RequestStatus::ResourceNotFound;
		comment = std::string("No filter was found in the source `") + RequestData[sourceKeyName].get<std::string>() + "` with the name `" + filterName + "`.";
		return FilterPair{source, nullptr};
	}

	return FilterPair{source, filter};
}

obs_sceneitem_t *Request::ValidateSceneItem(const std::string &sceneKeyName, const std::string &sceneItemIdKeyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const ObsWebSocketSceneFilter filter) const
{
	OBSSceneAutoRelease scene = ValidateScene2(sceneKeyName, statusCode, comment, filter);
	if (!scene)
		return nullptr;

	if (!ValidateNumber(sceneItemIdKeyName, statusCode, comment, 0))
		return nullptr;

	int64_t sceneItemId = RequestData[sceneItemIdKeyName];

	OBSSceneItem sceneItem = obs_scene_find_sceneitem_by_id(scene, sceneItemId);
	if (!sceneItem) {
		statusCode = RequestStatus::ResourceNotFound;
		comment = std::string("No scene items were found in scene `") + RequestData[sceneKeyName].get<std::string>() + "` with the ID `" + std::to_string(sceneItemId) + "`.";
		return nullptr;
	}

	obs_sceneitem_addref(sceneItem);
	return sceneItem;
}
