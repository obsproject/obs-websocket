#include "WSRequestHandler.h"
#include <cmath>
#include <QtCore/QByteArray>
#include <QtGui/QImageWriter>
#include "obs-websocket.h"
#include "Config.h"
#include "Utils.h"
#include "WSEvents.h"

obs_data_t* videoInfoToData(obs_video_info* ovi)
{
	obs_data_t* data = obs_data_create();
	obs_data_set_int(data, "baseWidth", ovi->base_width);
	obs_data_set_int(data, "baseHeight", ovi->base_height);
	obs_data_set_int(data, "outputWidth", ovi->output_width);
	obs_data_set_int(data, "outputHeight", ovi->output_height);
	obs_data_set_double(data, "fps", (double)ovi->fps_num / ovi->fps_den);
	obs_data_set_string(data, "videoFormat", Utils::videoFormatToString(ovi->output_format));
	obs_data_set_string(data, "colorSpace", Utils::videoColorspaceToString(ovi->colorspace));
	obs_data_set_string(data, "colorRange", Utils::videoRangeTypeToString(ovi->range));
	obs_data_set_string(data, "scaleType", Utils::videoScaleTypeToString(ovi->scale_type));
	return data;
}

const char* videoErrorToString(int errorCode)
{
	switch (errorCode) {
		case OBS_VIDEO_SUCCESS:
			return "success";

		case OBS_VIDEO_NOT_SUPPORTED:
			return "requested settings are not supported by the graphics adapter";

		case OBS_VIDEO_INVALID_PARAM:
			return "invalid video parameter";

		case OBS_VIDEO_CURRENTLY_ACTIVE:
			return "one or more outputs are currently active";

		case OBS_VIDEO_MODULE_NOT_FOUND:
			return "graphics module not found";

		default:
			return "unknown error";
	}
}

/**
 * Returns the latest version of the plugin and the API.
 *
 * @return {double} `version` OBSRemote compatible API version. Fixed to 1.1 for retrocompatibility.
 * @return {String} `obs-websocket-version` obs-websocket plugin version.
 * @return {String} `obs-studio-version` OBS Studio program version.
 * @return {String} `available-requests` List of available request types, formatted as a comma-separated list string (e.g. : "Method1,Method2,Method3").
 * @return {String} `supported-image-export-formats` List of supported formats for features that use image export (like the TakeSourceScreenshot request type) formatted as a comma-separated list string
 *
 * @api requests
 * @name GetVersion
 * @category general
 * @since 0.3
 */
RpcResponse WSRequestHandler::GetVersion(const RpcRequest& request) {
	QString obsVersion = Utils::OBSVersionString();

	QList<QString> names = messageMap.keys();
	QList<QByteArray> imageWriterFormats = QImageWriter::supportedImageFormats();

	// (Palakis) OBS' data arrays only support object arrays, so I improvised.
	QString requests;
	names.sort(Qt::CaseInsensitive);
	requests += names.takeFirst();
	for (const QString& reqName : names) {
		requests += ("," + reqName);
	}

	QString supportedImageExportFormats;
	supportedImageExportFormats += QString::fromUtf8(imageWriterFormats.takeFirst());
	for (const QByteArray& format : imageWriterFormats) {
		supportedImageExportFormats += ("," + QString::fromUtf8(format));
	}

	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_double(data, "version", 1.1);
	obs_data_set_string(data, "obs-websocket-version", OBS_WEBSOCKET_VERSION);
	obs_data_set_string(data, "obs-studio-version", obsVersion.toUtf8());
	obs_data_set_string(data, "available-requests", requests.toUtf8());
	obs_data_set_string(data, "supported-image-export-formats", supportedImageExportFormats.toUtf8());

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
 * @return {String} `scaleType` Output scaling method used if output size differs from base size
 * @return {double} `fps` Frames rendered per second
 * @return {String} `videoFormat` Video color format
 * @return {String} `colorSpace` YUV color space
 * @return {String} `colorRange` YUV color range (full or partial)
 * 
 * @api requests
 * @name GetVideoInfo
 * @category general
 * @since 4.6.0 
 */
RpcResponse WSRequestHandler::GetVideoInfo(const RpcRequest& request) {
	obs_video_info ovi;
	obs_get_video_info(&ovi);

	OBSDataAutoRelease response = videoInfoToData(&ovi);
	return request.success(response);
}

/**
 * Change one or more OBS video settings
 * 
 * @param {int (Optional)} `baseWidth` Base (canvas) width
 * @param {int (Optional)} `baseHeight` Base (canvas) height
 * @param {int (Optional)} `outputWidth` Output width
 * @param {int (Optional)} `outputHeight` Output height
 * @param {String (Optional)} `scaleType` Output scaling method used if output size differs from base size
 * @param {double (Optional)} `fps` Frames rendered per second
 * @param {String (Optional)} `videoFormat` Video color format
 * @param {String (Optional)} `colorSpace` YUV color space
 * @param {String (Optional)} `colorRange` YUV color range (full or partial)
 * 
 * @return {int} `baseWidth` New base (canvas) width
 * @return {int} `baseHeight` New base (canvas) height
 * @return {int} `outputWidth` New output width
 * @return {int} `outputHeight` New output height
 * @return {String} `scaleType` New output scaling method
 * @return {double} `fps` New FPS
 * @return {String} `videoFormat` New video color format
 * @return {String} `colorSpace` New YUV color space
 * @return {String} `colorRange` New YUV color range (full or partial)
 *
 * @api requests
 * @name GetVideoInfo
 * @category general
 * @since unreleased 
 */
RpcResponse WSRequestHandler::SetVideoSettings(const RpcRequest& request) {
	const OBSData& params = request.parameters();
	OBSDataItemAutoRelease firstItem = obs_data_first(params);
	if (!firstItem) {
		return request.failed("at least one parameter is required when calling SetVideoSettings");
	}

	obs_video_info obsVideoInfo = {};
	obs_get_video_info(&obsVideoInfo);

	if (request.hasInteger("baseWidth")) {
		obsVideoInfo.base_width = obs_data_get_int(params, "baseWidth");
	}
	if (request.hasInteger("baseHeight")) {
		obsVideoInfo.base_height = obs_data_get_int(params, "baseHeight");
	}

	if (request.hasInteger("outputWidth")) {
		obsVideoInfo.output_width = obs_data_get_int(params, "outputWidth");
	}
	if (request.hasInteger("outputHeight")) {
		obsVideoInfo.output_height = obs_data_get_int(params, "outputHeight");
	}

	if (request.hasString("scaleType")) {
		// TODO
		// obsVideoInfo.scale_type;
	}

	if (request.hasDouble("fps")) {
		double fps = obs_data_get_double(params, "fps");
		obsVideoInfo.fps_num = (uint32_t)(std::ceil(fps) * 1000.0);
		obsVideoInfo.fps_den = (uint32_t)std::floor((double)obsVideoInfo.fps_num / fps);
	}

	if (request.hasString("videoFormat")) {
		// TODO
		// obsVideoInfo.output_format;
	}
	if (request.hasString("colorSpace")) {
		// TODO
		// obsVideoInfo.colorspace;
	}
	if (request.hasString("colorRange")) {
		// TODO
		// obsVideoInfo.range;
	}

	int result = obs_reset_video(&obsVideoInfo);
	if (result != OBS_VIDEO_SUCCESS) {
		return request.failed(videoErrorToString(result));
	}

	obs_get_video_info(&obsVideoInfo);
	OBSDataAutoRelease response = videoInfoToData(&obsVideoInfo);
	return request.success(response);
}

/**
 * Open a projector window or create a projector on a monitor. Requires OBS v24.0.4 or newer.
 * 
 * @param {String (Optional)} `type` Type of projector: Preview (default), Source, Scene, StudioProgram, or Multiview (case insensitive).
 * @param {int (Optional)} `monitor` Monitor to open the projector on. If -1 or omitted, opens a window.
 * @param {String (Optional)} `geometry` Size and position of the projector window (only if monitor is -1). Encoded in Base64 using Qt's geometry encoding (https://doc.qt.io/qt-5/qwidget.html#saveGeometry). Corresponds to OBS's saved projectors.
 * @param {String (Optional)} `name` Name of the source or scene to be displayed (ignored for other projector types).
 * 
 * @api requests
 * @name OpenProjector
 * @category general
 * @since unreleased
 */
RpcResponse WSRequestHandler::OpenProjector(const RpcRequest& request) {
	if (!Utils::OpenProjectorSupported()) {
		return request.failed("Projector opening requires OBS 24.0.4 or newer.");
	}

	const char* type = obs_data_get_string(request.parameters(), "type");

	int monitor = -1;
	if (request.hasField("monitor")) {
		monitor = obs_data_get_int(request.parameters(), "monitor");
	}

	const char* geometry = obs_data_get_string(request.parameters(), "geometry");
	const char* name = obs_data_get_string(request.parameters(), "name");

	Utils::OpenProjector(type, monitor, geometry, name);
	return request.success();
}
