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

#include "RpcRequest.h"

RpcRequest::RpcRequest(const QString& messageId, const QString& methodName, obs_data_t* params) :
	_messageId(messageId),
	_methodName(methodName)
{
	_parameters = obs_data_create();
	obs_data_apply(_parameters, params);
}

const QString& RpcRequest::messageId() const
{
	return _messageId;
}
	
const QString& RpcRequest::methodName() const
{
	return _methodName;
}

const obs_data_t* RpcRequest::parameters() const
{
	return _parameters;
}

bool RpcRequest::hasField(QString name, obs_data_type expectedFieldType, obs_data_number_type expectedNumberType)
{
	if (!_parameters || name.isEmpty() || name.isNull()) {
		return false;
	}

	OBSDataItemAutoRelease dataItem = obs_data_item_byname(_parameters, name.toUtf8());
	if (!dataItem) {
		return false;
	}

	if (expectedFieldType != OBS_DATA_NULL) {
		obs_data_type fieldType = obs_data_item_gettype(dataItem);
		if (fieldType != expectedFieldType) {
			return false;
		}

		if (fieldType == OBS_DATA_NUMBER && expectedNumberType != OBS_DATA_NUM_INVALID) {
			obs_data_number_type numberType = obs_data_item_numtype(dataItem);
			if (numberType != expectedNumberType) {
				return false;
			}
		}
	}

	return true;
}

bool RpcRequest::hasBool(QString fieldName)
{
	return this->hasField(fieldName, OBS_DATA_BOOLEAN);
}

bool RpcRequest::hasString(QString fieldName)
{
	return this->hasField(fieldName, OBS_DATA_STRING);
}

bool RpcRequest::hasNumber(QString fieldName, obs_data_number_type expectedNumberType)
{
	return this->hasField(fieldName, OBS_DATA_NUMBER, expectedNumberType);
}

bool RpcRequest::hasInteger(QString fieldName)
{
	return this->hasNumber(fieldName, OBS_DATA_NUM_INT);
}

bool RpcRequest::hasDouble(QString fieldName)
{
	return this->hasNumber(fieldName, OBS_DATA_NUM_DOUBLE);
}

bool RpcRequest::hasArray(QString fieldName)
{
	return this->hasField(fieldName, OBS_DATA_ARRAY);
}

bool RpcRequest::hasObject(QString fieldName)
{
	return this->hasField(fieldName, OBS_DATA_OBJECT);
}
