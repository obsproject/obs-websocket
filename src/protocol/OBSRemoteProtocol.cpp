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

#include "OBSRemoteProtocol.h"
#include "../WSRequestHandler.h"

std::string buildResponse(QString messageId, QString status, obs_data_t* fields = nullptr) {
	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "message-id", messageId.toUtf8().constData());
	obs_data_set_string(response, "status", status.toUtf8().constData());

	if (fields) {
		obs_data_apply(response, fields);
	}

	std::string responseString = obs_data_get_json(response);
	return responseString;
}

std::string successResponse(QString messageId, obs_data_t* fields = nullptr) {
	return buildResponse(messageId, "ok", fields);
}

std::string errorResponse(QString messageId, QString errorMessage, obs_data_t* additionalFields = nullptr) {
	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_string(fields, "error", errorMessage.toUtf8().constData());
	return buildResponse(messageId, "error", fields);
}

OBSRemoteProtocol::OBSRemoteProtocol(WSRequestHandler& requestHandler) :
	_requestHandler(requestHandler)
{
}

std::string OBSRemoteProtocol::processMessage(std::string message)
{
	std::string msgContainer(message);
	const char* msg = msgContainer.c_str();

	OBSDataAutoRelease data = obs_data_create_from_json(msg);
	if (!data) {
		blog(LOG_ERROR, "invalid JSON payload received for '%s'", msg);
		return errorResponse(nullptr, "invalid JSON payload");
	}

	if (!obs_data_has_user_value(data, "request-type") || !obs_data_has_user_value(data, "message-id")) {
		return errorResponse(nullptr, "missing request parameters");
	}

	QString methodName = obs_data_get_string(data, "request-type");
	QString messageId = obs_data_get_string(data, "message-id");

	OBSDataAutoRelease params = obs_data_create();
	obs_data_apply(params, data);
	obs_data_unset_user_value(params, "request-type");
	obs_data_unset_user_value(params, "message-id");

	RpcRequest request(messageId, methodName, params);
	RpcResponse response = _requestHandler.processRequest(request);

	OBSData additionalFields = response.additionalFields();
	switch (response.status()) {
		case RpcResponse::Status::Ok:
			return successResponse(messageId, additionalFields);
		case RpcResponse::Status::Error:
			return errorResponse(messageId, response.errorMessage(), additionalFields);
	}

	return std::string();
}
