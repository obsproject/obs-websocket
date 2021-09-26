#include <util/profiler.h>

#include "WebSocketServer.h"
#include "requesthandler/RequestHandler.h"
#include "obs-websocket.h"
#include "utils/Compat.h"

struct SerialFrameBatch
{
	RequestHandler *requestHandler;
	size_t frameCount;
	size_t sleepUntilFrame;
	std::queue<Request> requests;
	std::vector<RequestResult> results;
	std::mutex conditionMutex;
	std::condition_variable condition;

	SerialFrameBatch(RequestHandler *requestHandler) :
		requestHandler(requestHandler),
		frameCount(0),
		sleepUntilFrame(0)
	{}
};

struct ParallelBatchResults
{
	RequestHandler *requestHandler;
	size_t requestCount;
	std::mutex resultsMutex;
	std::vector<json> results;
	std::condition_variable condition;

	ParallelBatchResults(RequestHandler *requestHandler, size_t requestCount) :
		requestHandler(requestHandler),
		requestCount(requestCount)
	{}
};

json ConstructRequestResult(RequestResult requestResult, json requestJson)
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
		Request request = serialFrameBatch->requests.front();
		// Process request and get result
		RequestResult requestResult = serialFrameBatch->requestHandler->ProcessRequest(request);
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

void WebSocketServer::ProcessRequestBatch(SessionPtr session, ObsWebSocketRequestBatchExecutionType executionType, std::vector<json> &requests, std::vector<json> &results)
{
	RequestHandler requestHandler(session);
	if (executionType == OBS_WEBSOCKET_REQUEST_BATCH_EXECUTION_TYPE_SERIAL_REALTIME) {
		// Recurse all requests in batch serially, processing the request then moving to the next one
		for (auto requestJson : requests) {
			Request request(requestJson["requestType"], requestJson["requestData"], executionType);

			RequestResult requestResult = requestHandler.ProcessRequest(request);

			json result = ConstructRequestResult(requestResult, requestJson);

			results.push_back(result);
		}
	} else if (executionType == OBS_WEBSOCKET_REQUEST_BATCH_EXECUTION_TYPE_SERIAL_FRAME) {
		SerialFrameBatch serialFrameBatch(&requestHandler);

		// Create Request objects in the worker thread (avoid unnecessary processing in graphics thread)
		for (auto requestJson : requests) {
			Request request(requestJson["requestType"], requestJson["requestData"], executionType);
			serialFrameBatch.requests.push(request);
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
		ParallelBatchResults parallelResults(&requestHandler, requests.size());

		// Submit each request as a task to the thread pool to be processed ASAP
		for (auto requestJson : requests) {
			_threadPool.start(Utils::Compat::CreateFunctionRunnable([&parallelResults, &executionType, requestJson]() {
				Request request(requestJson["requestType"], requestJson["requestData"], executionType);

				RequestResult requestResult = parallelResults.requestHandler->ProcessRequest(request);

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
