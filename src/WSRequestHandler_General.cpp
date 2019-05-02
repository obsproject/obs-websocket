#include "Config.h"
#include "Utils.h"
#include "WSEvents.h"

#include "WSRequestHandler.h"

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
HandlerResponse WSRequestHandler::HandleGetVersion(WSRequestHandler* req) {
	QString obsVersion = Utils::OBSVersionString();

	QList<QString> names = req->messageMap.keys();
	names.sort(Qt::CaseInsensitive);

	// (Palakis) OBS' data arrays only support object arrays, so I improvised.
	QString requests;
	requests += names.takeFirst();
	for (QString reqName : names) {
		requests += ("," + reqName);
	}

	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_string(data, "obs-websocket-version", OBS_WEBSOCKET_VERSION);
	obs_data_set_string(data, "obs-studio-version", obsVersion.toUtf8());
	obs_data_set_string(data, "available-requests", requests.toUtf8());

	return req->SendOKResponse(data);
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
HandlerResponse WSRequestHandler::HandleGetAuthRequired(WSRequestHandler* req) {
	bool authRequired = Config::Current()->AuthRequired;

	OBSDataAutoRelease data = obs_data_create();
	obs_data_set_bool(data, "authRequired", authRequired);

	if (authRequired) {
		obs_data_set_string(data, "challenge",
			Config::Current()->SessionChallenge.toUtf8());
		obs_data_set_string(data, "salt",
			Config::Current()->Salt.toUtf8());
	}

	return req->SendOKResponse(data);
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
HandlerResponse WSRequestHandler::HandleAuthenticate(WSRequestHandler* req) {
	if (!req->hasField("auth")) {
		return req->SendErrorResponse("missing request parameters");
	}

	if (req->_connProperties.isAuthenticated()) {
		return req->SendErrorResponse("already authenticated");
	}

	QString auth = obs_data_get_string(req->data, "auth");
	if (auth.isEmpty()) {
		return req->SendErrorResponse("auth not specified!");
	}

	if (Config::Current()->CheckAuth(auth) == false) {
		return req->SendErrorResponse("Authentication Failed.");
	}

	req->_connProperties.setAuthenticated(true);
	return req->SendOKResponse();
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
HandlerResponse WSRequestHandler::HandleSetHeartbeat(WSRequestHandler* req) {
	if (!req->hasField("enable")) {
		return req->SendErrorResponse("Heartbeat <enable> parameter missing");
	}

	auto events = WSEvents::Current();
	events->HeartbeatIsActive = obs_data_get_bool(req->data, "enable");

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_bool(response, "enable", events->HeartbeatIsActive);
	return req->SendOKResponse(response);
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
HandlerResponse WSRequestHandler::HandleSetFilenameFormatting(WSRequestHandler* req) {
	if (!req->hasField("filename-formatting")) {
		return req->SendErrorResponse("<filename-formatting> parameter missing");
	}

	QString filenameFormatting = obs_data_get_string(req->data, "filename-formatting");
	if (filenameFormatting.isEmpty()) {
		return req->SendErrorResponse("invalid request parameters");
	}

	Utils::SetFilenameFormatting(filenameFormatting.toUtf8());
	return req->SendOKResponse();
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
HandlerResponse WSRequestHandler::HandleGetFilenameFormatting(WSRequestHandler* req) {
	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "filename-formatting", Utils::GetFilenameFormatting());
	return req->SendOKResponse(response);
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
HandlerResponse WSRequestHandler::HandleGetStats(WSRequestHandler* req) {
	OBSDataAutoRelease stats = WSEvents::Current()->GetStats();

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_obj(response, "stats", stats);
	return req->SendOKResponse(response);
}