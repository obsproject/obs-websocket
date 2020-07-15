#include "obs-websocket.h"
#include "WSRequestHandler.h"

#include <functional>
#include <util/platform.h>
#include "Utils.h"
#include "WSEvents.h"

RpcResponse ifCanPause(const RpcRequest& request, std::function<RpcResponse()> callback)
{
	if (!obs_frontend_recording_active()) {
		return request.failed("recording is not active");
	}

	return callback();
}

 /**
 * Get current recording status.
 *
 * @return {boolean} `isRecording` Current recording status.
 * @return {boolean} `isRecordingPaused` Whether the recording is paused or not.
 * @return {String (optional)} `recordTimecode` Time elapsed since recording started (only present if currently recording).
 *
 * @api requests
 * @name GetRecordingStatus
 * @category recording
 * @since unreleased
 */
RpcResponse WSRequestHandler::GetRecordingStatus(const RpcRequest& request) {
        auto events = GetEventsSystem();

        OBSDataAutoRelease data = obs_data_create();
        obs_data_set_bool(data, "isRecording", obs_frontend_recording_active());
        obs_data_set_bool(data, "isRecordingPaused", obs_frontend_recording_paused());

        if (obs_frontend_recording_active()) {
                QString recordingTimecode = events->getRecordingTimecode();
                obs_data_set_string(data, "recordTimecode", recordingTimecode.toUtf8().constData());
        }

        return request.success(data);
}

/**
 * Toggle recording on or off (depending on the current recording state).
 *
 * @api requests
 * @name StartStopRecording
 * @category recording
 * @since 0.3
 */
RpcResponse WSRequestHandler::StartStopRecording(const RpcRequest& request) {
	(obs_frontend_recording_active() ? obs_frontend_recording_stop() : obs_frontend_recording_start());
	return request.success();
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
RpcResponse WSRequestHandler::StartRecording(const RpcRequest& request) {
	if (obs_frontend_recording_active()) {
		return request.failed("recording already active");
	}

	obs_frontend_recording_start();
	return request.success();
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
 RpcResponse WSRequestHandler::StopRecording(const RpcRequest& request) {
	if (!obs_frontend_recording_active()) {
		return request.failed("recording not active");
	}

	obs_frontend_recording_stop();
	return request.success();
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
RpcResponse WSRequestHandler::PauseRecording(const RpcRequest& request) {
	return ifCanPause(request, [request]() {
		if (obs_frontend_recording_paused()) {
			return request.failed("recording already paused");
		}

		obs_frontend_recording_pause(true);
		return request.success();
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
RpcResponse WSRequestHandler::ResumeRecording(const RpcRequest& request) {
	return ifCanPause(request, [request]() {
		if (!obs_frontend_recording_paused()) {
			return request.failed("recording is not paused");
		}

		obs_frontend_recording_pause(false);
		return request.success();
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
RpcResponse WSRequestHandler::SetRecordingFolder(const RpcRequest& request) {
	if (!request.hasField("rec-folder")) {
		return request.failed("missing request parameters");
	}

	const char* newRecFolder = obs_data_get_string(request.parameters(), "rec-folder");
	bool success = Utils::SetRecordingFolder(newRecFolder);
	if (!success) {
		return request.failed("invalid request parameters");
	}

	return request.success();
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
RpcResponse WSRequestHandler::GetRecordingFolder(const RpcRequest& request) {
	const char* recFolder = Utils::GetRecordingFolder();

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "rec-folder", recFolder);

	return request.success(response);
}
