/*
obs-websocket
Copyright (C) 2016-2020	Stéphane Lepin <stephane.lepin@gmail.com>

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

#include "RpcEvent.h"

RpcEvent::RpcEvent(const QString& updateType, uint64_t streamTime, uint64_t recordingTime, obs_data_t* fields) :
	_updateType(updateType),
	_streamTime(streamTime),
	_recordingTime(recordingTime),
	_fields(fields)
{
}

const QString& RpcEvent::updateType() const
{
	return _updateType;
}

const uint64_t RpcEvent::streamTime() const
{
	return _streamTime;
}

const uint64_t RpcEvent::recordingTime() const
{
	return _recordingTime;
}

const OBSData RpcEvent::fields() const
{
	return OBSData(_fields);
}