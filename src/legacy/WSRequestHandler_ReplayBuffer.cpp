#include <QString>
#include "src/Utils.h"

#include "WSRequestHandler.h"

/**
* Toggle the Replay Buffer on/off.
*
* @api requests
* @name StartStopReplayBuffer
* @category replay buffer
* @since 4.2.0
*/
void WSRequestHandler::HandleStartStopReplayBuffer(WSRequestHandler* req) {
    if (obs_frontend_replay_buffer_active()) {
        obs_frontend_replay_buffer_stop();
    } else {
        Utils::StartReplayBuffer();
    }
    req->SendOKResponse();
}

/**
* Start recording into the Replay Buffer.
* Will return an `error` if the Replay Buffer is already active or if the
* "Save Replay Buffer" hotkey is not set in OBS' settings.
* Setting this hotkey is mandatory, even when triggering saves only
* through obs-websocket.
*
* @api requests
* @name StartReplayBuffer
* @category replay buffer
* @since 4.2.0
*/
void WSRequestHandler::HandleStartReplayBuffer(WSRequestHandler* req) {
    if (!Utils::ReplayBufferEnabled()) {
        req->SendErrorResponse("replay buffer disabled in settings");
        return;
    }

    if (obs_frontend_replay_buffer_active() == true) {
        req->SendErrorResponse("replay buffer already active");
        return;
    }

    Utils::StartReplayBuffer();
    req->SendOKResponse();
}

/**
* Stop recording into the Replay Buffer.
* Will return an `error` if the Replay Buffer is not active.
*
* @api requests
* @name StopReplayBuffer
* @category replay buffer
* @since 4.2.0
*/
void WSRequestHandler::HandleStopReplayBuffer(WSRequestHandler* req) {
    if (obs_frontend_replay_buffer_active() == true) {
        obs_frontend_replay_buffer_stop();
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("replay buffer not active");
    }
}

/**
* Flush and save the contents of the Replay Buffer to disk. This is
* basically the same as triggering the "Save Replay Buffer" hotkey.
* Will return an `error` if the Replay Buffer is not active.
*
* @api requests
* @name SaveReplayBuffer
* @category replay buffer
* @since 4.2.0
*/
void WSRequestHandler::HandleSaveReplayBuffer(WSRequestHandler* req) {
    if (!obs_frontend_replay_buffer_active()) {
        req->SendErrorResponse("replay buffer not active");
        return;
    }

    OBSOutputAutoRelease replayOutput = obs_frontend_get_replay_buffer_output();

    calldata_t cd = { 0 };
    proc_handler_t* ph = obs_output_get_proc_handler(replayOutput);
    proc_handler_call(ph, "save", &cd);
    calldata_free(&cd);

    req->SendOKResponse();
}
