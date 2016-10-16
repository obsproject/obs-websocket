#include "WSRequestHandler.h"
#include "obs-websocket.h"
#include "Utils.h"

WSRequestHandler::WSRequestHandler(QWebSocket *client) {
	_client = client;

	messageMap["GetVersion"] = WSRequestHandler::HandleGetVersion;
	messageMap["GetAuthRequired"] = WSRequestHandler::HandleGetAuthRequired;
	messageMap["Authenticate"] = WSRequestHandler::HandleAuthenticate;

	messageMap["SetCurrentScene"] = WSRequestHandler::HandleSetCurrentScene;
	messageMap["GetCurrentScene"] = WSRequestHandler::HandleGetCurrentScene;
	messageMap["GetSceneList"] = WSRequestHandler::HandleGetSceneList;
	messageMap["SetSourceOrder"] = WSRequestHandler::ErrNotImplemented;
	messageMap["SetSourceRender"] = WSRequestHandler::ErrNotImplemented;
	messageMap["SetSceneItemPositionAndSize"] = WSRequestHandler::ErrNotImplemented;
	messageMap["GetStreamingStatus"] = WSRequestHandler::HandleGetStreamingStatus;
	messageMap["StartStopStreaming"] = WSRequestHandler::HandleStartStopStreaming;
	messageMap["StartStopRecording"] = WSRequestHandler::HandleStartStopRecording;
	messageMap["ToggleMute"] = WSRequestHandler::ErrNotImplemented;
	messageMap["GetVolumes"] = WSRequestHandler::ErrNotImplemented;
	messageMap["SetVolume"] = WSRequestHandler::ErrNotImplemented;
}

void WSRequestHandler::handleMessage(const char *message) {
	_requestData = obs_data_create_from_json(message);
	if (!_requestData) {
		blog(LOG_ERROR, "[obs-websockets] invalid JSON payload for '%s'", message);
		SendErrorResponse("invalid JSON payload");
		return;
	}

	_requestType = obs_data_get_string(_requestData, "request-type");
	_messageId = obs_data_get_int(_requestData, "message-id");

	void (*handlerFunc)(WSRequestHandler*) = (messageMap[_requestType]);

	if (handlerFunc != NULL) {
		handlerFunc(this);
	}
	else {
		SendErrorResponse("invalid request type");
	}
}

WSRequestHandler::~WSRequestHandler() {
	if (_requestData != NULL) {
		obs_data_release(_requestData);
	}
}

void WSRequestHandler::SendOKResponse(obs_data_t *additionalFields) {
	obs_data_t *response = obs_data_create();
	obs_data_set_string(response, "status", "ok");
	obs_data_set_int(response, "message-id", _messageId);

	if (additionalFields != NULL) {
		obs_data_apply(response, additionalFields);
	}

	_client->sendTextMessage(obs_data_get_json(response));

	obs_data_release(response);
}

void WSRequestHandler::SendErrorResponse(const char *errorMessage) {
	obs_data_t *response = obs_data_create();
	obs_data_set_string(response, "status", "error");
	obs_data_set_string(response, "error", errorMessage);
	obs_data_set_int(response, "message-id", _messageId);

	_client->sendTextMessage(obs_data_get_json(response));

	obs_data_release(response);
}

void WSRequestHandler::HandleGetVersion(WSRequestHandler *owner) {
	obs_data_t *data = obs_data_create();
	obs_data_set_double(data, "version", OBS_WEBSOCKET_VERSION);

	owner->SendOKResponse(data);

	obs_data_release(data);
}

void WSRequestHandler::HandleGetAuthRequired(WSRequestHandler *owner) {
	bool authRequired = false; // Auth isn't implemented yet
	
	obs_data_t *data = obs_data_create();
	obs_data_set_bool(data, "authRequired", authRequired);
	if (authRequired) {
		// Just here for protocol doc
		obs_data_set_string(data, "challenge", "");
		obs_data_set_string(data, "salt", "");
	}

	owner->SendOKResponse(data);

	obs_data_release(data);
}

void WSRequestHandler::HandleAuthenticate(WSRequestHandler *owner) {
	const char *auth = obs_data_get_string(owner->_requestData, "auth");
	if (!auth) {
		owner->SendErrorResponse("auth not specified!");
		return;
	}

	owner->SendOKResponse();
}

void WSRequestHandler::HandleSetCurrentScene(WSRequestHandler *owner) {
	const char *sceneName = obs_data_get_string(owner->_requestData, "scene-name");
	obs_source_t *source = obs_get_source_by_name(sceneName);

	if (source) {
		obs_frontend_set_current_scene(source);
		owner->SendOKResponse();
	}
	else {
		owner->SendErrorResponse("requested scene does not exist");
	}

	obs_source_release(source);
}

void WSRequestHandler::HandleGetCurrentScene(WSRequestHandler *owner) {
	obs_source_t *source = obs_frontend_get_current_scene();
	const char *name = obs_source_get_name(source);

	obs_data_t *data = obs_data_create();
	obs_data_set_string(data, "name", name);
	obs_data_set_array(data, "sources", Utils::GetSceneItems(source));

	owner->SendOKResponse(data);
	obs_data_release(data);
	obs_source_release(source);
}

void WSRequestHandler::HandleGetSceneList(WSRequestHandler *owner) {
	obs_data_t *data = obs_data_create();
	obs_data_set_string(data, "current-scene", obs_source_get_name(obs_frontend_get_current_scene()));
	obs_data_set_array(data, "scenes", Utils::GetScenes());

	owner->SendOKResponse(data);

	obs_data_release(data);
}

void WSRequestHandler::HandleGetStreamingStatus(WSRequestHandler *owner) {
	obs_data_t *data = obs_data_create();
	obs_data_set_bool(data, "streaming", obs_frontend_streaming_active());
	obs_data_set_bool(data, "recording", obs_frontend_recording_active());
	obs_data_set_bool(data, "preview-only", false);

	owner->SendOKResponse(data);
	obs_data_release(data);
}

void WSRequestHandler::HandleStartStopStreaming(WSRequestHandler *owner) {
	if (obs_frontend_streaming_active()) {
		obs_frontend_streaming_stop();
	}
	else {
		obs_frontend_streaming_start();
	}

	owner->SendOKResponse();
}

void WSRequestHandler::HandleStartStopRecording(WSRequestHandler *owner) {
	if (obs_frontend_recording_active()) {
		obs_frontend_recording_stop();
	}
	else {
		obs_frontend_recording_start();
	}

	owner->SendOKResponse();
}

void WSRequestHandler::ErrNotImplemented(WSRequestHandler *owner) {
	owner->SendErrorResponse("not implemented");
}
