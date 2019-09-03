#include "WSRequestHandler.h"

#include <util/platform.h>
#include "Utils.h"

HandlerResponse ifCanPause(WSRequestHandler* req, std::function<HandlerResponse()> callback)
{
	if (!obs_frontend_recording_active()) {
		return req->SendErrorResponse("recording is not active");
	}

	if (!Utils::RecordingPauseSupported()) {
		return req->SendErrorResponse("recording pauses are not available in this version of OBS Studio");
	}

	return callback();
}

/**
 * Toggle recording on or off.
 *
 * @api requests
 * @name StartStopRecording
 * @category recording
 * @since 0.3
 */
HandlerResponse WSRequestHandler::HandleStartStopRecording(WSRequestHandler* req) {
	(obs_frontend_recording_active() ? obs_frontend_recording_stop() : obs_frontend_recording_start());
	return req->SendOKResponse();
}

/**
 * Start recording.
 * Will return an `error` if recording is already active.
 *
 * @api requests
 * @name StartRecording
 * @category recording
 * @since 4.1.0
 */
HandlerResponse WSRequestHandler::HandleStartRecording(WSRequestHandler* req) {
	if (obs_frontend_recording_active()) {
		return req->SendErrorResponse("recording already active");
	}

	obs_frontend_recording_start();
	return req->SendOKResponse();
}

/**
 * Stop recording.
 * Will return an `error` if recording is not active.
 *
 * @api requests
 * @name StopRecording
 * @category recording
 * @since 4.1.0
 */
 HandlerResponse WSRequestHandler::HandleStopRecording(WSRequestHandler* req) {
	if (!obs_frontend_recording_active()) {
		return req->SendErrorResponse("recording not active");
	}

	obs_frontend_recording_stop();
	return req->SendOKResponse();
}

/**
* Pause the current recording.
* Returns an error if recording is not active or already paused.
*
* @api requests
* @name PauseRecording
* @category recording
* @since 4.7.0
*/
HandlerResponse WSRequestHandler::HandlePauseRecording(WSRequestHandler* req) {
	return ifCanPause(req, [req]() {
		if (Utils::RecordingPaused()) {
			return req->SendErrorResponse("recording already paused");
		}

		Utils::PauseRecording(true);
		return req->SendOKResponse();
	});
}

/**
* Resume/unpause the current recording (if paused).
* Returns an error if recording is not active or not paused.
*
* @api requests
* @name ResumeRecording
* @category recording
* @since 4.7.0
*/
HandlerResponse WSRequestHandler::HandleResumeRecording(WSRequestHandler* req) {
	return ifCanPause(req, [req]() {
		if (!Utils::RecordingPaused()) {
			return req->SendErrorResponse("recording is not paused");
		}

		Utils::PauseRecording(false);
		return req->SendOKResponse();
	});
}

/**
 * In the current profile, sets the recording folder of the Simple and Advanced
 * output modes to the specified value.
 * 
 * Please note: if `SetRecordingFolder` is called while a recording is
 * in progress, the change won't be applied immediately and will be
 * effective on the next recording.
 * 
 * @param {String} `rec-folder` Path of the recording folder.
 *
 * @api requests
 * @name SetRecordingFolder
 * @category recording
 * @since 4.1.0
 */
HandlerResponse WSRequestHandler::HandleSetRecordingFolder(WSRequestHandler* req) {
	if (!req->hasField("rec-folder")) {
		return req->SendErrorResponse("missing request parameters");
	}

	const char* newRecFolder = obs_data_get_string(req->data, "rec-folder");
	bool success = Utils::SetRecordingFolder(newRecFolder);
	if (!success) {
		return req->SendErrorResponse("invalid request parameters");
	}

	return req->SendOKResponse();
}

/**
 * Get the path of  the current recording folder.
 *
 * @return {String} `rec-folder` Path of the recording folder.
 *
 * @api requests
 * @name GetRecordingFolder
 * @category recording
 * @since 4.1.0
 */
HandlerResponse WSRequestHandler::HandleGetRecordingFolder(WSRequestHandler* req) {
	const char* recFolder = Utils::GetRecordingFolder();

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "rec-folder", recFolder);

	return req->SendOKResponse(response);
}
