#include <functional>

#include "WSRequestHandler.h"

/**
* @typedef {Object} `Output`
* @property {String} `name` Output name
* @property {String} `type` Output type/kind
* @property {int} `width` Video output width
* @property {int} `height` Video output height
* @property {Object} `flags` Output flags
* @property {int} `flags.rawValue` Raw flags value
* @property {boolean} `flags.audio` Output uses audio
* @property {boolean} `flags.video` Output uses video
* @property {boolean} `flags.encoded` Output is encoded
* @property {boolean} `flags.multiTrack` Output uses several audio tracks
* @property {boolean} `flags.service` Output uses a service
* @property {Object} `settings` Output name
* @property {boolean} `active` Output status (active or not)
* @property {boolean} `reconnecting` Output reconnection status (reconnecting or not)
* @property {double} `congestion` Output congestion
* @property {int} `totalFrames` Number of frames sent
* @property {int} `droppedFrames` Number of frames dropped
* @property {int} `totalBytes` Total bytes sent
*/
obs_data_t* getOutputInfo(obs_output_t* output)
{
	if (!output) {
		return nullptr;
	}

	OBSDataAutoRelease settings = obs_output_get_settings(output);

	uint32_t rawFlags = obs_output_get_flags(output);
	OBSDataAutoRelease flags = obs_data_create();
	obs_data_set_int(flags, "rawValue", rawFlags);
	obs_data_set_bool(flags, "audio", rawFlags & OBS_OUTPUT_AUDIO);
	obs_data_set_bool(flags, "video", rawFlags & OBS_OUTPUT_VIDEO);
	obs_data_set_bool(flags, "encoded", rawFlags & OBS_OUTPUT_ENCODED);
	obs_data_set_bool(flags, "multiTrack", rawFlags & OBS_OUTPUT_MULTI_TRACK);
	obs_data_set_bool(flags, "service",  rawFlags & OBS_OUTPUT_SERVICE);

	obs_data_t* data = obs_data_create();

	obs_data_set_string(data, "name", obs_output_get_name(output));
	obs_data_set_string(data, "type", obs_output_get_id(output));
	obs_data_set_int(data, "width", obs_output_get_width(output));
	obs_data_set_int(data, "height", obs_output_get_height(output));
	obs_data_set_obj(data, "flags", flags);
	obs_data_set_obj(data, "settings", settings);

	obs_data_set_bool(data, "active", obs_output_active(output));
	obs_data_set_bool(data, "reconnecting", obs_output_reconnecting(output));
	obs_data_set_double(data, "congestion", obs_output_get_congestion(output));
	obs_data_set_int(data, "totalFrames", obs_output_get_total_frames(output));
	obs_data_set_int(data, "droppedFrames", obs_output_get_frames_dropped(output));
	obs_data_set_int(data, "totalBytes", obs_output_get_total_bytes(output));

	return data;
}

RpcResponse findOutputOrFail(const RpcRequest& request, std::function<RpcResponse (obs_output_t*)> callback)
{
	if (!request.hasField("outputName")) {
		return request.failed("missing request parameters");
	}

	const char* outputName = obs_data_get_string(request.parameters(), "outputName");
	OBSOutputAutoRelease output = obs_get_output_by_name(outputName);
	if (!output) {
		return request.failed("specified output doesn't exist");
	}

	return callback(output);
}

/**
* List existing outputs
*
* @return {Array<Output>} `outputs` Outputs list
*
* @api requests
* @name ListOutputs
* @category outputs
* @since 4.7.0
*/
RpcResponse WSRequestHandler::ListOutputs(const RpcRequest& request)
{
	OBSDataArrayAutoRelease outputs = obs_data_array_create();

	obs_enum_outputs([](void* param, obs_output_t* output) {
		obs_data_array_t* outputs = reinterpret_cast<obs_data_array_t*>(param);

		OBSDataAutoRelease outputInfo = getOutputInfo(output);
		obs_data_array_push_back(outputs, outputInfo);

		return true;
	}, outputs);

	OBSDataAutoRelease fields = obs_data_create();
	obs_data_set_array(fields, "outputs", outputs);

	return request.success(fields);
}

/**
* Get information about a single output
*
* @param {String} `outputName` Output name
*
* @return {Output} `outputInfo` Output info
*
* @api requests
* @name GetOutputInfo
* @category outputs
* @since 4.7.0
*/
RpcResponse WSRequestHandler::GetOutputInfo(const RpcRequest& request)
{
	return findOutputOrFail(request, [request](obs_output_t* output) {
		OBSDataAutoRelease outputInfo = getOutputInfo(output);

		OBSDataAutoRelease fields = obs_data_create();
		obs_data_set_obj(fields, "outputInfo", outputInfo);
		return request.success(fields);
	});
}

/**
* Start an output
*
* Note: Controlling outputs is an experimental feature of obs-websocket. Some plugins which add outputs to OBS may not function properly when they are controlled in this way.
*
* @param {String} `outputName` Output name
*
* @api requests
* @name StartOutput
* @category outputs
* @since 4.7.0
*/
RpcResponse WSRequestHandler::StartOutput(const RpcRequest& request)
{
	return findOutputOrFail(request, [request](obs_output_t* output) {
		if (obs_output_active(output)) {
			return request.failed("output already active");
		}

		bool success = obs_output_start(output);
		if (!success) {
			QString lastError = obs_output_get_last_error(output);
			QString errorMessage = QString("output start failed: %1").arg(lastError);
			return request.failed(errorMessage);
		}

		return request.success();
	});
}

/**
* Stop an output
*
* Note: Controlling outputs is an experimental feature of obs-websocket. Some plugins which add outputs to OBS may not function properly when they are controlled in this way.
*
* @param {String} `outputName` Output name
* @param {boolean (optional)} `force` Force stop (default: false)
*
* @api requests
* @name StopOutput
* @category outputs
* @since 4.7.0
*/
RpcResponse WSRequestHandler::StopOutput(const RpcRequest& request)
{
	return findOutputOrFail(request, [request](obs_output_t* output) {
		if (!obs_output_active(output)) {
			return request.failed("output not active");
		}

		bool forceStop = obs_data_get_bool(request.parameters(), "force");
		if (forceStop) {
			obs_output_force_stop(output);
		} else {
			obs_output_stop(output);
		}

		return request.success();
	});
}
