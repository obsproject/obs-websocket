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

#pragma once

#include <string>
#include <obs-data.h>

#include "../rpc/RpcResponse.h"

class WSRequestHandler;
class RpcEvent;

class OBSRemoteProtocol
{
public:
	static std::string processMessage(WSRequestHandler& requestHandler, std::string message);
	static std::string encodeEvent(const RpcEvent& event);
	static obs_data_t* rpcResponseToJsonData(const RpcResponse& response);

private:
	static obs_data_t* successResponse(const char* messageId, obs_data_t* fields = nullptr);
	static obs_data_t* errorResponse(const char* messageId, const char* errorMessage, obs_data_t* additionalFields = nullptr);
	static obs_data_t* buildResponse(const char* messageId, const char*, obs_data_t* fields = nullptr);
	static std::string jsonDataToString(obs_data_t *data);
};
