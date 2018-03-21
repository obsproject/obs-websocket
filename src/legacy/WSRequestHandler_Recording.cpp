#include <QString>
#include "src/Utils.h"

#include "WSRequestHandler.h"

/**
 * Toggle recording on or off.
 *
 * @api requests
 * @name StartStopRecording
 * @category recording
 * @since 0.3
 */
 void WSRequestHandler::HandleStartStopRecording(WSRequestHandler* req) {
    if (obs_frontend_recording_active())
        obs_frontend_recording_stop();
    else
        obs_frontend_recording_start();

    req->SendOKResponse();
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
 void WSRequestHandler::HandleStartRecording(WSRequestHandler* req) {
    if (obs_frontend_recording_active() == false) {
        obs_frontend_recording_start();
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("recording already active");
    }
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
void WSRequestHandler::HandleStopRecording(WSRequestHandler* req) {
    if (obs_frontend_recording_active() == true) {
        obs_frontend_recording_stop();
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("recording not active");
    }
}

/**
 * Change the current recording folder.
 *
 * @param {String} `rec-folder` Path of the recording folder.
 *
 * @api requests
 * @name SetRecordingFolder
 * @category recording
 * @since 4.1.0
 */
 void WSRequestHandler::HandleSetRecordingFolder(WSRequestHandler* req) {
    if (!req->hasField("rec-folder")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* newRecFolder = obs_data_get_string(req->data, "rec-folder");
    bool success = Utils::SetRecordingFolder(newRecFolder);
    if (success)
        req->SendOKResponse();
    else
        req->SendErrorResponse("invalid request parameters");
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
void WSRequestHandler::HandleGetRecordingFolder(WSRequestHandler* req) {
    const char* recFolder = Utils::GetRecordingFolder();

    OBSDataAutoRelease response = obs_data_create();
    obs_data_set_string(response, "rec-folder", recFolder);

    req->SendOKResponse(response);
}
