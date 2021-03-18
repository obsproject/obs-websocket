#include "obs-websocket.h"
#include "Utils.h"
#include "WSEvents.h"

#include "WSRequestHandler.h"

 /**
 * Get current virtual cam status.
 *
 * @return {boolean} `isVirtualCam` Current virtual camera status.
 * @return {String (optional)} `virtualCamTimecode` Time elapsed since virtual cam started (only present if virtual cam currently active).
 *
 * @api requests
 * @name GetVirtualCamStatus
 * @category virtual cam
 * @since 4.9.0
 */
RpcResponse WSRequestHandler::GetVirtualCamStatus(const RpcRequest& request) {
		auto events = GetEventsSystem();

		OBSDataAutoRelease data = obs_data_create();
		obs_data_set_bool(data, "isVirtualCam", obs_frontend_virtualcam_active());

		if (obs_frontend_virtualcam_active()) {
				QString virtualCamTimecode = events->getVirtualCamTimecode();
				obs_data_set_string(data, "virtualCamTimecode", virtualCamTimecode.toUtf8().constData());
		}

		return request.success(data);
}

/**
 * Toggle virtual cam on or off (depending on the current virtual cam state).
 *
 * @api requests
 * @name StartStopVirtualCam
 * @category virtual cam
 * @since 4.9.0
 */
RpcResponse WSRequestHandler::StartStopVirtualCam(const RpcRequest& request) {
	(obs_frontend_virtualcam_active() ? obs_frontend_virtualcam_stop() : obs_frontend_virtualcam_start());
	return request.success();
}

/**
 * Start virtual cam.
 * Will return an `error` if virtual cam is already active.
 *
 * @api requests
 * @name StartVirtualCam
 * @category virtual cam
 * @since 4.9.0
 */
RpcResponse WSRequestHandler::StartVirtualCam(const RpcRequest& request) {
	if (obs_frontend_virtualcam_active()) {
		return request.failed("virtual cam already active");
	}

	obs_frontend_start_virtualcam();
	return request.success();
}

/**
 * Stop virtual cam.
 * Will return an `error` if virtual cam is not active.
 *
 * @api requests
 * @name StopVirtualCam
 * @category virtual cam
 * @since 4.9.0
 */
 RpcResponse WSRequestHandler::StopVirtualCam(const RpcRequest& request) {
	if (!obs_frontend_virtualcam_active()) {
		return request.failed("virtual cam not active");
	}

	obs_frontend_stop_virtualcam();
	return request.success();
}
