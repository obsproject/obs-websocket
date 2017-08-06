/*
obs-websocket
Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>
Copyright (C) 2017	Mikhail Swift <https://github.com/mikhailswift>

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

#include <obs-data.h>
#include "WSRequestHandler.h"
#include "WSEvents.h"
#include "obs-websocket.h"
#include "Config.h"
#include "Utils.h"
#include <qstring.h>

bool str_valid(const char* str) {
    return (str != nullptr && strlen(str) > 0);
}

obs_service_t* WSRequestHandler::_service = nullptr;

WSRequestHandler::WSRequestHandler(QWebSocket* client) :
    _messageId(0),
    _requestType(""),
    data(nullptr),
    _client(client) {
    messageMap["GetVersion"] = WSRequestHandler::HandleGetVersion;
    messageMap["GetAuthRequired"] = WSRequestHandler::HandleGetAuthRequired;
    messageMap["Authenticate"] = WSRequestHandler::HandleAuthenticate;

    messageMap["SetCurrentScene"] = WSRequestHandler::HandleSetCurrentScene;
    messageMap["GetCurrentScene"] = WSRequestHandler::HandleGetCurrentScene;
    messageMap["GetSceneList"] = WSRequestHandler::HandleGetSceneList;

    messageMap["SetSourceRender"] = WSRequestHandler::HandleSetSceneItemRender; // Retrocompat
    messageMap["SetSceneItemRender"] = WSRequestHandler::HandleSetSceneItemRender;
    messageMap["SetSceneItemPosition"] = WSRequestHandler::HandleSetSceneItemPosition;
    messageMap["SetSceneItemTransform"] = WSRequestHandler::HandleSetSceneItemTransform;
    messageMap["SetSceneItemCrop"] = WSRequestHandler::HandleSetSceneItemCrop;
    messageMap["ResetSceneItem"] = WSRequestHandler::HandleResetSceneItem;

    messageMap["GetStreamingStatus"] = WSRequestHandler::HandleGetStreamingStatus;
    messageMap["StartStopStreaming"] = WSRequestHandler::HandleStartStopStreaming;
    messageMap["StartStopRecording"] = WSRequestHandler::HandleStartStopRecording;
    messageMap["StartStreaming"] = WSRequestHandler::HandleStartStreaming;
    messageMap["StopStreaming"] = WSRequestHandler::HandleStopStreaming;
    messageMap["StartRecording"] = WSRequestHandler::HandleStartRecording;
    messageMap["StopRecording"] = WSRequestHandler::HandleStopRecording;

    messageMap["StartStopReplayBuffer"] = WSRequestHandler::HandleStartStopReplayBuffer;
    messageMap["StartReplayBuffer"] = WSRequestHandler::HandleStartReplayBuffer;
    messageMap["StopReplayBuffer"] = WSRequestHandler::HandleStopReplayBuffer;  
    messageMap["SaveReplayBuffer"] = WSRequestHandler::HandleSaveReplayBuffer;

    messageMap["SetRecordingFolder"] = WSRequestHandler::HandleSetRecordingFolder;
    messageMap["GetRecordingFolder"] = WSRequestHandler::HandleGetRecordingFolder;

    messageMap["GetTransitionList"] = WSRequestHandler::HandleGetTransitionList;
    messageMap["GetCurrentTransition"] = WSRequestHandler::HandleGetCurrentTransition;
    messageMap["SetCurrentTransition"] = WSRequestHandler::HandleSetCurrentTransition;
    messageMap["SetTransitionDuration"] = WSRequestHandler::HandleSetTransitionDuration;
    messageMap["GetTransitionDuration"] = WSRequestHandler::HandleGetTransitionDuration;

    messageMap["SetVolume"] = WSRequestHandler::HandleSetVolume;
    messageMap["GetVolume"] = WSRequestHandler::HandleGetVolume;
    messageMap["ToggleMute"] = WSRequestHandler::HandleToggleMute;
    messageMap["SetMute"] = WSRequestHandler::HandleSetMute;
    messageMap["GetMute"] = WSRequestHandler::HandleGetMute;
    messageMap["GetSpecialSources"] = WSRequestHandler::HandleGetSpecialSources;

    messageMap["SetCurrentSceneCollection"] = WSRequestHandler::HandleSetCurrentSceneCollection;
    messageMap["GetCurrentSceneCollection"] = WSRequestHandler::HandleGetCurrentSceneCollection;
    messageMap["ListSceneCollections"] = WSRequestHandler::HandleListSceneCollections;

    messageMap["SetCurrentProfile"] = WSRequestHandler::HandleSetCurrentProfile;
    messageMap["GetCurrentProfile"] = WSRequestHandler::HandleGetCurrentProfile;
    messageMap["ListProfiles"] = WSRequestHandler::HandleListProfiles;

    messageMap["SetStreamSettings"] = WSRequestHandler::HandleSetStreamSettings;
    messageMap["GetStreamSettings"] = WSRequestHandler::HandleGetStreamSettings;
    messageMap["SaveStreamSettings"] = WSRequestHandler::HandleSaveStreamSettings;

    messageMap["GetStudioModeStatus"] = WSRequestHandler::HandleGetStudioModeStatus;
    messageMap["GetPreviewScene"] = WSRequestHandler::HandleGetPreviewScene;
    messageMap["SetPreviewScene"] = WSRequestHandler::HandleSetPreviewScene;
    messageMap["TransitionToProgram"] = WSRequestHandler::HandleTransitionToProgram;
    messageMap["EnableStudioMode"] = WSRequestHandler::HandleEnableStudioMode;
    messageMap["DisableStudioMode"] = WSRequestHandler::HandleDisableStudioMode;
    messageMap["ToggleStudioMode"] = WSRequestHandler::HandleToggleStudioMode;

    messageMap["SetTextGDIPlusProperties"] = WSRequestHandler::HandleSetTextGDIPlusProperties;
    messageMap["GetTextGDIPlusProperties"] = WSRequestHandler::HandleGetTextGDIPlusProperties;

    messageMap["GetBrowserSourceProperties"] = WSRequestHandler::HandleGetBrowserSourceProperties;
    messageMap["SetBrowserSourceProperties"] = WSRequestHandler::HandleSetBrowserSourceProperties;

    authNotRequired.insert("GetVersion");
    authNotRequired.insert("GetAuthRequired");
    authNotRequired.insert("Authenticate");
}

void WSRequestHandler::processIncomingMessage(QString textMessage) {
    QByteArray msgData = textMessage.toUtf8();
    const char* msg = msgData;

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

    if (!hasField("request-type") ||
        !hasField("message-id")) {
        SendErrorResponse("missing request parameters");
        return;
    }

    _requestType = obs_data_get_string(data, "request-type");
    _messageId = obs_data_get_string(data, "message-id");

    if (Config::Current()->AuthRequired
        && (_client->property(PROP_AUTHENTICATED).toBool() == false)
        && (authNotRequired.find(_requestType) == authNotRequired.end())) {
        SendErrorResponse("Not Authenticated");
        return;
    }

    void (*handlerFunc)(WSRequestHandler*) = (messageMap[_requestType]);

    if (handlerFunc != NULL)
        handlerFunc(this);
    else
        SendErrorResponse("invalid request type");

    obs_data_release(data);
}

WSRequestHandler::~WSRequestHandler() {
}

void WSRequestHandler::SendOKResponse(obs_data_t* additionalFields) {
    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "status", "ok");
    obs_data_set_string(response, "message-id", _messageId);

    if (additionalFields)
        obs_data_apply(response, additionalFields);

    SendResponse(response);
}

void WSRequestHandler::SendErrorResponse(const char* errorMessage) {
    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "status", "error");
    obs_data_set_string(response, "error", errorMessage);
    obs_data_set_string(response, "message-id", _messageId);

    SendResponse(response);
}

void WSRequestHandler::SendResponse(obs_data_t* response)  {
    const char *json = obs_data_get_json(response);
    _client->sendTextMessage(json);
    if (Config::Current()->DebugEnabled)
        blog(LOG_DEBUG, "Response << '%s'", json);
    
    obs_data_release(response);
}

bool WSRequestHandler::hasField(const char* name) {
    if (!name || !data)
        return false;

    return obs_data_has_user_value(data, name);
}

void WSRequestHandler::HandleGetVersion(WSRequestHandler* req) {
    const char* obs_version = Utils::OBSVersionString();

    obs_data_t* data = obs_data_create();
    obs_data_set_double(data, "version", API_VERSION);
    obs_data_set_string(data, "obs-websocket-version", OBS_WEBSOCKET_VERSION);
    obs_data_set_string(data, "obs-studio-version", obs_version);

    req->SendOKResponse(data);

    obs_data_release(data);
    bfree((void*)obs_version);
}

void WSRequestHandler::HandleGetAuthRequired(WSRequestHandler* req) {
    bool authRequired = Config::Current()->AuthRequired;

    obs_data_t* data = obs_data_create();
    obs_data_set_bool(data, "authRequired", authRequired);

    if (authRequired) {
        obs_data_set_string(data, "challenge",
            Config::Current()->SessionChallenge);
        obs_data_set_string(data, "salt",
            Config::Current()->Salt);
    }

    req->SendOKResponse(data);

    obs_data_release(data);
}

void WSRequestHandler::HandleAuthenticate(WSRequestHandler* req) {
    if (!req->hasField("auth")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* auth = obs_data_get_string(req->data, "auth");
    if (!str_valid(auth)) {
        req->SendErrorResponse("auth not specified!");
        return;
    }

    if ((req->_client->property(PROP_AUTHENTICATED).toBool() == false)
        && Config::Current()->CheckAuth(auth)) {
        req->_client->setProperty(PROP_AUTHENTICATED, true);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("Authentication Failed.");
    }
}

void WSRequestHandler::HandleSetCurrentScene(WSRequestHandler* req) {
    if (!req->hasField("scene-name")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* sceneName = obs_data_get_string(req->data, "scene-name");
    obs_source_t* source = obs_get_source_by_name(sceneName);

    if (source) {
        obs_frontend_set_current_scene(source);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("requested scene does not exist");
    }

    obs_source_release(source);
}

void WSRequestHandler::HandleGetCurrentScene(WSRequestHandler* req) {
    obs_source_t* current_scene = obs_frontend_get_current_scene();
    const char* name = obs_source_get_name(current_scene);

    obs_data_array_t* scene_items = Utils::GetSceneItems(current_scene);

    obs_data_t* data = obs_data_create();
    obs_data_set_string(data, "name", name);
    obs_data_set_array(data, "sources", scene_items);

    req->SendOKResponse(data);

    obs_data_release(data);
    obs_data_array_release(scene_items);
    obs_source_release(current_scene);
}

void WSRequestHandler::HandleGetSceneList(WSRequestHandler* req) {
    obs_source_t* current_scene = obs_frontend_get_current_scene();
    obs_data_array_t* scenes = Utils::GetScenes();

    obs_data_t* data = obs_data_create();
    obs_data_set_string(data, "current-scene",
        obs_source_get_name(current_scene));
    obs_data_set_array(data, "scenes", scenes);

    req->SendOKResponse(data);

    obs_data_release(data);
    obs_data_array_release(scenes);
    obs_source_release(current_scene);
}

void WSRequestHandler::HandleSetSceneItemRender(WSRequestHandler* req) {
    if (!req->hasField("source") ||
        !req->hasField("render")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* itemName = obs_data_get_string(req->data, "source");
    bool isVisible = obs_data_get_bool(req->data, "render");

    if (!itemName) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    const char* sceneName = obs_data_get_string(req->data, "scene-name");
    obs_source_t* scene = Utils::GetSceneFromNameOrCurrent(sceneName);
    if (!scene) {
        req->SendErrorResponse("requested scene doesn't exist");
        return;
    }

    obs_sceneitem_t* sceneItem = Utils::GetSceneItemFromName(scene, itemName);
    if (sceneItem) {
        obs_sceneitem_set_visible(sceneItem, isVisible);
        obs_sceneitem_release(sceneItem);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("specified scene item doesn't exist");
    }

    obs_source_release(scene);
}

void WSRequestHandler::HandleGetStreamingStatus(WSRequestHandler* req) {
    obs_data_t* data = obs_data_create();
    obs_data_set_bool(data, "streaming", obs_frontend_streaming_active());
    obs_data_set_bool(data, "recording", obs_frontend_recording_active());
    obs_data_set_bool(data, "preview-only", false);

    const char* tc = nullptr;
    if (obs_frontend_streaming_active()) {
        tc = WSEvents::Instance->GetStreamingTimecode();
        obs_data_set_string(data, "stream-timecode", tc);
        bfree((void*)tc);
    }

    if (obs_frontend_recording_active()) {
        tc = WSEvents::Instance->GetRecordingTimecode();
        obs_data_set_string(data, "rec-timecode", tc);
        bfree((void*)tc);
    }

    req->SendOKResponse(data);
    obs_data_release(data);
}

void WSRequestHandler::HandleStartStopStreaming(WSRequestHandler* req) {
    if (obs_frontend_streaming_active())
        HandleStopStreaming(req);
    else
        HandleStartStreaming(req);
}

void WSRequestHandler::HandleStartStopRecording(WSRequestHandler* req)
{
    if (obs_frontend_recording_active())
        obs_frontend_recording_stop();
    else
        obs_frontend_recording_start();

    req->SendOKResponse();
}

void WSRequestHandler::HandleStartStreaming(WSRequestHandler* req)
{
    if (obs_frontend_streaming_active() == false) {
        obs_data_t* streamData = obs_data_get_obj(req->data, "stream");
        obs_service_t* currentService = nullptr;
        
        if (streamData) {
            currentService = obs_frontend_get_streaming_service();
            obs_service_addref(currentService);
            
            obs_service_t* service = _service;
            const char* currentServiceType = obs_service_get_type(currentService);
            
            const char* requestedType =
                obs_data_has_user_value(streamData, "type") ? obs_data_get_string(streamData, "type") : currentServiceType;
            const char* serviceType =
                service != nullptr ? obs_service_get_type(service) : currentServiceType;
            obs_data_t* settings = obs_data_get_obj(streamData, "settings");
            
            obs_data_t* metadata = obs_data_get_obj(streamData, "metadata");
            QString* query = Utils::ParseDataToQueryString(metadata);
            
            if (strcmp(requestedType, serviceType) != 0) {
                if (settings) {
                    obs_service_release(service);
                    service = nullptr; //different type so we can't reuse the existing service instance
                } else {
                    req->SendErrorResponse("Service type requested does not match currently configured type and no 'settings' were provided");
                    return;
                }
            } else {
                //if type isn't changing we should overlay the settings we got with the existing settings
                obs_data_t* existingSettings = obs_service_get_settings(currentService);
                obs_data_t* newSettings = obs_data_create(); //by doing this you can send a request to the websocket that only contains a setting you want to change instead of having to do a get and then change them
                
                obs_data_apply(newSettings, existingSettings); //first apply the existing settings
                
                obs_data_apply(newSettings, settings); //then apply the settings from the request should they exist
                obs_data_release(settings);
                
                settings = newSettings;
                obs_data_release(existingSettings);
            }
            
            if (!service){
                service = obs_service_create(requestedType, "websocket_custom_service", settings, nullptr);
            }
            
            //Supporting adding metadata parameters to key query string
            if (query && query->length() > 0) {
                const char* key = obs_data_get_string(settings, "key");
                int keylen = strlen(key);
                bool hasQuestionMark = false;
                for (int i = 0; i < keylen; i++) {
                    if (key[i] == '?') {
                        hasQuestionMark = true;
                        break;
                    }
                }
                if (hasQuestionMark) {
                    query->prepend('&');
                } else {
                    query->prepend('?');
                }
                query->prepend(key);
                key = query->toUtf8();
                obs_data_set_string(settings, "key", key);
            }
            
            obs_service_update(service, settings);
            obs_data_release(settings);
            obs_data_release(metadata);

            _service = service;
            obs_frontend_set_streaming_service(_service);
        } else if (_service != nullptr) {
            obs_service_release(_service);
            _service = nullptr;
        }
        
        obs_frontend_streaming_start();
        
        if (_service != nullptr) {
            obs_frontend_set_streaming_service(currentService);
        }
        
        req->SendOKResponse();
        obs_service_release(currentService);
    } else {
        req->SendErrorResponse("streaming already active");
    }
}

void WSRequestHandler::HandleStopStreaming(WSRequestHandler* req) {
    if (obs_frontend_streaming_active() == true) {
        obs_frontend_streaming_stop();
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("streaming not active");
    }
}

void WSRequestHandler::HandleStartRecording(WSRequestHandler* req) {
    if (obs_frontend_recording_active() == false) {
        obs_frontend_recording_start();
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("recording already active");
    }
}

void WSRequestHandler::HandleStopRecording(WSRequestHandler* req) {
    if (obs_frontend_recording_active() == true) {
        obs_frontend_recording_stop();
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("recording not active");
    }
}

void WSRequestHandler::HandleStartStopReplayBuffer(WSRequestHandler* req) {
    if (obs_frontend_replay_buffer_active())
        obs_frontend_replay_buffer_stop();
    else
        obs_frontend_replay_buffer_start();

    req->SendOKResponse();
}

void WSRequestHandler::HandleStartReplayBuffer(WSRequestHandler* req) {
    if (obs_frontend_replay_buffer_active() == false) {
        obs_frontend_replay_buffer_start();
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("replay buffer already active");
    }
}

void WSRequestHandler::HandleStopReplayBuffer(WSRequestHandler* req) {
    if (obs_frontend_replay_buffer_active() == true) {
        obs_frontend_replay_buffer_stop();
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("replay buffer not active");
    }
}

void WSRequestHandler::HandleSaveReplayBuffer(WSRequestHandler* req) {
    if (!obs_frontend_replay_buffer_active()) {
        req->SendErrorResponse("replay buffer not active");
        return;
    }

    obs_hotkey_id hk = Utils::FindHotkeyByName("ReplayBuffer.Save");
    if (hk >= 0) {
        obs_hotkey_trigger_routed_callback(hk, true);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("failed to save replay buffer");
    }
}

void WSRequestHandler::HandleGetTransitionList(WSRequestHandler* req) {
    obs_source_t* current_transition = obs_frontend_get_current_transition();
    obs_frontend_source_list transitionList = {};
    obs_frontend_get_transitions(&transitionList);

    obs_data_array_t* transitions = obs_data_array_create();
    for (size_t i = 0; i < transitionList.sources.num; i++) {
        obs_source_t* transition = transitionList.sources.array[i];

        obs_data_t* obj = obs_data_create();
        obs_data_set_string(obj, "name", obs_source_get_name(transition));

        obs_data_array_push_back(transitions, obj);
        obs_data_release(obj);
    }
    obs_frontend_source_list_free(&transitionList);

    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "current-transition",
        obs_source_get_name(current_transition));
    obs_data_set_array(response, "transitions", transitions);

    req->SendOKResponse(response);

    obs_data_release(response);
    obs_data_array_release(transitions);
    obs_source_release(current_transition);
}

void WSRequestHandler::HandleGetCurrentTransition(WSRequestHandler* req) {
    obs_source_t* current_transition = obs_frontend_get_current_transition();

    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "name",
        obs_source_get_name(current_transition));

    if (!obs_transition_fixed(current_transition))
        obs_data_set_int(response, "duration", Utils::GetTransitionDuration());

    req->SendOKResponse(response);

    obs_data_release(response);
    obs_source_release(current_transition);
}

void WSRequestHandler::HandleSetCurrentTransition(WSRequestHandler* req) {
    if (!req->hasField("transition-name")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* name = obs_data_get_string(req->data, "transition-name");
    bool success = Utils::SetTransitionByName(name);
    if (success)
        req->SendOKResponse();
    else
        req->SendErrorResponse("requested transition does not exist");
}

void WSRequestHandler::HandleSetTransitionDuration(WSRequestHandler* req) {
    if (!req->hasField("duration")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    int ms = obs_data_get_int(req->data, "duration");
    Utils::SetTransitionDuration(ms);
    req->SendOKResponse();
}

void WSRequestHandler::HandleGetTransitionDuration(WSRequestHandler* req) {
    obs_data_t* response = obs_data_create();
    obs_data_set_int(response, "transition-duration",
        Utils::GetTransitionDuration());

    req->SendOKResponse(response);
    obs_data_release(response);
}

void WSRequestHandler::HandleSetVolume(WSRequestHandler* req) {
    if (!req->hasField("source") ||
        !req->hasField("volume")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* source_name = obs_data_get_string(req->data, "source");
    float source_volume = obs_data_get_double(req->data, "volume");

    if (source_name == NULL || strlen(source_name) < 1 || 
        source_volume < 0.0 || source_volume > 1.0) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    obs_source_t* source = obs_get_source_by_name(source_name);
    if (!source) {
        req->SendErrorResponse("specified source doesn't exist");
        return;
    }

    obs_source_set_volume(source, source_volume);
    req->SendOKResponse();

    obs_source_release(source);
}

void WSRequestHandler::HandleGetVolume(WSRequestHandler* req) {
    if (!req->hasField("source")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* source_name = obs_data_get_string(req->data, "source");
    if (str_valid(source_name)) {
        obs_source_t* source = obs_get_source_by_name(source_name);

        obs_data_t* response = obs_data_create();
        obs_data_set_string(response, "name", source_name);
        obs_data_set_double(response, "volume", obs_source_get_volume(source));
        obs_data_set_bool(response, "muted", obs_source_muted(source));

        req->SendOKResponse(response);

        obs_data_release(response);
        obs_source_release(source);
    } else {
        req->SendErrorResponse("invalid request parameters");
    }
}

void WSRequestHandler::HandleToggleMute(WSRequestHandler* req) {
    if (!req->hasField("source")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* source_name = obs_data_get_string(req->data, "source");
    if (!str_valid(source_name)) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    obs_source_t* source = obs_get_source_by_name(source_name);
    if (!source) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    obs_source_set_muted(source, !obs_source_muted(source));
    req->SendOKResponse();

    obs_source_release(source);
}

void WSRequestHandler::HandleSetMute(WSRequestHandler* req) {
    if (!req->hasField("source") ||
        !req->hasField("mute")) {
        req->SendErrorResponse("mssing request parameters");
        return;
    }

    const char* source_name = obs_data_get_string(req->data, "source");
    bool mute = obs_data_get_bool(req->data, "mute");

    if (!str_valid(source_name)) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    obs_source_t* source = obs_get_source_by_name(source_name);
    if (!source) {
        req->SendErrorResponse("specified source doesn't exist");
        return;
    }

    obs_source_set_muted(source, mute);
    req->SendOKResponse();

    obs_source_release(source);
}

void WSRequestHandler::HandleGetMute(WSRequestHandler* req) {
    if (!req->hasField("source")) {
        req->SendErrorResponse("mssing request parameters");
        return;
    }

    const char* source_name = obs_data_get_string(req->data, "source");
    if (!str_valid(source_name)) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    obs_source_t* source = obs_get_source_by_name(source_name);
    if (!source) {
        req->SendErrorResponse("specified source doesn't exist");
        return;
    }

    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "name", obs_source_get_name(source));
    obs_data_set_bool(response, "muted", obs_source_muted(source));

    req->SendOKResponse(response);

    obs_source_release(source);
    obs_data_release(response);
}

void WSRequestHandler::HandleSetSceneItemPosition(WSRequestHandler* req) {
    if (!req->hasField("item") ||
        !req->hasField("x") || !req->hasField("y")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* item_name = obs_data_get_string(req->data, "item");
    if (!str_valid(item_name)) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    const char* scene_name = obs_data_get_string(req->data, "scene-name");
    obs_source_t* scene = Utils::GetSceneFromNameOrCurrent(scene_name);
    if (!scene) {
        req->SendErrorResponse("requested scene could not be found");
        return;
    }

    obs_sceneitem_t* scene_item = Utils::GetSceneItemFromName(scene, item_name);
    if (scene_item) {
        vec2 item_position = { 0 };
        item_position.x = obs_data_get_double(req->data, "x");
        item_position.y = obs_data_get_double(req->data, "y");

        obs_sceneitem_set_pos(scene_item, &item_position);

        obs_sceneitem_release(scene_item);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("specified scene item doesn't exist");
    }

    obs_source_release(scene);
}

void WSRequestHandler::HandleSetSceneItemTransform(WSRequestHandler* req) {
    if (!req->hasField("item") ||
        !req->hasField("x-scale") ||
        !req->hasField("y-scale") ||
        !req->hasField("rotation")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* item_name = obs_data_get_string(req->data, "item");
    if (!str_valid(item_name)) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    const char* scene_name = obs_data_get_string(req->data, "scene-name");
    obs_source_t* scene = Utils::GetSceneFromNameOrCurrent(scene_name);
    if (!scene) {
        req->SendErrorResponse("requested scene doesn't exist");
        return;
    }

    vec2 scale;
    scale.x = obs_data_get_double(req->data, "x-scale");
    scale.y = obs_data_get_double(req->data, "y-scale");
    float rotation = obs_data_get_double(req->data, "rotation");

    obs_sceneitem_t* scene_item = Utils::GetSceneItemFromName(scene, item_name);
    if (scene_item) {
        obs_sceneitem_set_scale(scene_item, &scale);
        obs_sceneitem_set_rot(scene_item, rotation);

        obs_sceneitem_release(scene_item);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("specified scene item doesn't exist");
    }

    obs_source_release(scene);
}

void WSRequestHandler::HandleSetSceneItemCrop(WSRequestHandler* req) {
    if (!req->hasField("item")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* item_name = obs_data_get_string(req->data, "item");
    if (!str_valid(item_name)) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    const char* scene_name = obs_data_get_string(req->data, "scene-name");
    obs_source_t* scene = Utils::GetSceneFromNameOrCurrent(scene_name);
    if (!scene) {
        req->SendErrorResponse("requested scene doesn't exist");
        return;
    }

    obs_sceneitem_t* scene_item = Utils::GetSceneItemFromName(scene, item_name);
    if (scene_item) {
        struct obs_sceneitem_crop crop = { 0 };
        crop.top = obs_data_get_int(req->data, "top");
        crop.bottom = obs_data_get_int(req->data, "bottom");;
        crop.left = obs_data_get_int(req->data, "left");;
        crop.right = obs_data_get_int(req->data, "right");

        obs_sceneitem_set_crop(scene_item, &crop);

        obs_sceneitem_release(scene_item);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("specified scene item doesn't exist");
    }

    obs_source_release(scene);
}

void WSRequestHandler::HandleSetCurrentSceneCollection(WSRequestHandler* req) {
    if (!req->hasField("sc-name")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* scene_collection = obs_data_get_string(req->data, "sc-name");
    if (str_valid(scene_collection)) {
        // TODO : Check if specified profile exists and if changing is allowed
        obs_frontend_set_current_scene_collection(scene_collection);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("invalid request parameters");
    }
}

void WSRequestHandler::HandleGetCurrentSceneCollection(WSRequestHandler* req) {
    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "sc-name",
        obs_frontend_get_current_scene_collection());

    req->SendOKResponse(response);
    obs_data_release(response);
}

void WSRequestHandler::HandleListSceneCollections(WSRequestHandler* req) {
    obs_data_array_t* scene_collections = Utils::GetSceneCollections();

    obs_data_t* response = obs_data_create();
    obs_data_set_array(response, "scene-collections", scene_collections);

    req->SendOKResponse(response);
    obs_data_release(response);
    obs_data_array_release(scene_collections);
}

void WSRequestHandler::HandleSetCurrentProfile(WSRequestHandler* req) {
    if (!req->hasField("profile-name")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* profile_name = obs_data_get_string(req->data, "profile-name");
    if (str_valid(profile_name)) {
        // TODO : check if profile exists
        obs_frontend_set_current_profile(profile_name);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("invalid request parameters");
    }
}

void WSRequestHandler::HandleGetCurrentProfile(WSRequestHandler* req) {
    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "profile-name",
        obs_frontend_get_current_profile());

    req->SendOKResponse(response);
    obs_data_release(response);
}

void WSRequestHandler::HandleSetStreamSettings(WSRequestHandler* req) {
    obs_service_t* service = obs_frontend_get_streaming_service();
    
    obs_data_t* settings = obs_data_get_obj(req->data, "settings");
    if (!settings) {
        req->SendErrorResponse("'settings' are required'");
        return;
    }
    
    const char* serviceType = obs_service_get_type(service);
    const char* requestedType = obs_data_get_string(req->data, "type");
    if (requestedType != nullptr && strcmp(requestedType, serviceType) != 0) {
        obs_data_t* hotkeys = obs_hotkeys_save_service(service);
        obs_service_release(service);
        service = obs_service_create(requestedType, "websocket_custom_service", settings, hotkeys);
        obs_data_release(hotkeys);
    } else {
        //if type isn't changing we should overlay the settings we got with the existing settings
        obs_data_t* existingSettings = obs_service_get_settings(service);
        //by doing this you can send a request to the websocket that only contains a setting you want to change instead of having to do a get and then change them
        obs_data_t* newSettings = obs_data_create(); 
        
        obs_data_apply(newSettings, existingSettings); //first apply the existing settings
        obs_data_apply(newSettings, settings); //then apply the settings from the request
        
        obs_data_release(settings);
        obs_data_release(existingSettings);
        
        obs_service_update(service, settings);
        settings = newSettings;
    }
    
    //if save is specified we should immediately save the streaming service
    if (obs_data_get_bool(req->data, "save")) {
        obs_frontend_save_streaming_service();
    }
    
    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "type", requestedType);
    obs_data_set_obj(response, "settings", settings);
    
    req->SendOKResponse(response);
    obs_data_release(settings);
    obs_data_release(response);
}

void WSRequestHandler::HandleGetStreamSettings(WSRequestHandler* req) {
    obs_service_t* service = obs_frontend_get_streaming_service();
    const char* serviceType = obs_service_get_type(service);
    obs_data_t* settings = obs_service_get_settings(service);
    
    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "type", serviceType);
    obs_data_set_obj(response, "settings", settings);
    
    req->SendOKResponse(response);
    obs_data_release(settings);
    obs_data_release(response);
}

void WSRequestHandler::HandleSaveStreamSettings(WSRequestHandler* req) {
    obs_frontend_save_streaming_service();
    req->SendOKResponse();
}

void WSRequestHandler::HandleListProfiles(WSRequestHandler* req) {
    obs_data_array_t* profiles = Utils::GetProfiles();

    obs_data_t* response = obs_data_create();
    obs_data_set_array(response, "profiles", profiles);

    req->SendOKResponse(response);
    obs_data_release(response);
    obs_data_array_release(profiles);
}

void WSRequestHandler::HandleGetStudioModeStatus(WSRequestHandler* req) {
    bool previewActive = Utils::IsPreviewModeActive();

    obs_data_t* response = obs_data_create();
    obs_data_set_bool(response, "studio-mode", previewActive);

    req->SendOKResponse(response);
    obs_data_release(response);
}

void WSRequestHandler::HandleGetPreviewScene(WSRequestHandler* req) {
    if (!Utils::IsPreviewModeActive()) {
        req->SendErrorResponse("studio mode not enabled");
        return;
    }

    obs_scene_t* preview_scene = Utils::GetPreviewScene();
    obs_source_t* source = obs_scene_get_source(preview_scene);
    const char* name = obs_source_get_name(source);

    obs_data_array_t* scene_items = Utils::GetSceneItems(source);

    obs_data_t* data = obs_data_create();
    obs_data_set_string(data, "name", name);
    obs_data_set_array(data, "sources", scene_items);

    req->SendOKResponse(data);
    obs_data_release(data);
    obs_data_array_release(scene_items);
    obs_scene_release(preview_scene);
}

void WSRequestHandler::HandleSetPreviewScene(WSRequestHandler* req) {
    if (!Utils::IsPreviewModeActive()) {
        req->SendErrorResponse("studio mode not enabled");
        return;
    }

    if (!req->hasField("scene-name")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* scene_name = obs_data_get_string(req->data, "scene-name");
    bool success = Utils::SetPreviewScene(scene_name);
    if (success)
        req->SendOKResponse();
    else
        req->SendErrorResponse("specified scene doesn't exist");
}

void WSRequestHandler::HandleTransitionToProgram(WSRequestHandler* req) {
    if (!Utils::IsPreviewModeActive()) {
        req->SendErrorResponse("studio mode not enabled");
        return;
    }

    if (req->hasField("with-transition")) {
        obs_data_t* transitionInfo =
            obs_data_get_obj(req->data, "with-transition");

        if (obs_data_has_user_value(transitionInfo, "name")) {
            const char* transitionName =
                obs_data_get_string(transitionInfo, "name");
            if (!str_valid(transitionName)) {
                req->SendErrorResponse("invalid request parameters");
                return;
            }

            bool success = Utils::SetTransitionByName(transitionName);
            if (!success) {
                req->SendErrorResponse("specified transition doesn't exist");
                obs_data_release(transitionInfo);
                return;
            }
        }

        if (obs_data_has_user_value(transitionInfo, "duration")) {
            int transitionDuration =
                obs_data_get_int(transitionInfo, "duration");
            Utils::SetTransitionDuration(transitionDuration);
        }

        obs_data_release(transitionInfo);
    }

    Utils::TransitionToProgram();
    req->SendOKResponse();
}

void WSRequestHandler::HandleEnableStudioMode(WSRequestHandler* req) {
    Utils::EnablePreviewMode();
    req->SendOKResponse();
}

void WSRequestHandler::HandleDisableStudioMode(WSRequestHandler* req) {
    Utils::DisablePreviewMode();
    req->SendOKResponse();
}

void WSRequestHandler::HandleToggleStudioMode(WSRequestHandler* req) {
    Utils::TogglePreviewMode();
    req->SendOKResponse();
}

void WSRequestHandler::HandleGetSpecialSources(WSRequestHandler* req) {
    obs_data_t* response = obs_data_create();

    QMap<const char*, int> sources;
    sources["desktop-1"] = 1;
    sources["desktop-2"] = 2;
    sources["mic-1"] = 3;
    sources["mic-2"] = 4;
    sources["mic-3"] = 5;

    QMapIterator<const char*, int> i(sources);
    while (i.hasNext()) {
        i.next();

        const char* id = i.key();
        obs_source_t* source = obs_get_output_source(i.value());
        blog(LOG_INFO, "%s : %p", id, source);

        if (source) {
            obs_data_set_string(response, id, obs_source_get_name(source));
            obs_source_release(source);
        }
    }

    req->SendOKResponse(response);
    obs_data_release(response);
}

void WSRequestHandler::HandleSetRecordingFolder(WSRequestHandler* req) {
    if (!req->hasField("rec-folder")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* newRecFolder = obs_data_get_string(req->data, "rec-folder");
    bool success = Utils::SetRecordingFolder(newRecFolder);
    if (success)
        req->SendOKResponse();
    else
        req->SendErrorResponse("invalid request parameters");
}

void WSRequestHandler::HandleGetRecordingFolder(WSRequestHandler* req) {
    const char* recFolder = Utils::GetRecordingFolder();

    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "rec-folder", recFolder);

    req->SendOKResponse(response);
    obs_data_release(response);
}

void WSRequestHandler::HandleGetTextGDIPlusProperties(WSRequestHandler* req) {
    const char* itemName = obs_data_get_string(req->data, "source");
    if (!itemName) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    const char* sceneName = obs_data_get_string(req->data, "scene-name");
    obs_source_t* scene = Utils::GetSceneFromNameOrCurrent(sceneName);
    if (!scene) {
        req->SendErrorResponse("requested scene doesn't exist");
        return;
    }

    obs_sceneitem_t* sceneItem = Utils::GetSceneItemFromName(scene, itemName);
    if (sceneItem) {
        obs_source_t* sceneItemSource = obs_sceneitem_get_source(sceneItem);
        const char* sceneItemSourceId = obs_source_get_id(sceneItemSource);

        if (strcmp(sceneItemSourceId, "text_gdiplus") == 0) {
            obs_data_t* response = obs_source_get_settings(sceneItemSource);
            obs_data_set_string(response, "source", itemName);
            obs_data_set_string(response, "scene-name", sceneName);
            obs_data_set_bool(response, "render",
                obs_sceneitem_visible(sceneItem));

            req->SendOKResponse(response);

            obs_data_release(response);
            obs_sceneitem_release(sceneItem);
        } else {
            req->SendErrorResponse("not text gdi plus source");
        }
    } else {
        req->SendErrorResponse("specified scene item doesn't exist");
    }

    obs_source_release(scene);
}

void WSRequestHandler::HandleSetTextGDIPlusProperties(WSRequestHandler* req) {
    if (!req->hasField("source")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* itemName = obs_data_get_string(req->data, "source");
    if (!itemName) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    const char* sceneName = obs_data_get_string(req->data, "scene-name");
    obs_source_t* scene = Utils::GetSceneFromNameOrCurrent(sceneName);
    if (!scene) {
        req->SendErrorResponse("requested scene doesn't exist");
        return;
    }

    obs_sceneitem_t* sceneItem = Utils::GetSceneItemFromName(scene, itemName);
    if (sceneItem) {
        obs_source_t* sceneItemSource = obs_sceneitem_get_source(sceneItem);
        const char* sceneItemSourceId = obs_source_get_id(sceneItemSource);

        if (strcmp(sceneItemSourceId, "text_gdiplus") == 0) {
            obs_data_t* settings = obs_source_get_settings(sceneItemSource);

            if (req->hasField("align")) {
                obs_data_set_string(settings, "align", 
                    obs_data_get_string(req->data, "align"));
            }

            if (req->hasField("bk_color")) {
                obs_data_set_int(settings, "bk_color", 
                    obs_data_get_int(req->data, "bk_color"));
            }

            if (req->hasField("bk-opacity")) {
                obs_data_set_int(settings, "bk_opacity", 
                    obs_data_get_int(req->data, "bk_opacity"));
            }

            if (req->hasField("chatlog")) {
                obs_data_set_bool(settings, "chatlog", 
                    obs_data_get_bool(req->data, "chatlog"));
            }
            
            if (req->hasField("chatlog_lines")) {
                obs_data_set_int(settings, "chatlog_lines",
                    obs_data_get_int(req->data, "chatlog_lines"));
            }

            if (req->hasField("color")) {
                obs_data_set_int(settings, "color", 
                    obs_data_get_int(req->data, "color"));
            }

            if (req->hasField("extents")) {
                obs_data_set_bool(settings, "extents", 
                    obs_data_get_bool(req->data, "extents"));
            }

            if (req->hasField("extents_wrap")) {
                obs_data_set_bool(settings, "extents_wrap",
                    obs_data_get_bool(req->data, "extents_wrap"));
            }

            if (req->hasField("extents_cx")) {
                obs_data_set_int(settings, "extents_cx",
                    obs_data_get_int(req->data, "extents_cx"));
            }

            if (req->hasField("extents_cy")) {
                obs_data_set_int(settings, "extents_cy",
                    obs_data_get_int(req->data, "extents_cy"));
            }

            if (req->hasField("file")) {
                obs_data_set_string(settings, "file", 
                    obs_data_get_string(req->data, "file"));
            }

            if (req->hasField("font")) {
                obs_data_t* font_obj = obs_data_get_obj(settings, "font");
                if (font_obj) {
                    obs_data_t* req_font_obj = obs_data_get_obj(req->data, "font");

                    if (obs_data_has_user_value(req_font_obj, "face")) {
                        obs_data_set_string(font_obj, "face",
                            obs_data_get_string(req_font_obj, "face"));
                    }

                    if (obs_data_has_user_value(req_font_obj, "flags")) {
                        obs_data_set_int(font_obj, "flags",
                            obs_data_get_int(req_font_obj, "flags"));
                    }

                    if (obs_data_has_user_value(req_font_obj, "size")) {
                        obs_data_set_int(font_obj, "size", 
                            obs_data_get_int(req_font_obj, "size"));
                    }

                    if (obs_data_has_user_value(req_font_obj, "style")) {
                        obs_data_set_string(font_obj, "style",
                            obs_data_get_string(req_font_obj, "style"));
                    }

                    obs_data_release(req_font_obj);
                    obs_data_release(font_obj);
                }
            }

            if (req->hasField("gradient")) {
                obs_data_set_bool(settings, "gradient", 
                    obs_data_get_bool(req->data, "gradient"));
            }

            if (req->hasField("gradient_color")) {
                obs_data_set_int(settings, "gradient_color",
                    obs_data_get_int(req->data, "gradient_color"));
            }

            if (req->hasField("gradient_dir")) {
                obs_data_set_double(settings, "gradient_dir",
                    obs_data_get_double(req->data, "gradient_dir"));
            }

            if (req->hasField("gradient_opacity")) {
                obs_data_set_int(settings, "gradient_opacity",
                    obs_data_get_int(req->data, "gradient_opacity"));
            }

            if (req->hasField("outline")) {
                obs_data_set_bool(settings, "outline", 
                    obs_data_get_bool(req->data, "outline"));
            }

            if (req->hasField("outline_size")) {
                obs_data_set_int(settings, "outline_size",
                    obs_data_get_int(req->data, "outline_size"));
            }

            if (req->hasField("outline_color")) {
                obs_data_set_int(settings, "outline_color",
                    obs_data_get_int(req->data, "outline_color"));
            }

            if (req->hasField("outline_opacity")) {
                obs_data_set_int(settings, "outline_opacity", 
                    obs_data_get_int(req->data, "outline_opacity"));
            }

            if (req->hasField("read_from_file")) {
                obs_data_set_bool(settings, "read_from_file", 
                    obs_data_get_bool(req->data, "read_from_file"));
            }

            if (req->hasField("text")) {
                obs_data_set_string(settings, "text", 
                    obs_data_get_string(req->data, "text"));
            }

            if (req->hasField("valign")) {
                obs_data_set_string(settings, "valign", 
                    obs_data_get_string(req->data, "valign"));
            }

            if (req->hasField("vertical")) {
                obs_data_set_bool(settings, "vertical", 
                    obs_data_get_bool(req->data, "vertical"));
            }

            obs_source_update(sceneItemSource, settings);

            if (req->hasField("render")) {
                obs_sceneitem_set_visible(sceneItem, 
                    obs_data_get_bool(req->data, "render"));
            }

            req->SendOKResponse();
            obs_data_release(settings);
            obs_sceneitem_release(sceneItem);
        } else {
            req->SendErrorResponse("not text gdi plus source");
        }
    } else {
        req->SendErrorResponse("specified scene item doesn't exist");
    }
    
    obs_source_release(scene);
}

void WSRequestHandler::HandleGetBrowserSourceProperties(WSRequestHandler* req) {
    const char* itemName = obs_data_get_string(req->data, "source");
    if (!itemName) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    const char* sceneName = obs_data_get_string(req->data, "scene-name");
    obs_source_t* scene = Utils::GetSceneFromNameOrCurrent(sceneName);
    if (!scene) {
        req->SendErrorResponse("requested scene doesn't exist");
        return;
    }

    obs_sceneitem_t* sceneItem = Utils::GetSceneItemFromName(scene, itemName);
    if (sceneItem) {
        obs_source_t* sceneItemSource = obs_sceneitem_get_source(sceneItem);
        const char* sceneItemSourceId = obs_source_get_id(sceneItemSource);

        if (strcmp(sceneItemSourceId, "browser_source") == 0) {
            obs_data_t* response = obs_source_get_settings(sceneItemSource);
            obs_data_set_string(response, "source", itemName);
            obs_data_set_string(response, "scene-name", sceneName);
            obs_data_set_bool(response, "render",
                obs_sceneitem_visible(sceneItem));

            req->SendOKResponse(response);

            obs_data_release(response);
            obs_sceneitem_release(sceneItem);
        } else {
            req->SendErrorResponse("not browser source");
        }
    } else {
        req->SendErrorResponse("specified scene item doesn't exist");
    }
    obs_source_release(scene);
}

void WSRequestHandler::HandleSetBrowserSourceProperties(WSRequestHandler* req) {
    if (!req->hasField("source")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* itemName = obs_data_get_string(req->data, "source");
    if (!itemName) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    const char* sceneName = obs_data_get_string(req->data, "scene-name");
    obs_source_t* scene = Utils::GetSceneFromNameOrCurrent(sceneName);
    if (!scene) {
        req->SendErrorResponse("requested scene doesn't exist");
        return;
    }

    obs_sceneitem_t* sceneItem = Utils::GetSceneItemFromName(scene, itemName);
    if (sceneItem) {
        obs_source_t* sceneItemSource = obs_sceneitem_get_source(sceneItem);
        const char* sceneItemSourceId = obs_source_get_id(sceneItemSource);

        if (strcmp(sceneItemSourceId, "browser_source") == 0) {
            obs_data_t* settings = obs_source_get_settings(sceneItemSource);

            if (req->hasField("restart_when_active")) {
                obs_data_set_bool(settings, "restart_when_active",
                    obs_data_get_bool(req->data, "restart_when_active"));
            }

            if (req->hasField("shutdown")) {
                obs_data_set_bool(settings, "shutdown",
                    obs_data_get_bool(req->data, "shutdown"));
            }

            if (req->hasField("is_local_file")) {
                obs_data_set_bool(settings, "is_local_file",
                    obs_data_get_bool(req->data, "is_local_file"));
            }

            if (req->hasField("url")) {
                obs_data_set_string(settings, "url",
                    obs_data_get_string(req->data, "url"));
            }

            if (req->hasField("css")) {
                obs_data_set_string(settings, "css",
                    obs_data_get_string(req->data, "css"));
            }

            if (req->hasField("width")) {
                obs_data_set_int(settings, "width",
                    obs_data_get_int(req->data, "width"));
            }

            if (req->hasField("height")) {
                obs_data_set_int(settings, "height",
                    obs_data_get_int(req->data, "height"));
            }
            
            if (req->hasField("fps")) {
                obs_data_set_int(settings, "fps",
                    obs_data_get_int(req->data, "fps"));
            }

            obs_source_update(sceneItemSource, settings);

            if (req->hasField("render")) {
                obs_sceneitem_set_visible(sceneItem,
                    obs_data_get_bool(req->data, "render"));
            }

            req->SendOKResponse();

            obs_data_release(settings);
            obs_sceneitem_release(sceneItem);
        } else {
            req->SendErrorResponse("not browser source");
        }
    } else {
        req->SendErrorResponse("specified scene item doesn't exist");
    }
    obs_source_release(scene);
}

void WSRequestHandler::HandleResetSceneItem(WSRequestHandler* req) {
    if (!req->hasField("item")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* itemName = obs_data_get_string(req->data, "item");
    if (!itemName) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    const char* sceneName = obs_data_get_string(req->data, "scene-name");
    obs_source_t* scene = Utils::GetSceneFromNameOrCurrent(sceneName);
    if (!scene) {
        req->SendErrorResponse("requested scene doesn't exist");
        return;
    }

    obs_sceneitem_t* sceneItem = Utils::GetSceneItemFromName(scene, itemName);
    if (sceneItem) {
        obs_source_t* sceneItemSource = obs_sceneitem_get_source(sceneItem);

        obs_data_t* settings = obs_source_get_settings(sceneItemSource);
        obs_source_update(sceneItemSource, settings);

        obs_sceneitem_release(sceneItem);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("specified scene item doesn't exist");
    }

    obs_source_release(scene);
}
