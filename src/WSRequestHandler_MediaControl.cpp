#include "Utils.h"

#include "WSRequestHandler.h"

bool isMediaSource(const QString& sourceKind)
{
	return (sourceKind == "vlc_source" || sourceKind == "ffmpeg_source");
}

QString getSourceMediaState(obs_source_t *source)
{
	QString mediaState;
	enum obs_media_state mstate = obs_source_media_get_state(source);
	switch (mstate) {
	case OBS_MEDIA_STATE_NONE:
		mediaState = "none";
		break;
	case OBS_MEDIA_STATE_PLAYING:
		mediaState = "playing";
		break;
	case OBS_MEDIA_STATE_OPENING:
		mediaState = "opening";
		break;
	case OBS_MEDIA_STATE_BUFFERING:
		mediaState = "buffering";
		break;
	case OBS_MEDIA_STATE_PAUSED:
		mediaState = "paused";
		break;
	case OBS_MEDIA_STATE_STOPPED:
		mediaState = "stopped";
		break;
	case OBS_MEDIA_STATE_ENDED:
		mediaState = "ended";
		break;
	case OBS_MEDIA_STATE_ERROR:
		mediaState = "error";
		break;
	default:
		mediaState = "unknown";
	}
	return mediaState;
}

/**
* Pause or play a media source. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
* Note :Leaving out `playPause` toggles the current pause state
*
* @param {String} `sourceName` Source name.
* @param {boolean} `playPause` (optional) Whether to pause or play the source. `false` for play, `true` for pause.
*
* @api requests
* @name PlayPauseMedia
* @category media control
* @since 4.9.0
*/
RpcResponse WSRequestHandler::PlayPauseMedia(const RpcRequest& request) {
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	bool playPause = obs_data_get_bool(request.parameters(), "playPause");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}
	if (!request.hasField("playPause")) {
		if (obs_source_media_get_state(source) == obs_media_state::OBS_MEDIA_STATE_PLAYING) {
			obs_source_media_play_pause(source, true);
		} else {
			obs_source_media_play_pause(source, false);
		}
	} else {
		bool playPause = obs_data_get_bool(request.parameters(), "playPause");
		obs_source_media_play_pause(source, playPause);
	}
	return request.success();
}

/**
* Restart a media source. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
*
* @param {String} `sourceName` Source name.
*
* @api requests
* @name RestartMedia
* @category media control
* @since 4.9.0
*/
RpcResponse WSRequestHandler::RestartMedia(const RpcRequest& request) {
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	obs_source_media_restart(source);
	return request.success();
}

/**
* Stop a media source. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
*
* @param {String} `sourceName` Source name.
*
* @api requests
* @name StopMedia
* @category media control
* @since 4.9.0
*/
RpcResponse WSRequestHandler::StopMedia(const RpcRequest& request) {
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	obs_source_media_stop(source);
	return request.success();
}

/**
* Skip to the next media item in the playlist. Supports only vlc media source (as of OBS v25.0.8)
*
* @param {String} `sourceName` Source name.
*
* @api requests
* @name NextMedia
* @category media control
* @since 4.9.0
*/
RpcResponse WSRequestHandler::NextMedia(const RpcRequest& request) {
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	obs_source_media_next(source);
	return request.success();
}

/**
* Go to the previous media item in the playlist. Supports only vlc media source (as of OBS v25.0.8)
*
* @param {String} `sourceName` Source name.
*
* @api requests
* @name PreviousMedia
* @category media control
* @since 4.9.0
*/
RpcResponse WSRequestHandler::PreviousMedia(const RpcRequest& request) {
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	obs_source_media_previous(source);
	return request.success();
}

/**
* Get the length of media in milliseconds. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
* Note: For some reason, for the first 5 or so seconds that the media is playing, the total duration can be off by upwards of 50ms.
*
* @param {String} `sourceName` Source name.
*
* @return {int} `mediaDuration` The total length of media in milliseconds..
*
* @api requests
* @name GetMediaDuration
* @category media control
* @since 4.9.0
*/
RpcResponse WSRequestHandler::GetMediaDuration(const RpcRequest& request) {
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_int(response, "mediaDuration", obs_source_media_get_duration(source));
	return request.success(response);
}

/**
* Get the current timestamp of media in milliseconds. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
*
* @param {String} `sourceName` Source name.
*
* @return {int} `timestamp` The time in milliseconds since the start of the media.
*
* @api requests
* @name GetMediaTime
* @category media control
* @since 4.9.0
*/
RpcResponse WSRequestHandler::GetMediaTime(const RpcRequest& request) {
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_int(response, "timestamp", obs_source_media_get_time(source));
	return request.success(response);
}

/**
* Set the timestamp of a media source. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
*
* @param {String} `sourceName` Source name.
* @param {int} `timestamp` Milliseconds to set the timestamp to.
*
* @api requests
* @name SetMediaTime
* @category media control
* @since 4.9.0
*/
RpcResponse WSRequestHandler::SetMediaTime(const RpcRequest& request) {
	if (!request.hasField("sourceName") || !request.hasField("timestamp")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	int64_t timestamp = (int64_t)obs_data_get_int(request.parameters(), "timestamp");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	obs_source_media_set_time(source, timestamp);
	return request.success();
}

/**
* Scrub media using a supplied offset. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
* Note: Due to processing/network delays, this request is not perfect. The processing rate of this request has also not been tested.
*
* @param {String} `sourceName` Source name.
* @param {int} `timeOffset` Millisecond offset (positive or negative) to offset the current media position.
*
* @api requests
* @name ScrubMedia
* @category media control
* @since 4.9.0
*/
RpcResponse WSRequestHandler::ScrubMedia(const RpcRequest& request) {
	if (!request.hasField("sourceName") || !request.hasField("timeOffset")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	int64_t timeOffset = (int64_t)obs_data_get_int(request.parameters(), "timeOffset");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	int64_t newTime = obs_source_media_get_time(source) + timeOffset;
	if (newTime < 0) {
		newTime = 0;
	}

	obs_source_media_set_time(source, newTime);
	return request.success();
}

/**
* Get the current playing state of a media source. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
*
* @param {String} `sourceName` Source name.
*
* @return {String} `mediaState` The media state of the provided source. States: `none`, `playing`, `opening`, `buffering`, `paused`, `stopped`, `ended`, `error`, `unknown`
*
* @api requests
* @name GetMediaState
* @category media control
* @since 4.9.0
*/
RpcResponse WSRequestHandler::GetMediaState(const RpcRequest& request) {
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "mediaState", getSourceMediaState(source).toUtf8());

	return request.success(response);
}

/**
* List the media state of all media sources (vlc and media source)
*
* @return {Array<Object>} `mediaSources` Array of sources
* @return {String} `mediaSources.*.sourceName` Unique source name
* @return {String} `mediaSources.*.sourceKind` Unique source internal type (a.k.a `ffmpeg_source` or `vlc_source`)
* @return {String} `mediaSources.*.mediaState` The current state of media for that source. States: `none`, `playing`, `opening`, `buffering`, `paused`, `stopped`, `ended`, `error`, `unknown`
*
* @api requests
* @name GetMediaSourcesList
* @category sources
* @since 4.9.0
*/
RpcResponse WSRequestHandler::GetMediaSourcesList(const RpcRequest& request)
{
	OBSDataArrayAutoRelease sourcesArray = obs_data_array_create();

	auto sourceEnumProc = [](void* privateData, obs_source_t* source) -> bool {
		obs_data_array_t* sourcesArray = (obs_data_array_t*)privateData;

		QString sourceKind = obs_source_get_id(source);
		if (isMediaSource(sourceKind)) {
			OBSDataAutoRelease sourceData = obs_data_create();
			obs_data_set_string(sourceData, "sourceName", obs_source_get_name(source));
			obs_data_set_string(sourceData, "sourceKind", sourceKind.toUtf8());

			QString mediaState = getSourceMediaState(source);
			obs_data_set_string(sourceData, "mediaState", mediaState.toUtf8());

			obs_data_array_push_back(sourcesArray, sourceData);
		}
		return true;
	};
	obs_enum_sources(sourceEnumProc, sourcesArray);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_array(response, "mediaSources", sourcesArray);
	return request.success(response);
}
