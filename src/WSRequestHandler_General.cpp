#include <QString>

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
 void WSRequestHandler::HandleGetVersion(WSRequestHandler* req) {
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

	req->SendOKResponse(data);
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
 void WSRequestHandler::HandleSetHeartbeat(WSRequestHandler* req) {
	if (!req->hasField("enable")) {
		req->SendErrorResponse("Heartbeat <enable> parameter missing");
		return;
	}

	WSEvents::Instance->HeartbeatIsActive =
		obs_data_get_bool(req->data, "enable");

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_bool(response, "enable",
		WSEvents::Instance->HeartbeatIsActive);
	req->SendOKResponse(response);
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
void WSRequestHandler::HandleSetFilenameFormatting(WSRequestHandler* req) {
	if (!req->hasField("filename-formatting")) {
		req->SendErrorResponse("<filename-formatting> parameter missing");
		return;
	}

	QString filenameFormatting = obs_data_get_string(req->data, "filename-formatting");
	if (!filenameFormatting.isEmpty()) {
		Utils::SetFilenameFormatting(filenameFormatting.toUtf8());
		req->SendOKResponse();
	} else {
		req->SendErrorResponse("invalid request parameters");
	}
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
void WSRequestHandler::HandleGetFilenameFormatting(WSRequestHandler* req) {
	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "filename-formatting", Utils::GetFilenameFormatting());
	req->SendOKResponse(response);
}
