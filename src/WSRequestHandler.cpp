/**
 * obs-websocket
 * Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>
 * Copyright (C) 2017	Mikhail Swift <https://github.com/mikhailswift>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>
 */

#include <obs-data.h>

#include "Config.h"
#include "Utils.h"

#include "WSRequestHandler.h"

QHash<QString, void(*)(WSRequestHandler*)> WSRequestHandler::messageMap {
    { "GetVersion", WSRequestHandler::HandleGetVersion },
    { "GetAuthRequired", WSRequestHandler::HandleGetAuthRequired },
    { "Authenticate", WSRequestHandler::HandleAuthenticate },

    { "SetHeartbeat", WSRequestHandler::HandleSetHeartbeat },

    { "SetCurrentScene", WSRequestHandler::HandleSetCurrentScene },
    { "GetCurrentScene", WSRequestHandler::HandleGetCurrentScene },
    { "GetSceneList", WSRequestHandler::HandleGetSceneList },

    { "SetSourceRender", WSRequestHandler::HandleSetSceneItemRender }, // Retrocompat
    { "SetSceneItemRender", WSRequestHandler::HandleSetSceneItemRender },
    { "SetSceneItemPosition", WSRequestHandler::HandleSetSceneItemPosition },
    { "SetSceneItemTransform", WSRequestHandler::HandleSetSceneItemTransform },
    { "SetSceneItemCrop", WSRequestHandler::HandleSetSceneItemCrop },
    { "GetSceneItemProperties", WSRequestHandler::HandleGetSceneItemProperties },
    { "SetSceneItemProperties", WSRequestHandler::HandleSetSceneItemProperties },
    { "ResetSceneItem", WSRequestHandler::HandleResetSceneItem },

    { "GetStreamingStatus", WSRequestHandler::HandleGetStreamingStatus },
    { "StartStopStreaming", WSRequestHandler::HandleStartStopStreaming },
    { "StartStopRecording", WSRequestHandler::HandleStartStopRecording },
    { "StartStreaming", WSRequestHandler::HandleStartStreaming },
    { "StopStreaming", WSRequestHandler::HandleStopStreaming },
    { "StartRecording", WSRequestHandler::HandleStartRecording },
    { "StopRecording", WSRequestHandler::HandleStopRecording },

    { "StartStopReplayBuffer", WSRequestHandler::HandleStartStopReplayBuffer },
    { "StartReplayBuffer", WSRequestHandler::HandleStartReplayBuffer },
    { "StopReplayBuffer", WSRequestHandler::HandleStopReplayBuffer },
    { "SaveReplayBuffer", WSRequestHandler::HandleSaveReplayBuffer },

    { "SetRecordingFolder", WSRequestHandler::HandleSetRecordingFolder },
    { "GetRecordingFolder", WSRequestHandler::HandleGetRecordingFolder },

    { "GetTransitionList", WSRequestHandler::HandleGetTransitionList },
    { "GetCurrentTransition", WSRequestHandler::HandleGetCurrentTransition },
    { "SetCurrentTransition", WSRequestHandler::HandleSetCurrentTransition },
    { "SetTransitionDuration", WSRequestHandler::HandleSetTransitionDuration },
    { "GetTransitionDuration", WSRequestHandler::HandleGetTransitionDuration },

    { "SetVolume", WSRequestHandler::HandleSetVolume },
    { "GetVolume", WSRequestHandler::HandleGetVolume },
    { "ToggleMute", WSRequestHandler::HandleToggleMute },
    { "SetMute", WSRequestHandler::HandleSetMute },
    { "GetMute", WSRequestHandler::HandleGetMute },
    { "SetSyncOffset", WSRequestHandler::HandleSetSyncOffset },
    { "GetSyncOffset", WSRequestHandler::HandleGetSyncOffset },
    { "GetSpecialSources", WSRequestHandler::HandleGetSpecialSources },
    { "GetSourcesList", WSRequestHandler::HandleGetSourcesList },
    { "GetSourceTypesList", WSRequestHandler::HandleGetSourceTypesList },
    { "GetSourceSettings", WSRequestHandler::HandleGetSourceSettings },
    { "SetSourceSettings", WSRequestHandler::HandleSetSourceSettings },

    { "SetCurrentSceneCollection", WSRequestHandler::HandleSetCurrentSceneCollection },
    { "GetCurrentSceneCollection", WSRequestHandler::HandleGetCurrentSceneCollection },
    { "ListSceneCollections", WSRequestHandler::HandleListSceneCollections },

    { "SetCurrentProfile", WSRequestHandler::HandleSetCurrentProfile },
    { "GetCurrentProfile", WSRequestHandler::HandleGetCurrentProfile },
    { "ListProfiles", WSRequestHandler::HandleListProfiles },

    { "SetStreamSettings", WSRequestHandler::HandleSetStreamSettings },
    { "GetStreamSettings", WSRequestHandler::HandleGetStreamSettings },
    { "SaveStreamSettings", WSRequestHandler::HandleSaveStreamSettings },

    { "GetStudioModeStatus", WSRequestHandler::HandleGetStudioModeStatus },
    { "GetPreviewScene", WSRequestHandler::HandleGetPreviewScene },
    { "SetPreviewScene", WSRequestHandler::HandleSetPreviewScene },
    { "TransitionToProgram", WSRequestHandler::HandleTransitionToProgram },
    { "EnableStudioMode", WSRequestHandler::HandleEnableStudioMode },
    { "DisableStudioMode", WSRequestHandler::HandleDisableStudioMode },
    { "ToggleStudioMode", WSRequestHandler::HandleToggleStudioMode },

    { "SetTextGDIPlusProperties", WSRequestHandler::HandleSetTextGDIPlusProperties },
    { "GetTextGDIPlusProperties", WSRequestHandler::HandleGetTextGDIPlusProperties },

    { "GetBrowserSourceProperties", WSRequestHandler::HandleGetBrowserSourceProperties },
    { "SetBrowserSourceProperties", WSRequestHandler::HandleSetBrowserSourceProperties },

    { "SetWampSettings", WSRequestHandler::HandleSetWampSettings },
    { "GetWampSettings", WSRequestHandler::HandleGetWampSettings },
    { "GetWampStatus", WSRequestHandler::HandleGetWampStatus },
	
};

QSet<QString> WSRequestHandler::authNotRequired {
    "GetVersion",
    "GetAuthRequired",
    "Authenticate",
    "GetWampSettings",
    "GetWampStatus"
};

QMutex WSRequestHandler::_requestMutex(QMutex::Recursive);


WSRequestHandler::WSRequestHandler() : data(nullptr)
{
	
}

WSRequestHandler::~WSRequestHandler() {
}


bool WSRequestHandler::hasField(QString name) {
    if (!data || name.isEmpty() || name.isNull())
        return false;

    return obs_data_has_user_value(data, name.toUtf8());
}

bool WSRequestHandler::hasField(const char* name) {
    if (!data || !name)
        return false;

    return obs_data_has_user_value(data, name);
}

WSWebSocketRequestHandler::WSWebSocketRequestHandler(QWebSocket* client) : WSRequestHandler(),
_client(client),
_messageId(0),
_requestType("")
{
}

WSWebSocketRequestHandler::~WSWebSocketRequestHandler() {
}


bool WSWebSocketRequestHandler::isAuthenticated()
{
    return _client->property(PROP_AUTHENTICATED).toBool();
}

void WSWebSocketRequestHandler::setAuthenticated(bool auth)
{
    _client->setProperty(PROP_AUTHENTICATED, auth);
}

void WSWebSocketRequestHandler::SendOKResponse(obs_data_t* additionalFields) {
    OBSDataAutoRelease response = obs_data_create();
    obs_data_set_string(response, "status", "ok");
    obs_data_set_string(response, "message-id", _messageId);

    if (additionalFields)
        obs_data_apply(response, additionalFields);

    SendResponse(response);
}

void WSWebSocketRequestHandler::SendErrorResponse(const char* errorMessage, obs_data_t* additionalFields) {
    OBSDataAutoRelease response = obs_data_create();
    obs_data_set_string(response, "status", "error");
    if (additionalFields)
    {
        obs_data_set_obj(response, "error", additionalFields);
        obs_data_set_string(response, "reason", errorMessage);
    }
    else
    {
        obs_data_set_string(response, "error", errorMessage);
    }

    obs_data_set_string(response, "message-id", _messageId);

    SendResponse(response);
}

void WSWebSocketRequestHandler::SendResponse(obs_data_t* response)  {
    QString json = obs_data_get_json(response);
    _client->sendTextMessage(json);

    if (Config::Current()->DebugEnabled)
        blog(LOG_DEBUG, "Response << '%s'", json.toUtf8().constData());
}

void WSWebSocketRequestHandler::processIncomingMessage(QString textMessage) {
    QByteArray msgData = textMessage.toUtf8();
    const char* msg = msgData.constData();

    data = obs_data_create_from_json(msg);
    if (!data) {
        if (!msg)
            msg = "<null pointer>";
        
        blog(LOG_ERROR, "invalid JSON payload received for '%s'", msg);
        SendErrorResponse("invalid JSON payload");
        return;
    }

    if (Config::Current()->DebugEnabled) {
        blog(LOG_DEBUG, "Request >> '%s'", msg);
    }

    if (!hasField("request-type")
        || !hasField("message-id"))
    {
        SendErrorResponse("missing request parameters");
        return;
    }

    _requestType = obs_data_get_string(data, "request-type");
    _messageId = obs_data_get_string(data, "message-id");

    if (Config::Current()->AuthRequired
        && (_client->property(PROP_AUTHENTICATED).toBool() == false)
        && (authNotRequired.find(_requestType) == authNotRequired.end()))
    {
        SendErrorResponse("Not Authenticated");
        return;
    }

    void (*handlerFunc)(WSRequestHandler*) = (messageMap[_requestType]);
    QMutexLocker locker(&_requestMutex);//prevents two concurrent requests
    if (handlerFunc != nullptr)
        handlerFunc(this);
    else
        SendErrorResponse("invalid request type");
}

WSWampRequestHandler::WSWampRequestHandler(void(*_requestMethod)(WSRequestHandler*)) : WSRequestHandler(),
_requestMethod(_requestMethod),
_response(QVariantMap()),
_error(QString())
{
}

WSWampRequestHandler::~WSWampRequestHandler() {
}


bool WSWampRequestHandler::isAuthenticated()
{
    return true;
}

void WSWampRequestHandler::setAuthenticated(bool auth)
{
    UNUSED_PARAMETER(auth);
    //do nothing as all Wamp connections are considered authenticated
}

void WSWampRequestHandler::SendOKResponse(obs_data_t* additionalFields) {
    _response = Utils::MapFromData(additionalFields);
}

void WSWampRequestHandler::SendErrorResponse(const char* errorMessage, obs_data_t* additionalFields) {
    _error = QString(errorMessage);
    if (additionalFields)
        _response = Utils::MapFromData(additionalFields);
}

WampResult WSWampRequestHandler::processIncomingMessage(QVariantList args) {
    data = Utils::DataFromList(args);
    QMutexLocker locker(&_requestMutex);
    _requestMethod(this); //prevents two concurrent requests
    return WampResult(_response, _error);
}
