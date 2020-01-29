#include "obs-websocket.h"
#include "Config.h"
#include "Utils.h"
#include "WSEvents.h"

#include "WSRequestHandler.h"

#define CASE(x) case x: return #x;
const char *describe_output_format(int format) {
	switch (format) {
		default:
		CASE(VIDEO_FORMAT_NONE)
		CASE(VIDEO_FORMAT_I420)
		CASE(VIDEO_FORMAT_NV12)
		CASE(VIDEO_FORMAT_YVYU)
		CASE(VIDEO_FORMAT_YUY2)
		CASE(VIDEO_FORMAT_UYVY)
		CASE(VIDEO_FORMAT_RGBA)
		CASE(VIDEO_FORMAT_BGRA)
		CASE(VIDEO_FORMAT_BGRX)
		CASE(VIDEO_FORMAT_Y800)
		CASE(VIDEO_FORMAT_I444)
	}
}

const char *describe_color_space(int cs) {
	switch (cs) {
		default:
		CASE(VIDEO_CS_DEFAULT)
		CASE(VIDEO_CS_601)
		CASE(VIDEO_CS_709)
	}
}

const char *describe_color_range(int range) {
	switch (range) {
		default:
		CASE(VIDEO_RANGE_DEFAULT)
		CASE(VIDEO_RANGE_PARTIAL)
		CASE(VIDEO_RANGE_FULL)
	}
}

const char *describe_scale_type(int scale) {
	switch (scale) {
		default:
		CASE(VIDEO_SCALE_DEFAULT)
		CASE(VIDEO_SCALE_POINT)
		CASE(VIDEO_SCALE_FAST_BILINEAR)
		CASE(VIDEO_SCALE_BILINEAR)
		CASE(VIDEO_SCALE_BICUBIC)
	}
}
#undef CASE

/**
 * Returns the latest version of the plugin and the API.
 *
 * @return {double} `version` OBSRemote compatible API version. Fixed to 1.1 for retrocompatibility.
 * @return {String} `obs-websocket-version` obs-websocket plugin version.
 * @return {String} `obs-studio-version` OBS Studio program version.
 * @return {String} `available-requests` List of available request types, formatted as a comma-separated list string (e.g. : "Method1,Method2,Method3").
 *
 * @api requests
 * @name GetVersion
 * @category general
 * @since 0.3
 */
RpcResponse WSRequestHandler::GetVersion(const RpcRequest& request) {
	QString obsVersion = Utils::OBSVersionString();

	QList<QString> names = messageMap.keys();
	names.sort(Qt::CaseInsensitive);

	// (Palakis) OBS' data arrays only support object arrays, so I improvised.
	QString requests;
	requests += names.takeFirst();
	for (QString reqName : names) {
		requests += ("," + reqName);
	}

	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_double(data, "version", 1.1);
	obs_data_set_string(data, "obs-websocket-version", OBS_WEBSOCKET_VERSION);
	obs_data_set_string(data, "obs-studio-version", obsVersion.toUtf8());
	obs_data_set_string(data, "available-requests", requests.toUtf8());

	return request.success(data);
}

/**
 * Tells the client if authentication is required. If so, returns authentication parameters `challenge`
 * and `salt` (see "Authentication" for more information).
 *
 * @return {boolean} `authRequired` Indicates whether authentication is required.
 * @return {String (optional)} `challenge`
 * @return {String (optional)} `salt`
 *
 * @api requests
 * @name GetAuthRequired
 * @category general
 * @since 0.3
 */
RpcResponse WSRequestHandler::GetAuthRequired(const RpcRequest& request) {
	bool authRequired = GetConfig()->AuthRequired;

	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_bool(data, "authRequired", authRequired);

	if (authRequired) {
		auto config = GetConfig();
		obs_data_set_string(data, "challenge",
			config->SessionChallenge.toUtf8());
		obs_data_set_string(data, "salt",
			config->Salt.toUtf8());
	}

	return request.success(data);
}

/**
 * Attempt to authenticate the client to the server.
 *
 * @param {String} `auth` Response to the auth challenge (see "Authentication" for more information).
 *
 * @api requests
 * @name Authenticate
 * @category general
 * @since 0.3
 */
RpcResponse WSRequestHandler::Authenticate(const RpcRequest& request) {
	if (!request.hasField("auth")) {
		return request.failed("missing request parameters");
	}

	if (_connProperties.isAuthenticated()) {
		return request.failed("already authenticated");
	}

	QString auth = obs_data_get_string(request.parameters(), "auth");
	if (auth.isEmpty()) {
		return request.failed("auth not specified!");
	}

	if (GetConfig()->CheckAuth(auth) == false) {
		return request.failed("Authentication Failed.");
	}

	_connProperties.setAuthenticated(true);
	return request.success();
}

/**
 * Enable/disable sending of the Heartbeat event
 *
 * @param {boolean} `enable` Starts/Stops emitting heartbeat messages
 *
 * @api requests
 * @name SetHeartbeat
 * @category general
 * @since 4.3.0
 */
RpcResponse WSRequestHandler::SetHeartbeat(const RpcRequest& request) {
	if (!request.hasField("enable")) {
		return request.failed("Heartbeat <enable> parameter missing");
	}

	auto events = GetEventsSystem();
	events->HeartbeatIsActive = obs_data_get_bool(request.parameters(), "enable");

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_bool(response, "enable", events->HeartbeatIsActive);

	return request.success(response);
}

/**
 * Set the filename formatting string
 *
 * @param {String} `filename-formatting` Filename formatting string to set.
 *
 * @api requests
 * @name SetFilenameFormatting
 * @category general
 * @since 4.3.0
 */
RpcResponse WSRequestHandler::SetFilenameFormatting(const RpcRequest& request) {
	if (!request.hasField("filename-formatting")) {
		return request.failed("<filename-formatting> parameter missing");
	}

	QString filenameFormatting = obs_data_get_string(request.parameters(), "filename-formatting");
	if (filenameFormatting.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	Utils::SetFilenameFormatting(filenameFormatting.toUtf8());

	return request.success();
}

/**
 * Get the filename formatting string
 *
 * @return {String} `filename-formatting` Current filename formatting string.
 *
 * @api requests
 * @name GetFilenameFormatting
 * @category general
 * @since 4.3.0
 */
RpcResponse WSRequestHandler::GetFilenameFormatting(const RpcRequest& request) {
	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "filename-formatting", Utils::GetFilenameFormatting());

	return request.success(response);
}

/**
 * Get OBS stats (almost the same info as provided in OBS' stats window)
 *
 * @return {OBSStats} `stats` OBS stats
 *
 * @api requests
 * @name GetStats
 * @category general
 * @since 4.6.0
 */
RpcResponse WSRequestHandler::GetStats(const RpcRequest& request) {
	OBSDataAutoRelease stats = GetEventsSystem()->GetStats();

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_obj(response, "stats", stats);

	return request.success(response);
}

/**
 * Broadcast custom message to all connected WebSocket clients
 *
 * @param {String} `realm` Identifier to be choosen by the client
 * @param {Object} `data` User-defined data
 *
 * @api requests
 * @name BroadcastCustomMessage
 * @category general
 * @since 4.7.0
 */
RpcResponse WSRequestHandler::BroadcastCustomMessage(const RpcRequest& request) {
	if (!request.hasField("realm") || !request.hasField("data")) {
		return request.failed("missing request parameters");
	}

	QString realm = obs_data_get_string(request.parameters(), "realm");
	OBSDataAutoRelease data = obs_data_get_obj(request.parameters(), "data");

	if (realm.isEmpty()) {
		return request.failed("realm not specified!");
	}

	if (!data) {
		return request.failed("data not specified!");
	}

	auto events = GetEventsSystem();
	events->OnBroadcastCustomMessage(realm, data);

	return request.success();
}


/**
 * Get basic OBS video information
 * 
 * @return {int} `baseWidth` Base (canvas) width
 * @return {int} `baseHeight` Base (canvas) height
 * @return {int} `outputWidth` Output width
 * @return {int} `outputHeight` Output height
 * @return {String} `scaleType` Scaling method used if output size differs from base size
 * @return {double} `fps` Frames rendered per second
 * @return {String} `videoFormat` Video color format
 * @return {String} `colorSpace` Color space for YUV
 * @return {String} `colorRange` Color range (full or partial)
 * 
 * @api requests
 * @name GetVideoInfo
 * @category general
 * @since 4.6.0 
 */
RpcResponse WSRequestHandler::GetVideoInfo(const RpcRequest& request) {
	obs_video_info ovi;
	obs_get_video_info(&ovi);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_int(response, "baseWidth", ovi.base_width);
	obs_data_set_int(response, "baseHeight", ovi.base_height);
	obs_data_set_int(response, "outputWidth", ovi.output_width);
	obs_data_set_int(response, "outputHeight", ovi.output_height);
	obs_data_set_double(response, "fps", (double)ovi.fps_num / ovi.fps_den);
	obs_data_set_string(response, "videoFormat", describe_output_format(ovi.output_format));
	obs_data_set_string(response, "colorSpace", describe_color_space(ovi.colorspace));
	obs_data_set_string(response, "colorRange", describe_color_range(ovi.range));
	obs_data_set_string(response, "scaleType", describe_scale_type(ovi.scale_type));

	return request.success(response);
}
