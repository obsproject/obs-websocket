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
* @since 4.8.0
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

