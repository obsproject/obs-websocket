/*
obs-websocket
Copyright (C) 2016-2021 Stephane Lepin <stephane.lepin@gmail.com>
Copyright (C) 2020-2021 Kyle Manning <tt2468@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <util/profiler.hpp>

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
		request(requestType, requestData, RequestBatchExecutionType::SerialFrame),
		inputVariables(inputVariables),
		outputVariables(outputVariables)
	{}
};

struct SerialFrameBatch
{
	RequestHandler &requestHandler;
	std::queue<SerialFrameRequest> requests;
	std::vector<RequestResult> results;
	json &variables;
	bool haltOnFailure;

	size_t frameCount;
	size_t sleepUntilFrame;
	std::mutex conditionMutex;
	std::condition_variable condition;

	SerialFrameBatch(RequestHandler &requestHandler, json &variables, bool haltOnFailure) :
		requestHandler(requestHandler),
		variables(variables),
		haltOnFailure(haltOnFailure),
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



static bool PreProcessVariables(const json &variables, const json &inputVariables, json &requestData)
{
	if (variables.empty() || inputVariables.empty() || !inputVariables.is_object() || !requestData.is_object())
		return !requestData.empty();

	for (auto it = inputVariables.begin(); it != inputVariables.end(); ++it) {
		std::string key = it.key();

		if (!variables.contains(key)) {
			blog_debug("[WebSocketServer::ProcessRequestBatch] inputVariables requested variable `%s`, but it does not exist. Skipping!", key.c_str());
			continue;
		}

		if (!it.value().is_string()) {
			blog_debug("[WebSocketServer::ProcessRequestBatch] Value of item `%s` in inputVariables is not a string. Skipping!", key.c_str());
			continue;
		}

		std::string value = it.value();
		requestData[value] = variables[key];
	}

	return !requestData.empty();
}

static void PostProcessVariables(json &variables, const json &outputVariables, const json &responseData)
{
	if (outputVariables.empty() || !outputVariables.is_object() || responseData.empty())
		return;

	for (auto it = outputVariables.begin(); it != outputVariables.end(); ++it) {
		std::string key = it.key();

		if (!responseData.contains(key)) {
			blog_debug("[WebSocketServer::ProcessRequestBatch] outputVariables requested responseData item `%s`, but it does not exist. Skipping!", key.c_str());
			continue;
		}

		if (!it.value().is_string()) {
			blog_debug("[WebSocketServer::ProcessRequestBatch] Value of item `%s` in outputVariables is not a string. Skipping!", key.c_str());
			continue;
		}

		std::string value = it.value();
		variables[key] = responseData[value];
	}
}

static json ConstructRequestResult(RequestResult requestResult, const json &requestJson)
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

static void ObsTickCallback(void *param, float)
{
	ScopeProfiler prof{"obs_websocket_request_batch_frame_tick"};

	auto serialFrameBatch = reinterpret_cast<SerialFrameBatch*>(param);

	// Increment frame count
	serialFrameBatch->frameCount++;

	if (serialFrameBatch->sleepUntilFrame) {
		if (serialFrameBatch->frameCount < serialFrameBatch->sleepUntilFrame) {
			// Do not process any requests if in "sleep mode"
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

		// If haltOnFailure and the request failed, clear the queue to make the batch return early.
		if (serialFrameBatch->haltOnFailure && requestResult.StatusCode != RequestStatus::Success) {
			serialFrameBatch->requests = std::queue<SerialFrameRequest>();
			break;
		}

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
}

void WebSocketServer::ProcessRequestBatch(SessionPtr session, RequestBatchExecutionType::RequestBatchExecutionType executionType, const std::vector<json> &requests, std::vector<json> &results, json &variables, bool haltOnFailure)
{
	RequestHandler requestHandler(session);
	if (executionType == RequestBatchExecutionType::SerialRealtime) {
		// Recurse all requests in batch serially, processing the request then moving to the next one
		for (auto requestJson : requests) {
			Request request(requestJson["requestType"], requestJson["requestData"], RequestBatchExecutionType::SerialRealtime);

			request.HasRequestData = PreProcessVariables(variables, requestJson["inputVariables"], request.RequestData);

			RequestResult requestResult = requestHandler.ProcessRequest(request);

			PostProcessVariables(variables, requestJson["outputVariables"], requestResult.ResponseData);

			json result = ConstructRequestResult(requestResult, requestJson);

			results.push_back(result);

			if (haltOnFailure && requestResult.StatusCode != RequestStatus::Success)
				break;
		}
	} else if (executionType == RequestBatchExecutionType::SerialFrame) {
		SerialFrameBatch serialFrameBatch(requestHandler, variables, haltOnFailure);

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
	} else if (executionType == RequestBatchExecutionType::Parallel) {
		ParallelBatchResults parallelResults(requestHandler, requests.size());

		// Submit each request as a task to the thread pool to be processed ASAP
		for (auto requestJson : requests) {
			_threadPool.start(Utils::Compat::CreateFunctionRunnable([&parallelResults, &executionType, requestJson]() {
				Request request(requestJson["requestType"], requestJson["requestData"], RequestBatchExecutionType::Parallel);

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
