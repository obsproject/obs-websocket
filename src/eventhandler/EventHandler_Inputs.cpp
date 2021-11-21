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

#include "EventHandler.h"

void EventHandler::HandleInputCreated(obs_source_t *source)
{
	std::string inputKind = obs_source_get_id(source);
	OBSDataAutoRelease inputSettings = obs_source_get_settings(source);
	OBSDataAutoRelease defaultInputSettings = obs_get_source_defaults(inputKind.c_str());

	json eventData;
	eventData["inputName"] = obs_source_get_name(source);
	eventData["inputKind"] = inputKind;
	eventData["unversionedInputKind"] = obs_source_get_unversioned_id(source);
	eventData["inputSettings"] = Utils::Json::ObsDataToJson(inputSettings);
	eventData["defaultInputSettings"] = Utils::Json::ObsDataToJson(defaultInputSettings, true);
	BroadcastEvent(EventSubscription::Inputs, "InputCreated", eventData);
}

void EventHandler::HandleInputRemoved(obs_source_t *source)
{
	json eventData;
	eventData["inputName"] = obs_source_get_name(source);
	BroadcastEvent(EventSubscription::Inputs, "InputRemoved", eventData);
}

void EventHandler::HandleInputNameChanged(obs_source_t *, std::string oldInputName, std::string inputName)
{
	json eventData;
	eventData["oldInputName"] = oldInputName;
	eventData["inputName"] = inputName;
	BroadcastEvent(EventSubscription::Inputs, "InputNameChanged", eventData);
}

void EventHandler::HandleInputActiveStateChanged(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	if (!eventHandler->_inputActiveStateChangedRef.load())
		return;

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	json eventData;
	eventData["inputName"] = obs_source_get_name(source);
	eventData["videoActive"] = obs_source_active(source);
	eventHandler->BroadcastEvent(EventSubscription::InputActiveStateChanged, "InputActiveStateChanged", eventData);
}

void EventHandler::HandleInputShowStateChanged(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	if (!eventHandler->_inputShowStateChangedRef.load())
		return;

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	json eventData;
	eventData["inputName"] = obs_source_get_name(source);
	eventData["videoShowing"] = obs_source_showing(source);
	eventHandler->BroadcastEvent(EventSubscription::InputShowStateChanged, "InputShowStateChanged", eventData);
}

void EventHandler::HandleInputMuteStateChanged(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	json eventData;
	eventData["inputName"] = obs_source_get_name(source);
	eventData["inputMuted"] = obs_source_muted(source);
	eventHandler->BroadcastEvent(EventSubscription::Inputs, "InputMuteStateChanged", eventData);
}

void EventHandler::HandleInputVolumeChanged(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	// Volume must be grabbed from the calldata. Running obs_source_get_volume() will return the previous value.
	double inputVolumeMul = calldata_float(data, "volume");

	double inputVolumeDb = obs_mul_to_db(inputVolumeMul);
	if (inputVolumeDb == -INFINITY)
		inputVolumeDb = -100;

	json eventData;
	eventData["inputName"] = obs_source_get_name(source);
	eventData["inputVolumeMul"] = inputVolumeMul;
	eventData["inputVolumeDb"] = inputVolumeDb;
	eventHandler->BroadcastEvent(EventSubscription::Inputs, "InputVolumeChanged", eventData);
}

void EventHandler::HandleInputAudioSyncOffsetChanged(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	long long inputAudioSyncOffset = calldata_int(data, "offset");

	json eventData;
	eventData["inputName"] = obs_source_get_name(source);
	eventData["inputAudioSyncOffset"] = inputAudioSyncOffset / 1000000;
	eventHandler->BroadcastEvent(EventSubscription::Inputs, "InputAudioSyncOffsetChanged", eventData);
}

void EventHandler::HandleInputAudioTracksChanged(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	long long tracks = calldata_int(data, "mixers");

	json inputAudioTracks;
	for (long long i = 0; i < MAX_AUDIO_MIXES; i++) {
		inputAudioTracks[std::to_string(i + 1)] = (bool)((tracks >> i) & 1);
	}

	json eventData;
	eventData["inputName"] = obs_source_get_name(source);
	eventData["inputAudioTracks"] = inputAudioTracks;
	eventHandler->BroadcastEvent(EventSubscription::Inputs, "InputAudioTracksChanged", eventData);
}

void EventHandler::HandleInputAudioMonitorTypeChanged(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	enum obs_monitoring_type monitorType = (obs_monitoring_type)calldata_int(data, "type");

	std::string monitorTypeString;
	switch (monitorType) {
		default:
		case OBS_MONITORING_TYPE_NONE:
			monitorTypeString = "OBS_WEBSOCKET_MONITOR_TYPE_NONE";
			break;
		case OBS_MONITORING_TYPE_MONITOR_ONLY:
			monitorTypeString = "OBS_WEBSOCKET_MONITOR_TYPE_MONITOR_ONLY";
			break;
		case OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT:
			monitorTypeString = "OBS_WEBSOCKET_MONITOR_TYPE_MONITOR_AND_OUTPUT";
			break;
	}

	json eventData;
	eventData["inputName"] = obs_source_get_name(source);
	eventData["monitorType"] = monitorTypeString;
	eventHandler->BroadcastEvent(EventSubscription::Inputs, "InputAudioMonitorTypeChanged", eventData);
}
