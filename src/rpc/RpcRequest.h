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

#include <obs-data.h>
#include <QtCore/QString>
#include "../obs-websocket.h"

class RpcRequest
{
public:
	explicit RpcRequest(const QString& messageId, const QString& methodName, obs_data_t* params);
	
	const QString& messageId() const {
		return _messageId;
	}
	
	const QString& methodName() const {
		return _methodName;
	}
	
	const obs_data_t* parameters() const {
		return _parameters;
	}

private:
	const QString _messageId;
	const QString _methodName;
	OBSDataAutoRelease _parameters;
};
