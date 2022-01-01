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

#include <inttypes.h>
#include <QString>

#include "Obs.h"
#include "../plugin-macros.generated.h"

#define CASE(x) case x: return #x;

std::string Utils::Obs::StringHelper::GetObsVersion()
{
	uint32_t version = obs_get_version();

	uint8_t major, minor, patch;
	major = (version >> 24) & 0xFF;
	minor = (version >> 16) & 0xFF;
	patch = version & 0xFF;

	QString combined = QString("%1.%2.%3").arg(major).arg(minor).arg(patch);
	return combined.toStdString();
}

std::string Utils::Obs::StringHelper::GetCurrentSceneCollection()
{
	char *sceneCollectionName = obs_frontend_get_current_scene_collection();
	std::string ret = sceneCollectionName;
	bfree(sceneCollectionName);
	return ret;
}

std::string Utils::Obs::StringHelper::GetCurrentProfile()
{
	char *profileName = obs_frontend_get_current_profile();
	std::string ret = profileName;
	bfree(profileName);
	return ret;
}

std::string Utils::Obs::StringHelper::GetCurrentProfilePath()
{
	char *profilePath = obs_frontend_get_current_profile_path();
	std::string ret = profilePath;
	bfree(profilePath);
	return ret;
}

std::string Utils::Obs::StringHelper::GetCurrentRecordOutputPath()
{
	char *recordOutputPath = obs_frontend_get_current_record_output_path();
	std::string ret = recordOutputPath;
	bfree(recordOutputPath);
	return ret;
}

std::string Utils::Obs::StringHelper::GetSourceType(obs_source_t *source)
{
	obs_source_type sourceType = obs_source_get_type(source);

	switch (sourceType) {
		default:
		CASE(OBS_SOURCE_TYPE_INPUT)
		CASE(OBS_SOURCE_TYPE_FILTER)
		CASE(OBS_SOURCE_TYPE_TRANSITION)
		CASE(OBS_SOURCE_TYPE_SCENE)
	}
}

std::string Utils::Obs::StringHelper::GetInputMonitorType(enum obs_monitoring_type monitorType)
{
	switch (monitorType) {
		default:
		CASE(OBS_MONITORING_TYPE_NONE)
		CASE(OBS_MONITORING_TYPE_MONITOR_ONLY)
		CASE(OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT)
	}
}

std::string Utils::Obs::StringHelper::GetInputMonitorType(obs_source_t *input)
{
	obs_monitoring_type monitorType = obs_source_get_monitoring_type(input);

	return GetInputMonitorType(monitorType);
}

std::string Utils::Obs::StringHelper::GetMediaInputState(obs_source_t *input)
{
	obs_media_state mediaState = obs_source_media_get_state(input);

	switch (mediaState) {
		default:
		CASE(OBS_MEDIA_STATE_NONE)
		CASE(OBS_MEDIA_STATE_PLAYING)
		CASE(OBS_MEDIA_STATE_OPENING)
		CASE(OBS_MEDIA_STATE_BUFFERING)
		CASE(OBS_MEDIA_STATE_PAUSED)
		CASE(OBS_MEDIA_STATE_STOPPED)
		CASE(OBS_MEDIA_STATE_ENDED)
		CASE(OBS_MEDIA_STATE_ERROR)
	}
}

std::string Utils::Obs::StringHelper::GetLastReplayBufferFilePath()
{
	OBSOutputAutoRelease output = obs_frontend_get_replay_buffer_output();
	calldata_t cd = {0};
	proc_handler_t *ph = obs_output_get_proc_handler(output);
	proc_handler_call(ph, "get_last_replay", &cd);
	auto ret = calldata_string(&cd, "path");
	calldata_free(&cd);
	return ret;
}

std::string Utils::Obs::StringHelper::GetSceneItemBoundsType(enum obs_bounds_type type)
{
	switch (type) {
		default:
		CASE(OBS_BOUNDS_NONE)
		CASE(OBS_BOUNDS_STRETCH)
		CASE(OBS_BOUNDS_SCALE_INNER)
		CASE(OBS_BOUNDS_SCALE_OUTER)
		CASE(OBS_BOUNDS_SCALE_TO_WIDTH)
		CASE(OBS_BOUNDS_SCALE_TO_HEIGHT)
		CASE(OBS_BOUNDS_MAX_ONLY)
	}
}

std::string Utils::Obs::StringHelper::GetSceneItemBlendMode(enum obs_blending_type mode)
{
	switch (mode) {
		default:
		CASE(OBS_BLEND_NORMAL)
		CASE(OBS_BLEND_ADDITIVE)
		CASE(OBS_BLEND_SUBTRACT)
		CASE(OBS_BLEND_SCREEN)
		CASE(OBS_BLEND_MULTIPLY)
		CASE(OBS_BLEND_LIGHTEN)
		CASE(OBS_BLEND_DARKEN)
	}
}

std::string Utils::Obs::StringHelper::DurationToTimecode(uint64_t ms)
{
	uint64_t secs = ms / 1000ULL;
	uint64_t minutes = secs / 60ULL;

	uint64_t hoursPart = minutes / 60ULL;
	uint64_t minutesPart = minutes % 60ULL;
	uint64_t secsPart = secs % 60ULL;
	uint64_t msPart = ms % 1000ULL;

	QString formatted = QString::asprintf("%02" PRIu64 ":%02" PRIu64 ":%02" PRIu64 ".%03" PRIu64, hoursPart, minutesPart, secsPart, msPart);
	return formatted.toStdString();
}

std::string Utils::Obs::StringHelper::GetOutputState(ObsOutputState state)
{
	switch (state) {
		default:
		CASE(OBS_WEBSOCKET_OUTPUT_UNKNOWN)
		CASE(OBS_WEBSOCKET_OUTPUT_STARTING)
		CASE(OBS_WEBSOCKET_OUTPUT_STARTED)
		CASE(OBS_WEBSOCKET_OUTPUT_STOPPING)
		CASE(OBS_WEBSOCKET_OUTPUT_STOPPED)
		CASE(OBS_WEBSOCKET_OUTPUT_PAUSED)
		CASE(OBS_WEBSOCKET_OUTPUT_RESUMED)
	}
}
