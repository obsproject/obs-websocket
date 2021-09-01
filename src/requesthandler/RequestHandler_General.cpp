#include <QImageWriter>

#include "RequestHandler.h"
#include "../eventhandler/types/EventSubscription.h"
#include "../obs-websocket.h"
#include "../WebSocketServer.h"
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
	if (!request.ValidateObject("eventData", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	auto webSocketServer = GetWebSocketServer();
	if (!webSocketServer)
		return RequestResult::Error(RequestStatus::RequestProcessingFailed, "Unable to send event.");

	webSocketServer->BroadcastEvent(EventSubscription::General, "CustomEvent", request.RequestData["eventData"]);

	return RequestResult::Success();
}

RequestResult RequestHandler::GetStats(const Request& request)
{
	json responseData = Utils::Obs::DataHelper::GetStats();

	responseData["webSocketSessionIncomingMessages"] = request.Session->IncomingMessages();
	responseData["webSocketSessionOutgoingMessages"] = request.Session->OutgoingMessages();

	return RequestResult::Success(responseData);
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
	if (!request.ValidateString("hotkeyName", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	obs_hotkey_t *hotkey = Utils::Obs::SearchHelper::GetHotkeyByName(request.RequestData["hotkeyName"]);
	if (!hotkey)
		return RequestResult::Error(RequestStatus::ResourceNotFound, "No hotkeys were found by that name.");

	obs_hotkey_trigger_routed_callback(obs_hotkey_get_id(hotkey), true);

	return RequestResult::Success();
}

RequestResult RequestHandler::TriggerHotkeyByKeySequence(const Request& request)
{
	obs_key_combination_t combo = {0};

	RequestStatus::RequestStatus statusCode = RequestStatus::NoError;
	std::string comment;

	if (request.RequestData.contains("keyId") && !request.RequestData["keyId"].is_null()) {
		if (!request.ValidateString("keyId", statusCode, comment))
			return RequestResult::Error(statusCode, comment);

		std::string keyId = request.RequestData["keyId"];
		combo.key = obs_key_from_name(keyId.c_str());
	}

	statusCode = RequestStatus::NoError;
	if (request.RequestData.contains("keyModifiers") && !request.RequestData["keyModifiers"].is_null()) {
		if (!request.ValidateObject("keyModifiers", statusCode, comment, true))
			return RequestResult::Error(statusCode, comment);

		const json keyModifiersJson = request.RequestData["keyModifiers"];
		uint32_t keyModifiers = 0;
		if (keyModifiersJson.contains("shift") && keyModifiersJson["shift"].is_boolean() && keyModifiersJson["shift"].get<bool>())
			keyModifiers |= INTERACT_SHIFT_KEY;
		if (keyModifiersJson.contains("control") && keyModifiersJson["control"].is_boolean() && keyModifiersJson["control"].get<bool>())
			keyModifiers |= INTERACT_CONTROL_KEY;
		if (keyModifiersJson.contains("alt") && keyModifiersJson["alt"].is_boolean() && keyModifiersJson["alt"].get<bool>())
			keyModifiers |= INTERACT_ALT_KEY;
		if (keyModifiersJson.contains("command") && keyModifiersJson["command"].is_boolean() && keyModifiersJson["command"].get<bool>())
			keyModifiers |= INTERACT_COMMAND_KEY;
		combo.modifiers = keyModifiers;
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
	if (!request.ValidateBoolean("studioModeEnabled", statusCode, comment))
		return RequestResult::Error(statusCode, comment);

	// Avoid queueing tasks if nothing will change
	if (obs_frontend_preview_program_mode_active() != request.RequestData["studioModeEnabled"]) {
		// (Bad) Create a boolean then pass it as a reference to the task. Requires `wait` in obs_queue_task() to be true, else undefined behavior
		bool studioModeEnabled = request.RequestData["studioModeEnabled"];
		// Queue the task inside of the UI thread to prevent race conditions
		obs_queue_task(OBS_TASK_UI, [](void* param) {
			auto studioModeEnabled = (bool*)param;
			obs_frontend_set_preview_program_mode(*studioModeEnabled);
		}, &studioModeEnabled, true);
	}

	return RequestResult::Success();
}

RequestResult RequestHandler::Sleep(const Request& request)
{
	RequestStatus::RequestStatus statusCode;
	std::string comment;
	if (!request.ValidateNumber("sleepMillis", statusCode, comment, 0, 50000))
		return RequestResult::Error(statusCode, comment);

	int64_t sleepMillis = request.RequestData["sleepMillis"];
	std::this_thread::sleep_for(std::chrono::milliseconds(sleepMillis));

	return RequestResult::Success();
}
