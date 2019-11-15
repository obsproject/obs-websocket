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
	
	const QString& messageId() const;
	const QString& methodName() const;
	const obs_data_t* parameters() const;

	bool hasField(QString fieldName, obs_data_type expectedFieldType = OBS_DATA_NULL,
					obs_data_number_type expectedNumberType = OBS_DATA_NUM_INVALID);
	bool hasBool(QString fieldName);
	bool hasString(QString fieldName);
	bool hasNumber(QString fieldName, obs_data_number_type expectedNumberType = OBS_DATA_NUM_INVALID);
	bool hasInteger(QString fieldName);
	bool hasDouble(QString fieldName);
	bool hasArray(QString fieldName);
	bool hasObject(QString fieldName);

private:
	const QString _messageId;
	const QString _methodName;
	OBSDataAutoRelease _parameters;
};
