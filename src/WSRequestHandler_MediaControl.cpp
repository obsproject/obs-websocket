#include "Utils.h"

#include "WSRequestHandler.h"

/**
* Pause or play a media source. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
*
* @param {String} `sourceName` Source name.
* @param {boolean} `playPause` Whether to pause or play the source. `false` for play, `true` for pause.
*
* @api requests
* @name PlayPauseMedia
* @category media control
* @since 4.9.0
*/
RpcResponse WSRequestHandler::PlayPauseMedia(const RpcRequest& request) {
	if ((!request.hasField("sourceName")) || (!request.hasField("playPause"))) {
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

	obs_source_media_play_pause(source, playPause);
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
	obs_data_set_int(response, "timeStamp", obs_source_media_get_duration(source));
	return request.success(response);
}

/**
* Get the current timestamp of media in milliseconds. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
*
* @param {String} `sourceName` Source name.
*
* @return {int} `timeStamp` The time in milliseconds since the start of the media.
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
	obs_data_set_int(response, "timeStamp", obs_source_media_get_time(source));
	return request.success(response);
}

/**
* Set the timestamp of a media source. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
*
* @param {String} `sourceName` Source name.
* @param {int} `timeStamp` Milliseconds to set the timestamp to.
*
* @api requests
* @name SetMediaTime
* @category media control
* @since 4.9.0
*/
RpcResponse WSRequestHandler::SetMediaTime(const RpcRequest& request) {
	if (!request.hasField("sourceName") || !request.hasField("timeStamp")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	int64_t timeStamp = (int64_t)obs_data_get_int(request.parameters(), "timeStamp");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	obs_source_media_set_time(source, timeStamp);
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
* @return {String} `mediaState` The media state of the provided source. States: `none`, `playing`, `opening`, `buffering`, `paused`, `stopped`, `ended`, `error`
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

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "mediaState", mediaState.toUtf8());

	return request.success(response);
}
