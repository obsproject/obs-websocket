#include <util/profiler.h>

#include "WebSocketServer.h"
#include "../requesthandler/RequestHandler.h"
#include "../obs-websocket.h"
#include "../utils/Compat.h"

struct SerialFrameRequest
{
	Request request;
	const json inputVariables;
	const json outputVariables;

	SerialFrameRequest(const std::string &requestType, const json &requestData, const json &inputVariables, const json &outputVariables) :
		request(requestType, requestData, OBS_WEBSOCKET_REQUEST_BATCH_EXECUTION_TYPE_SERIAL_FRAME),
		inputVariables(inputVariables),
		outputVariables(outputVariables)
	{}
};

struct SerialFrameBatch
{
	RequestHandler &requestHandler;
	json &variables;
	size_t frameCount;
	size_t sleepUntilFrame;
	std::queue<SerialFrameRequest> requests;
	std::vector<RequestResult> results;
	std::mutex conditionMutex;
	std::condition_variable condition;

	SerialFrameBatch(RequestHandler &requestHandler, json &variables) :
		requestHandler(requestHandler),
		variables(variables),
		frameCount(0),
		sleepUntilFrame(0)
	{}
};

struct ParallelBatchResults
{
	RequestHandler &requestHandler;
	size_t requestCount;
	std::mutex resultsMutex;
	std::vector<json> results;
	std::condition_variable condition;

	ParallelBatchResults(RequestHandler &requestHandler, size_t requestCount) :
		requestHandler(requestHandler),
		requestCount(requestCount)
	{}
};



bool PreProcessVariables(const json &variables, const json &inputVariables, json &requestData)
{
	if (variables.empty() || inputVariables.empty() || !inputVariables.is_object() || !requestData.is_object())
		return !requestData.empty();

	for (auto it = inputVariables.begin(); it != inputVariables.end(); ++it) {
		std::string key = it.key();

		if (!variables.contains(key)) {
			if (IsDebugMode())
				blog(LOG_WARNING, "[WebSocketServer::ProcessRequestBatch] inputVariables requested variable `%s`, but it does not exist. Skipping!", key.c_str());
			continue;
		}

		if (!it.value().is_string()) {
			if (IsDebugMode())
				blog(LOG_WARNING, "[WebSocketServer::ProcessRequestBatch] Value of item `%s` in inputVariables is not a string. Skipping!", key.c_str());
			continue;
		}

		std::string value = it.value();
		requestData[value] = variables[key];
	}

	return !requestData.empty();
}

void PostProcessVariables(json &variables, const json &outputVariables, const json &responseData)
{
	if (outputVariables.empty() || !outputVariables.is_object() || responseData.empty())
		return;

	for (auto it = outputVariables.begin(); it != outputVariables.end(); ++it) {
		std::string key = it.key();

		if (!responseData.contains(key)) {
			if (IsDebugMode())
				blog(LOG_WARNING, "[WebSocketServer::ProcessRequestBatch] outputVariables requested responseData item `%s`, but it does not exist. Skipping!", key.c_str());
			continue;
		}

		if (!it.value().is_string()) {
			if (IsDebugMode())
				blog(LOG_WARNING, "[WebSocketServer::ProcessRequestBatch] Value of item `%s` in outputVariables is not a string. Skipping!", key.c_str());
			continue;
		}

		std::string value = it.value();
		variables[key] = responseData[value];
	}
}

json ConstructRequestResult(RequestResult requestResult, const json &requestJson)
{
	json ret;

	ret["requestType"] = requestJson["requestType"];

	if (requestJson.contains("requestId") && !requestJson["requestId"].is_null())
		ret["requestId"] = requestJson["requestId"];

	ret["requestStatus"] = {
		{"result", requestResult.StatusCode == RequestStatus::Success},
		{"code", requestResult.StatusCode}
	};

	if (!requestResult.Comment.empty())
		ret["requestStatus"]["comment"] = requestResult.Comment;

	if (requestResult.ResponseData.is_object())
		ret["responseData"] = requestResult.ResponseData;

	return ret;
}

void ObsTickCallback(void *param, float)
{
	profile_start("obs-websocket-request-batch-frame-tick");

	auto serialFrameBatch = reinterpret_cast<SerialFrameBatch*>(param);

	// Increment frame count
	serialFrameBatch->frameCount++;

	if (serialFrameBatch->sleepUntilFrame) {
		if (serialFrameBatch->frameCount < serialFrameBatch->sleepUntilFrame) {
			// Do not process any requests if in "sleep mode"
			profile_end("obs-websocket-request-batch-frame-tick");
			return;
		} else {
			// Reset frame sleep until counter if not being used
			serialFrameBatch->sleepUntilFrame = 0;
		}
	}

	// Begin recursing any unprocessed requests
	while (!serialFrameBatch->requests.empty()) {
		// Fetch first in queue
		SerialFrameRequest frameRequest = serialFrameBatch->requests.front();
		// Pre-process batch variables
		frameRequest.request.HasRequestData = PreProcessVariables(serialFrameBatch->variables, frameRequest.inputVariables, frameRequest.request.RequestData);
		// Process request and get result
		RequestResult requestResult = serialFrameBatch->requestHandler.ProcessRequest(frameRequest.request);
		// Post-process batch variables
		PostProcessVariables(serialFrameBatch->variables, frameRequest.outputVariables, requestResult.ResponseData);
		// Add to results vector
		serialFrameBatch->results.push_back(requestResult);
		// Remove from front of queue
		serialFrameBatch->requests.pop();

		// If the processed request tells us to sleep, do so accordingly
		if (requestResult.SleepFrames) {
			serialFrameBatch->sleepUntilFrame = serialFrameBatch->frameCount + requestResult.SleepFrames;
			break;
		}
	}

	// If request queue is empty, we can notify the paused worker thread
	if (serialFrameBatch->requests.empty()) {
		serialFrameBatch->condition.notify_one();
	}

	profile_end("obs-websocket-request-batch-frame-tick");
}

void WebSocketServer::ProcessRequestBatch(SessionPtr session, ObsWebSocketRequestBatchExecutionType executionType, std::vector<json> &requests, std::vector<json> &results, json &variables)
{
	RequestHandler requestHandler(session);
	if (executionType == OBS_WEBSOCKET_REQUEST_BATCH_EXECUTION_TYPE_SERIAL_REALTIME) {
		// Recurse all requests in batch serially, processing the request then moving to the next one
		for (auto requestJson : requests) {
			Request request(requestJson["requestType"], requestJson["requestData"], OBS_WEBSOCKET_REQUEST_BATCH_EXECUTION_TYPE_SERIAL_REALTIME);

			request.HasRequestData = PreProcessVariables(variables, requestJson["inputVariables"], request.RequestData);

			RequestResult requestResult = requestHandler.ProcessRequest(request);

			PostProcessVariables(variables, requestJson["outputVariables"], requestResult.ResponseData);

			json result = ConstructRequestResult(requestResult, requestJson);

			results.push_back(result);
		}
	} else if (executionType == OBS_WEBSOCKET_REQUEST_BATCH_EXECUTION_TYPE_SERIAL_FRAME) {
		SerialFrameBatch serialFrameBatch(requestHandler, variables);

		// Create Request objects in the worker thread (avoid unnecessary processing in graphics thread)
		for (auto requestJson : requests) {
			SerialFrameRequest frameRequest(requestJson["requestType"], requestJson["requestData"], requestJson["inputVariables"], requestJson["outputVariables"]);
			serialFrameBatch.requests.push(frameRequest);
		}

		// Create a callback entry for the graphics thread to execute on each video frame
		obs_add_tick_callback(ObsTickCallback, &serialFrameBatch);

		// Wait until the graphics thread processes the last request in the queue
		std::unique_lock<std::mutex> lock(serialFrameBatch.conditionMutex);
		serialFrameBatch.condition.wait(lock, [&serialFrameBatch]{return serialFrameBatch.requests.empty();});

		// Remove the created callback entry since we don't need it anymore
		obs_remove_tick_callback(ObsTickCallback, &serialFrameBatch);

		// Create Request objects in the worker thread (avoid unnecessary processing in graphics thread)
		size_t i = 0;
		for (auto requestResult : serialFrameBatch.results) {
			results.push_back(ConstructRequestResult(requestResult, requests[i]));
			i++;
		}
	} else if (executionType == OBS_WEBSOCKET_REQUEST_BATCH_EXECUTION_TYPE_PARALLEL) {
		ParallelBatchResults parallelResults(requestHandler, requests.size());

		// Submit each request as a task to the thread pool to be processed ASAP
		for (auto requestJson : requests) {
			_threadPool.start(Utils::Compat::CreateFunctionRunnable([&parallelResults, &executionType, requestJson]() {
				Request request(requestJson["requestType"], requestJson["requestData"], OBS_WEBSOCKET_REQUEST_BATCH_EXECUTION_TYPE_PARALLEL);

				RequestResult requestResult = parallelResults.requestHandler.ProcessRequest(request);

				json result = ConstructRequestResult(requestResult, requestJson);

				std::unique_lock<std::mutex> lock(parallelResults.resultsMutex);
				parallelResults.results.push_back(result);
				lock.unlock();
				parallelResults.condition.notify_one();
			}));
		}

		// Wait for the last request to finish processing
		std::unique_lock<std::mutex> lock(parallelResults.resultsMutex);
		auto cb = [&parallelResults]{return parallelResults.results.size() == parallelResults.requestCount;};
		// A check just in case all requests managed to complete before we started waiting for the condition to be notified
		if (!cb())
			parallelResults.condition.wait(lock, cb);

		results = parallelResults.results;
	}
}
