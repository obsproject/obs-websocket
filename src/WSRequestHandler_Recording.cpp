#include "WSRequestHandler.h"

#include <util/platform.h>
#include "Utils.h"

typedef void(*pauseRecordingFunction)(bool);
typedef bool(*recordingPausedFunction)();

HandlerResponse ifCanPause(WSRequestHandler* req, std::function<HandlerResponse(recordingPausedFunction, pauseRecordingFunction)> callback)
{
	void* frontendApi = os_dlopen("obs-frontend-api");

	bool (*recordingPaused)() = (bool(*)())os_dlsym(frontendApi, "obs_frontend_recording_paused");
	void (*pauseRecording)(bool) = (void(*)(bool))os_dlsym(frontendApi, "obs_frontend_recording_pause");

	if (!recordingPaused || !pauseRecording) {
		return req->SendErrorResponse("recording pause not supported");
	}

	if (!obs_frontend_recording_active()) {
		return req->SendErrorResponse("recording is not active");
	}

	return callback(recordingPaused, pauseRecording);
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
	return ifCanPause(req, [req](recordingPausedFunction recordingPaused, pauseRecordingFunction pauseRecording) {
		if (recordingPaused()) {
			return req->SendErrorResponse("recording already paused");
		}

		pauseRecording(true);
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
	return ifCanPause(req, [req](recordingPausedFunction recordingPaused, pauseRecordingFunction pauseRecording) {
		if (!recordingPaused()) {
			return req->SendErrorResponse("recording is not paused");
		}

		pauseRecording(false);
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
