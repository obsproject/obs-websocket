#include <QImageWriter>

#include "../obs-websocket.h"
#include "../WebSocketServer.h"

#include "RequestHandler.h"

#include "../plugin-macros.generated.h"

RequestResult RequestHandler::GetVersion(const Request& request)
{
	json responseData;

	responseData["obsVersion"] = Utils::Obs::StringHelper::GetObsVersionString();
	responseData["obsWebSocketVersion"] = OBS_WEBSOCKET_VERSION;
	responseData["rpcVersion"] = OBS_WEBSOCKET_RPC_VERSION;
	responseData["availableRequests"] = GetRequestList();

	QList<QByteArray> imageWriterFormats = QImageWriter::supportedImageFormats();
	std::vector<std::string> supportedImageFormats;
	for (const QByteArray& format : imageWriterFormats) {
		supportedImageFormats.push_back(format.toStdString());
	}
	responseData["supportedImageFormats"] = supportedImageFormats;

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::BroadcastCustomEvent(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateObject("eventData", statusCode, comment)) {
		return RequestResult::Error(statusCode, comment);
	}

	auto webSocketServer = GetWebSocketServer();
	if (!webSocketServer)
		return RequestResult::Error(RequestStatus::RequestProcessingFailed, "Unable to send event.");

	webSocketServer->BroadcastEvent((1 << 0), "CustomEvent", request.RequestData["eventData"]);

	return RequestResult::Success();
}

RequestResult RequestHandler::GetHotkeyList(const Request& request)
{
	json responseData;

	responseData["hotkeys"] = Utils::Obs::ListHelper::GetHotkeyNameList();

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::TriggerHotkeyByName(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateString("hotkeyName", statusCode, comment)) {
		return RequestResult::Error(statusCode, comment);
	}

	obs_hotkey_t *hotkey = Utils::Obs::SearchHelper::GetHotkeyByName(request.RequestData["hotkeyName"]);
	if (!hotkey)
		return RequestResult::Error(RequestStatus::HotkeyNotFound, "Unable to find a hotkey by that name.");

	obs_hotkey_trigger_routed_callback(obs_hotkey_get_id(hotkey), true);

	return RequestResult::Success();
}

RequestResult RequestHandler::TriggerHotkeyByKeySequence(const Request& request)
{
	obs_key_combination_t combo = {0};

	RequestStatus::RequestStatus statusCode = RequestStatus::NoError;
	std::string comment;
	if (request.ValidateString("keyId", statusCode, comment)) {
		std::string keyId = request.RequestData["keyId"];
		combo.key = obs_key_from_name(keyId.c_str());
	} else if (statusCode != RequestStatus::MissingRequestParameter) {
		return RequestResult::Error(statusCode, comment);
	}

	statusCode = RequestStatus::NoError;
	if (request.ValidateObject("keyModifiers", statusCode, comment, true)) {
		uint32_t keyModifiers = 0;
		if (request.RequestData["keyModifiers"].contains("shift") && request.RequestData["keyModifiers"]["shift"].is_boolean() && request.RequestData["keyModifiers"]["shift"].get<bool>())
			keyModifiers |= INTERACT_SHIFT_KEY;
		if (request.RequestData["keyModifiers"].contains("control") && request.RequestData["keyModifiers"]["control"].is_boolean() && request.RequestData["keyModifiers"]["control"].get<bool>())
			keyModifiers |= INTERACT_CONTROL_KEY;
		if (request.RequestData["keyModifiers"].contains("alt") && request.RequestData["keyModifiers"]["alt"].is_boolean() && request.RequestData["keyModifiers"]["alt"].get<bool>())
			keyModifiers |= INTERACT_ALT_KEY;
		if (request.RequestData["keyModifiers"].contains("command") && request.RequestData["keyModifiers"]["command"].is_boolean() && request.RequestData["keyModifiers"]["command"].get<bool>())
			keyModifiers |= INTERACT_COMMAND_KEY;
		combo.modifiers = keyModifiers;
	} else if (statusCode != RequestStatus::MissingRequestParameter) {
		return RequestResult::Error(statusCode, comment);
	}

	if (!combo.modifiers && (combo.key == OBS_KEY_NONE || combo.key >= OBS_KEY_LAST_VALUE))
		return RequestResult::Error(RequestStatus::CannotAct, "Your provided request parameters cannot be used to trigger a hotkey.");

	// Apparently things break when you don't start by setting the combo to false
	obs_hotkey_inject_event(combo, false);
	obs_hotkey_inject_event(combo, true);
	obs_hotkey_inject_event(combo, false);

	return RequestResult::Success();
}

RequestResult RequestHandler::GetStudioModeEnabled(const Request& request)
{
	json responseData;

	responseData["studioModeEnabled"] = obs_frontend_preview_program_mode_active();

	return RequestResult::Success(responseData);
}

RequestResult RequestHandler::SetStudioModeEnabled(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateBoolean("studioModeEnabled", statusCode, comment)) {
		return RequestResult::Error(statusCode, comment);
	}

	// Avoid queueing tasks if nothing will change
	if (obs_frontend_preview_program_mode_active() != request.RequestData["studioModeEnabled"]) {
		// (Bad) Create a boolean on the stack, then free it after the task is completed. Requires `wait` in obs_queue_task() to be true
		bool studioModeEnabled = request.RequestData["studioModeEnabled"];
		// Queue the task inside of the UI thread to prevent race conditions
		obs_queue_task(OBS_TASK_UI, [](void* param) {
			bool *studioModeEnabled = (bool*)param;
			obs_frontend_set_preview_program_mode(*studioModeEnabled);
		}, &studioModeEnabled, true);
	}

	return RequestResult::Success();
}

RequestResult RequestHandler::Sleep(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateNumber("sleepMillis", statusCode, comment, 0, 50000)) {
		return RequestResult::Error(statusCode, comment);
	}

	int64_t sleepMillis = request.RequestData["sleepMillis"];
	std::this_thread::sleep_for(std::chrono::milliseconds(sleepMillis));

	return RequestResult::Success();
}
