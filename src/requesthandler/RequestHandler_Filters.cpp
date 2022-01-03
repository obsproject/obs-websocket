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
 * Gets the info for a specific source filter.
 *
 * @requestField sourceName | String | Name of the source
 * @requestField filterName | String | Name of the filter
 *
 * @responseField filterEnabled  | Boolean | Whether the filter is enabled
 * @responseField filterIndex    | Number  | Index of the filter in the list, beginning at 0
 * @responseField filterKind     | String  | The kind of filter
 * @responseField filterSettings | Object  | Settings object associated with the filter
 *
 * @requestType GetSourceFilter
 * @complexity 2
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api requests
 * @category filters
 */
RequestResult RequestHandler::GetSourceFilter(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	FilterPair pair = request.ValidateFilter("sourceName", "filterName", statusCode, comment);
	if (!pair.filter)
		return RequestResult::Error(statusCode, comment);

	json responseData;
	responseData["filterEnabled"] = obs_source_enabled(pair.filter);
	responseData["filterIndex"] = Utils::Obs::NumberHelper::GetSourceFilterIndex(pair.source, pair.filter); // Todo: Use `GetSourceFilterlist` to select this filter maybe
	responseData["filterKind"] = obs_source_get_id(pair.filter);

	OBSDataAutoRelease filterSettings = obs_source_get_settings(pair.filter);
	responseData["filterSettings"] = Utils::Json::ObsDataToJson(filterSettings);

	return RequestResult::Success(responseData);
}
