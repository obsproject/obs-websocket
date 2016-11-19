/*
obs-websocket
Copyright (C) 2016	Stéphane Lepin <stephane.lepin@gmail.com>

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

#include "WSRequestHandler.h"
#include "obs-websocket.h"
#include "Config.h"
#include "Utils.h"

WSRequestHandler::WSRequestHandler(QWebSocket *client) :
	_authenticated(false),
	_messageId(0),
	_requestType(""),
	_requestData(nullptr)
{
	_client = client;

	messageMap["GetVersion"] = WSRequestHandler::HandleGetVersion;
	messageMap["GetAuthRequired"] = WSRequestHandler::HandleGetAuthRequired;
	messageMap["Authenticate"] = WSRequestHandler::HandleAuthenticate;

	messageMap["SetCurrentScene"] = WSRequestHandler::HandleSetCurrentScene;
	messageMap["GetCurrentScene"] = WSRequestHandler::HandleGetCurrentScene;
	messageMap["GetSceneList"] = WSRequestHandler::HandleGetSceneList;
	messageMap["SetSourceOrder"] = WSRequestHandler::ErrNotImplemented;
	messageMap["SetSourceRender"] = WSRequestHandler::HandleSetSourceRender;
	messageMap["SetSceneItemPositionAndSize"] = WSRequestHandler::ErrNotImplemented;
	messageMap["GetStreamingStatus"] = WSRequestHandler::HandleGetStreamingStatus;
	messageMap["StartStopStreaming"] = WSRequestHandler::HandleStartStopStreaming;
	messageMap["StartStopRecording"] = WSRequestHandler::HandleStartStopRecording;
	messageMap["ToggleMute"] = WSRequestHandler::ErrNotImplemented;
	messageMap["GetVolumes"] = WSRequestHandler::ErrNotImplemented;
	messageMap["SetVolume"] = WSRequestHandler::ErrNotImplemented;

	messageMap["GetTransitionList"] = WSRequestHandler::HandleGetTransitionList;
	messageMap["GetCurrentTransition"] = WSRequestHandler::HandleGetCurrentTransition;
	messageMap["SetCurrentTransition"] = WSRequestHandler::HandleSetCurrentTransition;

	authNotRequired.insert("GetVersion");
	authNotRequired.insert("GetAuthRequired");
	authNotRequired.insert("Authenticate");

	blog(LOG_INFO, "[obs-websockets] new client connected from %s:%d", _client->peerAddress().toString().toLocal8Bit(), _client->peerPort());

	connect(_client, &QWebSocket::textMessageReceived, this, &WSRequestHandler::processTextMessage);
	connect(_client, &QWebSocket::disconnected, this, &WSRequestHandler::socketDisconnected);
}

void WSRequestHandler::processTextMessage(QString textMessage) {
	QByteArray msgData = textMessage.toLocal8Bit();
	const char *msg = msgData;

	_requestData = obs_data_create_from_json(msg);
	if (!_requestData) {
		blog(LOG_ERROR, "[obs-websockets] invalid JSON payload for '%s'", msg);
		SendErrorResponse("invalid JSON payload");
		return;
	}

	_requestType = obs_data_get_string(_requestData, "request-type");
	_messageId = obs_data_get_string(_requestData, "message-id");

	if (Config::Current()->AuthRequired 
		&& !_authenticated 
		&& authNotRequired.find(_requestType) == authNotRequired.end()) 
	{
		SendErrorResponse("Not Authenticated");
		return;
	}

	void (*handlerFunc)(WSRequestHandler*) = (messageMap[_requestType]);

	if (handlerFunc != NULL) {
		handlerFunc(this);
	}
	else {
		SendErrorResponse("invalid request type");
	}
}

void WSRequestHandler::socketDisconnected() {
	blog(LOG_INFO, "[obs-websockets] client %s:%d disconnected", _client->peerAddress().toString().toStdString(), _client->peerPort());

	_authenticated = false;
	_client->deleteLater();
	emit disconnected();
}

void WSRequestHandler::sendTextMessage(QString textMessage) {
	_client->sendTextMessage(textMessage);
}

WSRequestHandler::~WSRequestHandler() {
	if (_requestData != NULL) {
		obs_data_release(_requestData);
	}
}

void WSRequestHandler::SendOKResponse(obs_data_t *additionalFields) {
	obs_data_t *response = obs_data_create();
	obs_data_set_string(response, "status", "ok");
	obs_data_set_string(response, "message-id", _messageId);

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
	obs_data_set_string(response, "message-id", _messageId);

	_client->sendTextMessage(obs_data_get_json(response));

	obs_data_release(response);
}

void WSRequestHandler::HandleGetVersion(WSRequestHandler *owner) {
	obs_data_t *data = obs_data_create();
	obs_data_set_double(data, "version", 1.1);
	obs_data_set_string(data, "obs-websocket-version", OBS_WEBSOCKET_VERSION);
	obs_data_set_string(data, "obs-studio-version", OBS_VERSION);
	owner->SendOKResponse(data);

	obs_data_release(data);
}

void WSRequestHandler::HandleGetAuthRequired(WSRequestHandler *owner) {
	bool authRequired = Config::Current()->AuthRequired;

	obs_data_t *data = obs_data_create();
	obs_data_set_bool(data, "authRequired", authRequired);

	if (authRequired) {
		obs_data_set_string(data, "challenge", Config::Current()->SessionChallenge);
		obs_data_set_string(data, "salt", Config::Current()->Salt);
	}

	owner->SendOKResponse(data);

	obs_data_release(data);
}

void WSRequestHandler::HandleAuthenticate(WSRequestHandler *owner) {
	const char *auth = obs_data_get_string(owner->_requestData, "auth");
	if (!auth || strlen(auth) < 1) {
		owner->SendErrorResponse("auth not specified!");
		return;
	}

	if (!(owner->_authenticated) && Config::Current()->CheckAuth(auth)) {
		owner->_authenticated = true;
		owner->SendOKResponse();
	}
	else {
		owner->SendErrorResponse("Authentication Failed.");
	}
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

	obs_data_array_t *scene_items = Utils::GetSceneItems(source);

	obs_data_t *data = obs_data_create();
	obs_data_set_string(data, "name", name);
	obs_data_set_array(data, "sources", scene_items);

	owner->SendOKResponse(data);
	obs_data_release(data);
	obs_data_array_release(scene_items);
	//obs_source_release(source); // causes a source destroy sometimes
}

void WSRequestHandler::HandleGetSceneList(WSRequestHandler *owner) {
	obs_data_t *data = obs_data_create();
	obs_data_set_string(data, "current-scene", obs_source_get_name(obs_frontend_get_current_scene()));
	obs_data_set_array(data, "scenes", Utils::GetScenes());

	owner->SendOKResponse(data);

	obs_data_release(data);
}

void WSRequestHandler::HandleSetSourceRender(WSRequestHandler *owner) {
	const char *itemName = obs_data_get_string(owner->_requestData, "source");
	bool isVisible = obs_data_get_bool(owner->_requestData, "render");
	if (itemName == NULL) {
		owner->SendErrorResponse("invalid request parameters");
		return;
	}

	obs_source_t* currentScene = obs_frontend_get_current_scene();
	
	obs_sceneitem_t *sceneItem = Utils::GetSceneItemFromName(currentScene, itemName);
	if (sceneItem != NULL) {
		obs_sceneitem_set_visible(sceneItem, isVisible);
		obs_sceneitem_release(sceneItem);
		owner->SendOKResponse();
	}
	else {
		owner->SendErrorResponse("specified scene item doesn't exist");
	}

	obs_source_release(currentScene);
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

void WSRequestHandler::HandleGetTransitionList(WSRequestHandler *owner) {
	obs_frontend_source_list transitionList = {};
	obs_frontend_get_transitions(&transitionList);

	obs_data_array_t* transitions = obs_data_array_create();
	for (size_t i = 0; i < (&transitionList)->sources.num; i++) {
		obs_source_t* transition = (&transitionList)->sources.array[i];
		
		obs_data_t *obj = obs_data_create();
		obs_data_set_string(obj, "name", obs_source_get_name(transition));

		obs_data_array_push_back(transitions, obj);
	}
	obs_frontend_source_list_free(&transitionList);

	obs_data_t *response = obs_data_create();
	obs_data_set_string(response, "current-transition", obs_source_get_name(obs_frontend_get_current_transition()));
	obs_data_set_array(response, "transitions", transitions);
	owner->SendOKResponse(response);

	obs_data_release(response);
}

void WSRequestHandler::HandleGetCurrentTransition(WSRequestHandler *owner) {
	obs_data_t *response = obs_data_create();
	obs_data_set_string(response, "name", obs_source_get_name(obs_frontend_get_current_transition()));
	owner->SendOKResponse(response);

	obs_data_release(response);
}

void WSRequestHandler::HandleSetCurrentTransition(WSRequestHandler *owner) {
	const char *name = obs_data_get_string(owner->_requestData, "transition-name");
	obs_source_t *transition = obs_get_source_by_name(name);

	if (transition) {
		obs_frontend_set_current_transition(transition);
		owner->SendOKResponse();

		obs_source_release(transition);
	}
	else {
		owner->SendErrorResponse("requested transition does not exist");
	}
}

void WSRequestHandler::ErrNotImplemented(WSRequestHandler *owner) {
	owner->SendErrorResponse("not implemented");
}
