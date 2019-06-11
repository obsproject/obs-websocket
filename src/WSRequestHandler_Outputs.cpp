#include "WSRequestHandler.h"

/**
* @typedef {Object} `Output`
* @property {String} `name` Output name
* @property {String} `type` Output type/kind
* @property {Object} `settings` Output name
* @property {boolean} `active` Output status (active or not)
* @property {double} `congestion` Output congestion
*/
obs_data_t* getOutputInfo(obs_output_t* output)
{
	if (!output) {
		return nullptr;
	}

	OBSDataAutoRelease settings = obs_output_get_settings(output);

	obs_data_t* data = obs_data_create();
	obs_data_set_string(data, "name", obs_output_get_name(output));
	obs_data_set_string(data, "type", obs_output_get_id(output));
	// TODO flags
	obs_data_set_obj(data, "settings", settings);
	obs_data_set_bool(data, "active", obs_output_active(output));
	// TODO reconnecting
	obs_data_set_double(data, "congestion", obs_output_get_congestion(output));
	// TODO width
	// TODO height
	// TODO delay
	// TODO active delay
	// TODO connect time ms
	// TODO total frames
	// TODO frames dropped
	// TODO total bytes
	return data;
}

HandlerResponse findOutputOrFail(WSRequestHandler* req, std::function<HandlerResponse (obs_output_t*)> callback)
{
	if (!req->hasField("outputName")) {
		return req->SendErrorResponse("missing request parameters");
	}

	const char* outputName = obs_data_get_string(req->parameters(), "outputName");
	OBSOutputAutoRelease output = obs_get_output_by_name(outputName);
	if (!output) {
		return req->SendErrorResponse("specified output doesn't exist");
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
HandlerResponse WSRequestHandler::HandleListOutputs(WSRequestHandler* req)
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
	return req->SendOKResponse(fields);
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
HandlerResponse WSRequestHandler::HandleGetOutputInfo(WSRequestHandler* req)
{
	return findOutputOrFail(req, [req](obs_output_t* output) {
		OBSDataAutoRelease outputInfo = getOutputInfo(output);

		OBSDataAutoRelease fields = obs_data_create();
		obs_data_set_obj(fields, "outputInfo", outputInfo);
		return req->SendOKResponse(fields);
	});
}

/**
* Start an output
*
* @param {String} `outputName` Output name
*
* @api requests
* @name StartOutput
* @category outputs
* @since 4.7.0
*/
HandlerResponse WSRequestHandler::HandleStartOutput(WSRequestHandler* req)
{
	return findOutputOrFail(req, [req](obs_output_t* output) {
		// TODO check if already active
		if (!obs_output_start(output)) {
			// TODO get last error message
			return req->SendErrorResponse("output start failed");
		}
		return req->SendOKResponse();
	});
}

/**
* Stop an output
*
* @param {String} `outputName` Output name
* @param {boolean (optional)} `force` Force stop (default: false)
*
* @api requests
* @name StopOutput
* @category outputs
* @since 4.7.0
*/
HandlerResponse WSRequestHandler::HandleStopOutput(WSRequestHandler* req)
{
	return findOutputOrFail(req, [req](obs_output_t* output) {
		// TODO check if output is already stopped
		
		bool forceStop = obs_data_get_bool(req->data, "force");
		if (forceStop) {
			obs_output_force_stop(output);
		} else {
			obs_output_stop(output);
		}

		return req->SendOKResponse();
	});
}
