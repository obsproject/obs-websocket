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
#include <QtCore/QString>

class WSRequestHandler;
class RpcEvent;

class OBSRemoteProtocol
{
public:
	std::string processMessage(WSRequestHandler& requestHandler, std::string message);
	std::string encodeEvent(const RpcEvent& event);

private:
	std::string buildResponse(QString messageId, QString status, obs_data_t* fields = nullptr);
	std::string successResponse(QString messageId, obs_data_t* fields = nullptr);
	std::string errorResponse(QString messageId, QString errorMessage, obs_data_t* additionalFields = nullptr);
};
