/*
obs-websocket
Copyright (C) 2016-2019	Stéphane Lepin <stephane.lepin@gmail.com>

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

#include <inttypes.h>

#include "OBSRemoteProtocol.h"
#include "../WSRequestHandler.h"
#include "../rpc/RpcEvent.h"
#include "../Utils.h"

std::string OBSRemoteProtocol::processMessage(WSRequestHandler& requestHandler, std::string message)
{
	std::string msgContainer(message);
	const char* msg = msgContainer.c_str();

	OBSDataAutoRelease data = obs_data_create_from_json(msg);
	if (!data) {
		blog(LOG_ERROR, "invalid JSON payload received for '%s'", msg);
		return jsonDataToString(
			errorResponse(nullptr, "invalid JSON payload")
		);
	}

	if (!obs_data_has_user_value(data, "request-type") || !obs_data_has_user_value(data, "message-id")) {
		return jsonDataToString(
			errorResponse(nullptr, "missing request parameters")
		);
	}

	QString methodName = obs_data_get_string(data, "request-type");
	QString messageId = obs_data_get_string(data, "message-id");

	OBSDataAutoRelease params = obs_data_create();
	obs_data_apply(params, data);
	obs_data_unset_user_value(params, "request-type");
	obs_data_unset_user_value(params, "message-id");

	RpcRequest request(messageId, methodName, params);
	RpcResponse response = requestHandler.processRequest(request);

	OBSDataAutoRelease responseData = rpcResponseToJsonData(response);
	return jsonDataToString(responseData);
}

std::string OBSRemoteProtocol::encodeEvent(const RpcEvent& event)
{
	OBSDataAutoRelease eventData = obs_data_create();

	QString updateType = event.updateType();
	obs_data_set_string(eventData, "update-type", updateType.toUtf8().constData());

	std::optional<uint64_t> streamTime = event.streamTime();
	if (streamTime.has_value()) {
		QString streamingTimecode = Utils::nsToTimestamp(streamTime.value());
		obs_data_set_string(eventData, "stream-timecode", streamingTimecode.toUtf8().constData());
	}

	std::optional<uint64_t> recordingTime = event.recordingTime();
	if (recordingTime.has_value()) {
		QString recordingTimecode = Utils::nsToTimestamp(recordingTime.value());
		obs_data_set_string(eventData, "rec-timecode", recordingTimecode.toUtf8().constData());
	}

	OBSData additionalFields = event.additionalFields();
	if (additionalFields) {
		obs_data_apply(eventData, additionalFields);
	}

	return std::string(obs_data_get_json(eventData));
}

obs_data_t* OBSRemoteProtocol::rpcResponseToJsonData(const RpcResponse& response)
{
	QByteArray messageIdBytes = response.messageId().toUtf8();
	const char* messageId = messageIdBytes.constData();

	OBSData additionalFields = response.additionalFields();
	switch (response.status()) {
		case RpcResponse::Status::Ok:
			return successResponse(messageId, additionalFields);
		case RpcResponse::Status::Error:
			return errorResponse(messageId, response.errorMessage().toUtf8().constData(), additionalFields);
		default:
			assert(false);
	}

	return nullptr;
}

obs_data_t* OBSRemoteProtocol::successResponse(const char* messageId, obs_data_t* fields)
{
	return buildResponse(messageId, "ok", fields);
}

obs_data_t* OBSRemoteProtocol::errorResponse(const char* messageId, const char* errorMessage, obs_data_t* additionalFields)
{
	OBSDataAutoRelease fields = obs_data_create();
	if (additionalFields) {
		obs_data_apply(fields, additionalFields);
	}
	obs_data_set_string(fields, "error", errorMessage);
	return buildResponse(messageId, "error", fields);
}

obs_data_t* OBSRemoteProtocol::buildResponse(const char* messageId, const char* status, obs_data_t* fields)
{
	obs_data_t* response = obs_data_create();
	if (messageId) {
		obs_data_set_string(response, "message-id", messageId);
	}
	obs_data_set_string(response, "status", status);

	if (fields) {
		obs_data_apply(response, fields);
	}

	return response;
}

std::string OBSRemoteProtocol::jsonDataToString(obs_data_t *data)
{
	std::string responseString = obs_data_get_json(data);
	return responseString;
}
