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

#include <QList>
#include <QObject>
#include <QString>

#include "WSEvents.h"
#include "obs-websocket.h"
#include "Config.h"
#include "Utils.h"

#include "WSRequestHandler.h"

bool str_valid(const char* str) {
    return (str != nullptr && strlen(str) > 0);
}

WSRequestHandler::WSRequestHandler(QWebSocket* client) :
    _messageId(0),
    _requestType(""),
    data(nullptr),
    _client(client)
{
    messageMap["GetVersion"] = WSRequestHandler::HandleGetVersion;
    messageMap["GetAuthRequired"] = WSRequestHandler::HandleGetAuthRequired;
    messageMap["Authenticate"] = WSRequestHandler::HandleAuthenticate;

    messageMap["SetHeartbeat"] = WSRequestHandler::HandleSetHeartbeat;

    messageMap["SetCurrentScene"] = WSRequestHandler::HandleSetCurrentScene;
    messageMap["GetCurrentScene"] = WSRequestHandler::HandleGetCurrentScene;
    messageMap["GetSceneList"] = WSRequestHandler::HandleGetSceneList;

    messageMap["SetSourceRender"] = WSRequestHandler::HandleSetSceneItemRender; // Retrocompat
    messageMap["SetSceneItemRender"] = WSRequestHandler::HandleSetSceneItemRender;
    messageMap["SetSceneItemPosition"] = WSRequestHandler::HandleSetSceneItemPosition;
    messageMap["SetSceneItemTransform"] = WSRequestHandler::HandleSetSceneItemTransform;
    messageMap["SetSceneItemCrop"] = WSRequestHandler::HandleSetSceneItemCrop;
    messageMap["GetSceneItemProperties"] = WSRequestHandler::HandleGetSceneItemProperties;
    messageMap["SetSceneItemProperties"] = WSRequestHandler::HandleSetSceneItemProperties;
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
    messageMap["SetSyncOffset"] = WSRequestHandler::HandleSetSyncOffset;
    messageMap["GetSyncOffset"] = WSRequestHandler::HandleGetSyncOffset;
    messageMap["GetSpecialSources"] = WSRequestHandler::HandleGetSpecialSources;
    messageMap["GetSourcesList"] = WSRequestHandler::HandleGetSourcesList;
    messageMap["GetSourceSettings"] = WSRequestHandler::HandleGetSourceSettings;
    messageMap["SetSourceSettings"] = WSRequestHandler::HandleSetSourceSettings;

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

void WSRequestHandler::SendErrorResponse(obs_data_t* additionalFields) {
    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "status", "error");
    obs_data_set_string(response, "message-id", _messageId);

    if (additionalFields)
        obs_data_set_obj(response, "error", additionalFields);

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

/**
 * Returns the latest version of the plugin and the API.
 * 
 * @return {double} `version` OBSRemote compatible API version. Fixed to 1.1 for retrocompatibility.
 * @return {String} `obs-websocket-version` obs-websocket plugin version.
 * @return {String} `obs-studio-version` OBS Studio program version.
 * @return {String|Array} `available-requests` List of available request types.
 * 
 * @api requests
 * @name GetVersion
 * @category general
 * @since 0.3
 */
void WSRequestHandler::HandleGetVersion(WSRequestHandler* req) {
    const char* obs_version = Utils::OBSVersionString();

    // (Palakis) OBS' data arrays only support object arrays, so I improvised.
    QList<QString> names = req->messageMap.keys();
    names.sort(Qt::CaseInsensitive);
    QString requests;
    requests += names.takeFirst();
    for (QString reqName : names) {
        requests += ("," + reqName);
    }

    obs_data_t* data = obs_data_create();
    obs_data_set_string(data, "obs-websocket-version", OBS_WEBSOCKET_VERSION);
    obs_data_set_string(data, "obs-studio-version", obs_version);
    obs_data_set_string(data, "available-requests", requests.toUtf8().constData());

    req->SendOKResponse(data);
    obs_data_release(data);
    bfree((void*)obs_version);
}

/**
 * Tells the client if authentication is required. If so, returns authentication parameters `challenge`
 * and `salt` (see "Authentication" for more information).
 * 
 * @return {boolean} `authRequired` Indicates whether authentication is required.
 * @return {String (optional)} `challenge`
 * @return {String (optional)} `salt`
 * 
 * @api requests
 * @name GetAuthRequired
 * @category general
 * @since 0.3
 */
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

/**
 * Attempt to authenticate the client to the server.
 * 
 * @param {String} `auth` Response to the auth challenge (see "Authentication" for more information).
 *
 * @api requests
 * @name Authenticate
 * @category general
 * @since 0.3
 */
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

/**
 * Enable/disable sending of the Heartbeat event
 *
 * @param {boolean} `enable` Starts/Stops emitting heartbeat messages
 *
 * @api requests
 * @name SetHeartbeat
 * @category general
 */
void WSRequestHandler::HandleSetHeartbeat(WSRequestHandler* req) {
    if (!req->hasField("enable")) {
        req->SendErrorResponse("Heartbeat <enable> parameter missing");
        return;
    }

    WSEvents::Instance->Heartbeat_active =
        obs_data_get_bool(req->data, "enable");

    obs_data_t* response = obs_data_create();
    obs_data_set_bool(response, "enable",
        WSEvents::Instance->Heartbeat_active);
    req->SendOKResponse(response);

    obs_data_release(response);
}

/**
 * Switch to the specified scene.
 *
 * @param {String} `scene-name` Name of the scene to switch to.
 *
 * @api requests
 * @name SetCurrentScene
 * @category scenes
 * @since 0.3
 */
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

/**
 * Get the current scene's name and source items.
 * 
 * @return {String} `name` Name of the currently active scene.
 * @return {Source|Array} `sources` Ordered list of the current scene's source items.
 *
 * @api requests
 * @name GetCurrentScene
 * @category scenes
 * @since 0.3
 */
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

/**
 * Get a list of scenes in the currently active profile.
 * 
 * @return {String} `current-scene` Name of the currently active scene.
 * @return {Scene|Array} `scenes` Ordered list of the current profile's scenes (See `[GetCurrentScene](#getcurrentscene)` for more information).
 *
 * @api requests
 * @name GetSceneList
 * @category scenes
 * @since 0.3
 */
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

 /**
 * Show or hide a specified source item in a specified scene.
 * 
 * @param {String} `source` Name of the source in the specified scene.
 * @param {boolean} `render` Desired visibility.
 * @param {String (optional)} `scene-name` Name of the scene where the source resides. Defaults to the currently active scene.
 *
 * @api requests
 * @name SetSourceRender
 * @category sources
 * @since 0.3
 * @deprecated Since unreleased. Prefer the use of SetSceneItemProperties.
 */
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

 /**
 * Get current streaming and recording status.
 * 
 * @return {boolean} `streaming` Current streaming status.
 * @return {boolean} `recording` Current recording status.
 * @return {String (optional)} `stream-timecode` Time elapsed since streaming started (only present if currently streaming).
 * @return {String (optional)} `rec-timecode` Time elapsed since recording started (only present if currently recording).
 * @return {boolean} `preview-only` Always false. Retrocompatibility with OBSRemote.
 *
 * @api requests
 * @name GetStreamingStatus
 * @category streaming
 * @since 0.3
 */
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

/**
 * Toggle streaming on or off.
 *
 * @api requests
 * @name StartStopStreaming
 * @category streaming
 * @since 0.3
 */
void WSRequestHandler::HandleStartStopStreaming(WSRequestHandler* req) {
    if (obs_frontend_streaming_active())
        HandleStopStreaming(req);
    else
        HandleStartStreaming(req);
}

/**
 * Toggle recording on or off.
 *
 * @api requests
 * @name StartStopRecording
 * @category recording
 * @since 0.3
 */
void WSRequestHandler::HandleStartStopRecording(WSRequestHandler* req)
{
    if (obs_frontend_recording_active())
        obs_frontend_recording_stop();
    else
        obs_frontend_recording_start();

    req->SendOKResponse();
}

/**
 * Start streaming.
 * Will return an `error` if streaming is already active.
 *
 * @param {Object (optional)} `stream` Special stream configuration.
 * @param {String (optional)} `stream.type` If specified ensures the type of stream matches the given type (usually 'rtmp_custom' or 'rtmp_common'). If the currently configured stream type does not match the given stream type, all settings must be specified in the `settings` object or an error will occur when starting the stream.
 * @param {Object (optional)} `stream.metadata` Adds the given object parameters as encoded query string parameters to the 'key' of the RTMP stream. Used to pass data to the RTMP service about the streaming. May be any String, Numeric, or Boolean field. 
 * @param {Object (optional)} `stream.settings` Settings for the stream.
 * @param {String (optional)} `stream.settings.server` The publish URL.
 * @param {String (optional)} `stream.settings.key` The publish key of the stream.
 * @param {boolean (optional)} `stream.settings.use-auth` Indicates whether authentication should be used when connecting to the streaming server.
 * @param {String (optional)} `stream.settings.username` If authentication is enabled, the username for the streaming server. Ignored if `use-auth` is not set to `true`.
 * @param {String (optional)} `stream.settings.password` If authentication is enabled, the password for the streaming server. Ignored if `use-auth` is not set to `true`.
 *
 * @api requests
 * @name StartStreaming
 * @category streaming
 * @since 4.1.0
 */
void WSRequestHandler::HandleStartStreaming(WSRequestHandler* req)
{
    if (obs_frontend_streaming_active() == false) {
        obs_service_t* currentService = obs_frontend_get_streaming_service();
        // get_streaming_service doesn't addref, so let's do it ourselves
        obs_service_addref(currentService);

        if (req->hasField("stream")) {
            obs_data_t* streamData = obs_data_get_obj(req->data, "stream");
            obs_data_t* newSettings = obs_data_get_obj(streamData, "settings");
            obs_data_t* newMetadata = obs_data_get_obj(streamData, "metadata");

            QString currentType = obs_service_get_type(currentService);
            QString newType = obs_data_get_string(streamData, "type");
            if (newType.isEmpty() || newType.isNull()) {
                newType = currentType;
            }

            //Supporting adding metadata parameters to key query string
            QString query = Utils::ParseDataToQueryString(newMetadata);
            if (!query.isEmpty()
                    && obs_data_has_user_value(newSettings, "key"))
            {
                const char* key = obs_data_get_string(newSettings, "key");
                int keylen = strlen(key);

                bool hasQuestionMark = false;
                for (int i = 0; i < keylen; i++) {
                    if (key[i] == '?') {
                        hasQuestionMark = true;
                        break;
                    }
                }

                if (hasQuestionMark) {
                    query.prepend('&');
                } else {
                    query.prepend('?');
                }

                query.prepend(key);
                obs_data_set_string(newSettings, "key", query.toUtf8());
            }

            if (newType == currentType) {
                // Service type doesn't change: apply settings to current service
                obs_data_t* currentSettings = obs_service_get_settings(currentService);
                obs_data_t* updatedSettings = obs_data_create(); //by doing this you can send a request to the websocket that only contains a setting you want to change instead of having to do a get and then change them

                obs_data_apply(updatedSettings, currentSettings); //first apply the existing settings
                obs_data_apply(updatedSettings, newSettings); //then apply the settings from the request should they exist

                obs_service_update(currentService, updatedSettings);

                obs_data_release(updatedSettings);
                obs_data_release(currentSettings);
            }
            else {
                // Service type changed: create new service
                obs_data_t* hotkeys =
                    obs_hotkeys_save_service(currentService);

                obs_service_t* newService = obs_service_create(
                    newType.toUtf8(), "websocket_custom_service",
                    newSettings, hotkeys);

                obs_frontend_set_streaming_service(newService);

                obs_data_release(hotkeys);
            }

            obs_data_release(newMetadata);
            obs_data_release(newSettings);
            obs_data_release(streamData);
        }

        obs_frontend_streaming_start();
        req->SendOKResponse();

        obs_service_release(currentService);
    } else {
        req->SendErrorResponse("streaming already active");
    }
}

/**
 * Stop streaming.
 * Will return an `error` if streaming is not active.
 *
 * @api requests
 * @name StopStreaming
 * @category streaming
 * @since 4.1.0
 */
void WSRequestHandler::HandleStopStreaming(WSRequestHandler* req) {
    if (obs_frontend_streaming_active() == true) {
        obs_frontend_streaming_stop();
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("streaming not active");
    }
}

/**
 * Start recording.
 * Will return an `error` if recording is already active.
 *
 * @api requests
 * @name StartRecording
 * @category recording
 * @since 4.1.0
 */
void WSRequestHandler::HandleStartRecording(WSRequestHandler* req) {
    if (obs_frontend_recording_active() == false) {
        obs_frontend_recording_start();
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("recording already active");
    }
}

/**
 * Stop recording.
 * Will return an `error` if recording is not active.
 *
 * @api requests
 * @name StopRecording
 * @category recording
 * @since 4.1.0
 */
void WSRequestHandler::HandleStopRecording(WSRequestHandler* req) {
    if (obs_frontend_recording_active() == true) {
        obs_frontend_recording_stop();
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("recording not active");
    }
}

/**
* Toggle the Replay Buffer on/off.
*
* @api requests
* @name StartStopReplayBuffer
* @category replay buffer
* @since 4.2.0
*/
void WSRequestHandler::HandleStartStopReplayBuffer(WSRequestHandler* req) {
    if (obs_frontend_replay_buffer_active()) {
        obs_frontend_replay_buffer_stop();
    } else {
        Utils::StartReplayBuffer();
    }
    req->SendOKResponse();
}

/**
* Start recording into the Replay Buffer.
* Will return an `error` if the Replay Buffer is already active or if the
* "Save Replay Buffer" hotkey is not set in OBS' settings.
* Setting this hotkey is mandatory, even when triggering saves only
* through obs-websocket.
*
* @api requests
* @name StartReplayBuffer
* @category replay buffer
* @since 4.2.0
*/
void WSRequestHandler::HandleStartReplayBuffer(WSRequestHandler* req) {
    if (!Utils::ReplayBufferEnabled()) {
        req->SendErrorResponse("replay buffer disabled in settings");
        return;
    }

    if (obs_frontend_replay_buffer_active() == true) {
        req->SendErrorResponse("replay buffer already active");
        return;
    }

    Utils::StartReplayBuffer();
    req->SendOKResponse();
}

/**
* Stop recording into the Replay Buffer.
* Will return an `error` if the Replay Buffer is not active.
*
* @api requests
* @name StopReplayBuffer
* @category replay buffer
* @since 4.2.0
*/
void WSRequestHandler::HandleStopReplayBuffer(WSRequestHandler* req) {
    if (obs_frontend_replay_buffer_active() == true) {
        obs_frontend_replay_buffer_stop();
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("replay buffer not active");
    }
}

/**
* Save and flush the contents of the Replay Buffer to disk. This is
* basically the same as triggering the "Save Replay Buffer" hotkey.
* Will return an `error` if the Replay Buffer is not active.
*
* @api requests
* @name SaveReplayBuffer
* @category replay buffer
* @since 4.2.0
*/
void WSRequestHandler::HandleSaveReplayBuffer(WSRequestHandler* req) {
    if (!obs_frontend_replay_buffer_active()) {
        req->SendErrorResponse("replay buffer not active");
        return;
    }

    calldata_t cd = {0};
    obs_output_t* replay_output = obs_frontend_get_replay_buffer_output();
    proc_handler_t* ph = obs_output_get_proc_handler(replay_output);
    proc_handler_call(ph, "save", &cd);

    req->SendOKResponse();

    calldata_free(&cd);
    obs_output_release(replay_output);
}

/**
 * List of all transitions available in the frontend's dropdown menu.
 *
 * @return {String} `current-transition` Name of the currently active transition.
 * @return {Object|Array} `transitions` List of transitions.
 * @return {String} `transitions[].name` Name of the transition.
 *
 * @api requests
 * @name GetTransitionList
 * @category transitions
 * @since 4.1.0
 */
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

/**
 * Get the name of the currently selected transition in the frontend's dropdown menu.
 *
 * @return {String} `name` Name of the selected transition.
 * @return {int (optional)} `duration` Transition duration (in milliseconds) if supported by the transition.
 *
 * @api requests
 * @name GetCurrentTransition
 * @category transitions
 * @since 0.3
 */
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

/**
 * Set the active transition.
 *
 * @param {String} `transition-name` The name of the transition.
 *
 * @api requests
 * @name SetCurrentTransition
 * @category transitions
 * @since 0.3
 */
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

/**
 * Set the duration of the currently selected transition if supported.
 *
 * @param {int} `duration` Desired duration of the transition (in milliseconds).
 *
 * @api requests
 * @name SetTransitionDuration
 * @category transitions
 * @since 4.0.0
 */
void WSRequestHandler::HandleSetTransitionDuration(WSRequestHandler* req) {
    if (!req->hasField("duration")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    int ms = obs_data_get_int(req->data, "duration");
    Utils::SetTransitionDuration(ms);
    req->SendOKResponse();
}

/**
 * Get the duration of the currently selected transition if supported.
 *
 * @return {int} `transition-duration` Duration of the current transition (in milliseconds).
 *
 * @api requests
 * @name GetTransitionDuration
 * @category transitions
 * @since 4.1.0
 */
void WSRequestHandler::HandleGetTransitionDuration(WSRequestHandler* req) {
    obs_data_t* response = obs_data_create();
    obs_data_set_int(response, "transition-duration",
        Utils::GetTransitionDuration());

    req->SendOKResponse(response);
    obs_data_release(response);
}

/**
 * Set the volume of the specified source.
 *
 * @param {String} `source` Name of the source.
 * @param {double} `volume` Desired volume. Must be between `0.0` and `1.0`.
 *
 * @api requests
 * @name SetVolume
 * @category sources
 * @since 4.0.0
 */
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

/**
 * Get the volume of the specified source.
 *
 * @param {String} `source` Name of the source.
 *
 * @return {String} `name` Name of the source.
 * @return {double} `volume` Volume of the source. Between `0.0` and `1.0`.
 * @return {boolean} `mute` Indicates whether the source is muted.
 *
 * @api requests
 * @name GetVolume
 * @category sources
 * @since 4.0.0
 */
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

/**
 * Inverts the mute status of a specified source.
 *
 * @param {String} `source` The name of the source.
 *
 * @api requests
 * @name ToggleMute
 * @category sources
 * @since 4.0.0
 */
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

/**
 * Sets the mute status of a specified source.
 *
 * @param {String} `source` The name of the source.
 * @param {boolean} `mute` Desired mute status.
 *
 * @api requests
 * @name SetMute
 * @category sources
 * @since 4.0.0
 */
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

/**
 * Get the mute status of a specified source.
 *
 * @param {String} `source` The name of the source.
 *
 * @return {String} `name` The name of the source.
 * @return {boolean} `muted` Mute status of the source.
 *
 * @api requests
 * @name GetMute
 * @category sources
 * @since 4.0.0
 */
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

/**
 * Set the audio sync offset of a specified source.
 * 
 * @param {String} `source` The name of the source.
 * @param {int} `offset` The desired audio sync offset (in nanoseconds).
 * 
 * @api requests
 * @name SetSyncOffset
 * @category sources
 * @since 4.2.0
 */
void WSRequestHandler::HandleSetSyncOffset(WSRequestHandler* req) {
    if (!req->hasField("source") || !req->hasField("offset")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* source_name = obs_data_get_string(req->data, "source");
    int64_t source_sync_offset = (int64_t)obs_data_get_int(req->data, "offset");

    if (!source_name || strlen(source_name) < 1 || source_sync_offset < 0) {
        req->SendErrorResponse("invalid request parameters");
        return;
    }

    obs_source_t* source = obs_get_source_by_name(source_name);
    if (!source) {
        req->SendErrorResponse("specified source doesn't exist");
        return;
    }

    obs_source_set_sync_offset(source, source_sync_offset);
    req->SendOKResponse();

    obs_source_release(source);
}

/**
 * Get the audio sync offset of a specified source.
 * 
 * @param {String} `source` The name of the source.
 * 
 * @return {String} `name` The name of the source.
 * @return {int} `offset` The audio sync offset (in nanoseconds).
 * 
 * @api requests
 * @name GetSyncOffset
 * @category sources
 * @since 4.2.0
 */
void WSRequestHandler::HandleGetSyncOffset(WSRequestHandler* req) {
    if (!req->hasField("source")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* source_name = obs_data_get_string(req->data, "source");
    if (str_valid(source_name)) {
        obs_source_t* source = obs_get_source_by_name(source_name);

        obs_data_t* response = obs_data_create();
        obs_data_set_string(response, "name", source_name);
        obs_data_set_int(response, "offset", obs_source_get_sync_offset(source));

        req->SendOKResponse(response);

        obs_data_release(response);
        obs_source_release(source);
    } else {
        req->SendErrorResponse("invalid request parameters");
    }
}

/**
 * Sets the coordinates of a specified source item.
 *
 * @param {String (optional)} `scene-name` The name of the scene that the source item belongs to. Defaults to the current scene.
 * @param {String} `item` The name of the source item.
 * @param {double} `x` X coordinate.
 * @param {double} `y` Y coordinate.
 
 *
 * @api requests
 * @name SetSceneItemPosition
 * @category sources
 * @since 4.0.0
 * @deprecated Since unreleased. Prefer the use of SetSceneItemProperties.
 */
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

/**
 * Set the transform of the specified source item.
 *
 * @param {String (optional)} `scene-name` The name of the scene that the source item belongs to. Defaults to the current scene.
 * @param {String} `item` The name of the source item.
 * @param {double} `x-scale` Width scale factor.
 * @param {double} `y-scale` Height scale factor.
 * @param {double} `rotation` Source item rotation (in degrees). 
 *
 * @api requests
 * @name SetSceneItemTransform
 * @category sources
 * @since 4.0.0
 * @deprecated Since unreleased. Prefer the use of SetSceneItemProperties.
 */
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

/**
 * Sets the crop coordinates of the specified source item.
 *
 * @param {String (optional)} `scene-name` the name of the scene that the source item belongs to. Defaults to the current scene.
 * @param {String} `item` The name of the source.
 * @param {int} `top` Pixel position of the top of the source item.
 * @param {int} `bottom` Pixel position of the bottom of the source item.
 * @param {int} `left` Pixel position of the left of the source item.
 * @param {int} `right` Pixel position of the right of the source item.
 *
 * @api requests
 * @name SetSceneItemCrop
 * @category sources
 * @since 4.1.0
 * @deprecated Since unreleased. Prefer the use of SetSceneItemProperties.
 */
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
        crop.bottom = obs_data_get_int(req->data, "bottom");
        crop.left = obs_data_get_int(req->data, "left");
        crop.right = obs_data_get_int(req->data, "right");

        obs_sceneitem_set_crop(scene_item, &crop);

        obs_sceneitem_release(scene_item);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("specified scene item doesn't exist");
    }

    obs_source_release(scene);
}

/**
 * Gets the scene specific properties of the specified source item.
 *
 * @param {String (optional)} `scene-name` the name of the scene that the source item belongs to. Defaults to the current scene.
 * @param {String} `item` The name of the source.
 *
 * @return {String} `name` The name of the source.
 * @return {int} `position.x` The x position of the source from the left.
 * @return {int} `position.y` The y position of the source from the top.
 * @return {int} `position.alignment` The point on the source that the item is manipulated from.
 * @return {double} `rotation` The clockwise rotation of the item in degrees around the point of alignment.
 * @return {double} `scale.x` The x-scale factor of the source.
 * @return {double} `scale.y` The y-scale factor of the source.
 * @return {int} `crop.top` The number of pixels cropped off the top of the source before scaling.
 * @return {int} `crop.right` The number of pixels cropped off the right of the source before scaling.
 * @return {int} `crop.bottom` The number of pixels cropped off the bottom of the source before scaling.
 * @return {int} `crop.left` The number of pixels cropped off the left of the source before scaling.
 * @return {bool} `visible` If the source is visible.
 * @return {String} `bounds.type` Type of bounding box.
 * @return {int} `bounds.alignment` Alignment of the bounding box.
 * @return {double} `bounds.x` Width of the bounding box.
 * @return {double} `bounds.y` Height of the bounding box.
 *
 * @api requests
 * @name GetSceneItemSceneProperties
 * @category sources
 * @since unreleased
 */
void WSRequestHandler::HandleGetSceneItemProperties(WSRequestHandler* req) {
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
    if (!scene_item) {
        req->SendErrorResponse("specified scene item doesn't exist");
        obs_source_release(scene);
        return;
    }

    obs_data_t* data = obs_data_create();

    obs_data_set_string(data, "name", item_name);

    obs_data_t* pos_data = obs_data_create();
    vec2 pos;
    obs_sceneitem_get_pos(scene_item, &pos);
    obs_data_set_double(pos_data, "x", pos.x);
    obs_data_set_double(pos_data, "y", pos.y);
    obs_data_set_int(pos_data, "alignment", obs_sceneitem_get_alignment(scene_item));
    obs_data_set_obj(data, "position", pos_data);

    obs_data_set_double(data, "rotation", obs_sceneitem_get_rot(scene_item));

    obs_data_t* scale_data = obs_data_create();
    vec2 scale;
    obs_sceneitem_get_scale(scene_item, &scale);
    obs_data_set_double(scale_data, "x", scale.x);
    obs_data_set_double(scale_data, "y", scale.y);
    obs_data_set_obj(data, "scale", scale_data);

    obs_data_t* crop_data = obs_data_create();
    obs_sceneitem_crop crop;
    obs_sceneitem_get_crop(scene_item, &crop);
    obs_data_set_int(crop_data, "left", crop.left);
    obs_data_set_int(crop_data, "top", crop.top);
    obs_data_set_int(crop_data, "right", crop.right);
    obs_data_set_int(crop_data, "bottom", crop.bottom);
    obs_data_set_obj(data, "crop", crop_data);

    obs_data_set_bool(data, "visible", obs_sceneitem_visible(scene_item));

    obs_data_t* bounds_data = obs_data_create();
    obs_bounds_type bounds_type = obs_sceneitem_get_bounds_type(scene_item);
    if (bounds_type == OBS_BOUNDS_NONE) {
        obs_data_set_string(bounds_data, "type", "OBS_BOUNDS_NONE");
    }
    else {
        switch(bounds_type) {
            case OBS_BOUNDS_STRETCH: {
                obs_data_set_string(bounds_data, "type", "OBS_BOUNDS_STRETCH");
                break;
            }
            case OBS_BOUNDS_SCALE_INNER: {
                obs_data_set_string(bounds_data, "type", "OBS_BOUNDS_SCALE_INNER");
                break;
            }
            case OBS_BOUNDS_SCALE_OUTER: {
                obs_data_set_string(bounds_data, "type", "OBS_BOUNDS_SCALE_OUTER");
                break;
            }
            case OBS_BOUNDS_SCALE_TO_WIDTH: {
                obs_data_set_string(bounds_data, "type", "OBS_BOUNDS_SCALE_TO_WIDTH");
                break;
            }
            case OBS_BOUNDS_SCALE_TO_HEIGHT: {
                obs_data_set_string(bounds_data, "type", "OBS_BOUNDS_SCALE_TO_HEIGHT");
                break;
            }
            case OBS_BOUNDS_MAX_ONLY: {
                obs_data_set_string(bounds_data, "type", "OBS_BOUNDS_MAX_ONLY");
                break;
            }
        }
        obs_data_set_int(bounds_data, "alignment", obs_sceneitem_get_bounds_alignment(scene_item));
        vec2 bounds;
        obs_sceneitem_get_bounds(scene_item, &bounds);
        obs_data_set_double(bounds_data, "x", bounds.x);
        obs_data_set_double(bounds_data, "y", bounds.y);
    }
    obs_data_set_obj(data, "bounds", bounds_data);

    obs_sceneitem_release(scene_item);
    req->SendOKResponse(data);
    obs_source_release(scene);
}

/**
 * Sets the scene specific properties of a source. Unspecified properties will remain unchanged.
 *
 * @param {String (optional)} `scene-name` the name of the scene that the source item belongs to. Defaults to the current scene.
 * @param {String} `item` The name of the source.
 * @param {int} `position.x` The new x position of the source.
 * @param {int} `position.y` The new y position of the source.
 * @param {int} `position.alignment` The new alignment of the source.
 * @param {double} `rotation` The new clockwise rotation of the item in degrees.
 * @param {double} `scale.x` The new x scale of the item.
 * @param {double} `scale.y` The new y scale of the item.
 * @param {int} `crop.top` The new amount of pixels cropped off the top of the source before scaling.
 * @param {int} `crop.bottom` The new amount of pixels cropped off the bottom of the source before scaling.
 * @param {int} `crop.left` The new amount of pixels cropped off the left of the source before scaling.
 * @param {int} `crop.right` The new amount of pixels cropped off the right of the source before scaling.
 * @param {bool} `visible` The new visibility of the source. 'true' shows source, 'false' hides source.
 * @param {String} `bounds.type` The new bounds type of the source.
 * @param {int} `bounds.alignment` The new alignment of the bounding box. (0-2, 4-6, 8-10)
 * @param {double} `bounds.x` The new width of the bounding box.
 * @param {double} `bounds.y' The new height of the bounding box.
 *
 * @api requests
 * @name SetSceneItemProperties
 * @category sources
 * @since unreleased
 */
void WSRequestHandler::HandleSetSceneItemProperties(WSRequestHandler* req) {
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
    if (!scene_item) {
        req->SendErrorResponse("specified scene item doesn't exist");
        obs_source_release(scene);
        return;
    }

    bool bad_request = false;
    obs_data_t* error_message = obs_data_create();

    if (req->hasField("position")) {
        vec2 old_position;
        obs_data_t* position_error = obs_data_create();
        obs_sceneitem_get_pos(scene_item, &old_position);
        obs_data_t* req_position = obs_data_get_obj(req->data, "position");
        vec2 new_position = old_position;
        if (obs_data_has_user_value(req_position, "x")) {
            new_position.x = obs_data_get_int(req_position, "x");
        }
        if (obs_data_has_user_value(req_position, "y")) {
            new_position.y = obs_data_get_int(req_position, "y");
        }
        if (obs_data_has_user_value(req_position, "alignment")) {
            const uint32_t alignment = obs_data_get_int(req_position, "alignment");
            if (Utils::IsValidAlignment(alignment)) {
                obs_sceneitem_set_alignment(scene_item, alignment);
            } else {
                bad_request = true;
                obs_data_set_string(position_error, "alignment", "invalid");
                obs_data_set_obj(error_message, "position", position_error);
            }
        }
        obs_sceneitem_set_pos(scene_item, &new_position);
    }

    if (req->hasField("rotation")) {
        obs_sceneitem_set_rot(scene_item, (float)obs_data_get_double(req->data, "rotation"));
    }

    if (req->hasField("scale")) {
        vec2 old_scale;
        obs_sceneitem_get_scale(scene_item, &old_scale);
        obs_data_t* req_scale = obs_data_get_obj(req->data, "scale");
        vec2 new_scale = old_scale;
        if (obs_data_has_user_value(req_scale, "x")) {
            new_scale.x = obs_data_get_double(req_scale, "x");
        }
        if (obs_data_has_user_value(req_scale, "y")) {
            new_scale.y = obs_data_get_double(req_scale, "y");
        }
        obs_sceneitem_set_scale(scene_item, &new_scale);
    }

    if (req->hasField("crop")) {
        obs_sceneitem_crop old_crop;
        obs_sceneitem_get_crop(scene_item, &old_crop);
        obs_data_t* req_crop = obs_data_get_obj(req->data, "crop");
        obs_sceneitem_crop new_crop = old_crop;
        if (obs_data_has_user_value(req_crop, "top")) {
            new_crop.top = obs_data_get_int(req_crop, "top");
        }
        if (obs_data_has_user_value(req_crop, "right")) {
            new_crop.right = obs_data_get_int(req_crop, "right");
        }
        if (obs_data_has_user_value(req_crop, "bottom")) {
            new_crop.bottom = obs_data_get_int(req_crop, "bottom");
        }
        if (obs_data_has_user_value(req_crop, "left")) {
            new_crop.left = obs_data_get_int(req_crop, "left");
        }
        obs_sceneitem_set_crop(scene_item, &new_crop);
    }

    if (req->hasField("visible")) {
        obs_sceneitem_set_visible(scene_item, obs_data_get_bool(req->data, "visible"));
    }

    if (req->hasField("bounds")) {
        bool bad_bounds = false;
        obs_data_t* bounds_error = obs_data_create();
        obs_data_t* req_bounds = obs_data_get_obj(req->data, "bounds");
        if (obs_data_has_user_value(req_bounds, "type")) {
            const char* new_bounds_type = obs_data_get_string(req_bounds, "type");
            if (new_bounds_type == "OBS_BOUNDS_NONE") {
                obs_sceneitem_set_bounds_type(scene_item, OBS_BOUNDS_NONE);
            }
            else if (new_bounds_type == "OBS_BOUNDS_STRETCH") {
                obs_sceneitem_set_bounds_type(scene_item, OBS_BOUNDS_STRETCH);
            }
            else if (new_bounds_type == "OBS_BOUNDS_SCALE_INNER") {
                obs_sceneitem_set_bounds_type(scene_item, OBS_BOUNDS_SCALE_INNER);
            }
            else if (new_bounds_type == "OBS_BOUNDS_SCALE_OUTER") {
                obs_sceneitem_set_bounds_type(scene_item, OBS_BOUNDS_SCALE_OUTER);
            }
            else if (new_bounds_type == "OBS_BOUNDS_SCALE_TO_WIDTH") {
                obs_sceneitem_set_bounds_type(scene_item, OBS_BOUNDS_SCALE_TO_WIDTH);
            }
            else if (new_bounds_type == "OBS_BOUNDS_SCALE_TO_HEIGHT") {
                obs_sceneitem_set_bounds_type(scene_item, OBS_BOUNDS_SCALE_TO_HEIGHT);
            }
            else if (new_bounds_type == "OBS_BOUNDS_MAX_ONLY") {
                obs_sceneitem_set_bounds_type(scene_item, OBS_BOUNDS_MAX_ONLY);
            }
            else {
                bad_request = bad_bounds = true;
                obs_data_set_string(bounds_error, "type", "invalid");
            }
        }
        vec2 old_bounds;
        obs_sceneitem_get_bounds(scene_item, &old_bounds);
        vec2 new_bounds = old_bounds;
        if (obs_data_has_user_value(req_bounds, "x")) {
            new_bounds.x = obs_data_get_double(req_bounds, "x");
        }
        if (obs_data_has_user_value(req_bounds, "y")) {
            new_bounds.y = obs_data_get_double(req_bounds, "y");
        }
        obs_sceneitem_set_bounds(scene_item, &new_bounds);
        if (obs_data_has_user_value(req_bounds, "alignment")) {
            const uint32_t bounds_alignment = obs_data_get_int(req_bounds, "alignment");
            if (Utils::IsValidAlignment(bounds_alignment)) {
                obs_sceneitem_set_bounds_alignment(scene_item, bounds_alignment);
            } else {
                bad_request = bad_bounds = true;
                obs_data_set_string(bounds_error, "alignment", "invalid");
            }
        }
        if (bad_bounds) {
            obs_data_set_obj(error_message, "bounds", bounds_error);
        }
    }

    obs_sceneitem_release(scene_item);
    if (bad_request) {
        req->SendErrorResponse(error_message);
    } else {
        req->SendOKResponse();
    }
    obs_source_release(scene);
}

/**
 * Change the active scene collection.
 *
 * @param {String} `sc-name` Name of the desired scene collection.
 *
 * @api requests
 * @name SetCurrentSceneCollection
 * @category scene collections
 * @since 4.0.0
 */
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

/**
 * Get the name of the current scene collection.
 *
 * @return {String} `sc-name` Name of the currently active scene collection.
 *
 * @api requests
 * @name GetCurrentSceneCollection
 * @category scene collections
 * @since 4.0.0
 */
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

/**
 * Set the currently active profile.
 * 
 * @param {String} `profile-name` Name of the desired profile.
 *
 * @api requests
 * @name SetCurrentProfile
 * @category profiles
 * @since 4.0.0
 */
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

 /**
 * Get the name of the current profile.
 * 
 * @return {String} `profile-name` Name of the currently active profile.
 *
 * @api requests
 * @name GetCurrentProfile
 * @category profiles
 * @since 4.0.0
 */
void WSRequestHandler::HandleGetCurrentProfile(WSRequestHandler* req) {
    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "profile-name",
        obs_frontend_get_current_profile());

    req->SendOKResponse(response);
    obs_data_release(response);
}

/**
 * Sets one or more attributes of the current streaming server settings. Any options not passed will remain unchanged. Returns the updated settings in response. If 'type' is different than the current streaming service type, all settings are required. Returns the full settings of the stream (the same as GetStreamSettings).
 * 
 * @param {String} `type` The type of streaming service configuration, usually `rtmp_custom` or `rtmp_common`.
 * @param {Object} `settings` The actual settings of the stream.
 * @param {String (optional)} `settings.server` The publish URL.
 * @param {String (optional)} `settings.key` The publish key.
 * @param {boolean (optional)} `settings.use-auth` Indicates whether authentication should be used when connecting to the streaming server.
 * @param {String (optional)} `settings.username` The username for the streaming service.
 * @param {String (optional)} `settings.password` The password for the streaming service.
 * @param {boolean} `save` Persist the settings to disk.
 *
 * @api requests
 * @name SetStreamSettings
 * @category settings
 * @since 4.1.0
 */
void WSRequestHandler::HandleSetStreamSettings(WSRequestHandler* req) {
    obs_service_t* service = obs_frontend_get_streaming_service();
    // get_streaming_service doesn't addref, so let's do it ourselves
    obs_service_addref(service);

    obs_data_t* requestSettings = obs_data_get_obj(req->data, "settings");
    if (!requestSettings) {
        req->SendErrorResponse("'settings' are required'");
        return;
    }

    QString serviceType = obs_service_get_type(service);
    QString requestedType = obs_data_get_string(req->data, "type");

    if (requestedType != nullptr && requestedType != serviceType) {
        obs_data_t* hotkeys = obs_hotkeys_save_service(service);

        // Release current service pointer before creating the new one
        obs_service_release(service);

        service = obs_service_create(
            requestedType.toUtf8(), "websocket_custom_service", requestSettings, hotkeys);

        obs_service_addref(service);

        obs_data_release(hotkeys);
    } else {
        // If type isn't changing, we should overlay the settings we got
        // to the existing settings. By doing so, you can send a request that
        // only contains the settings you want to change, instead of having to
        // do a get and then change them

        obs_data_t* existingSettings = obs_service_get_settings(service);
        obs_data_t* newSettings = obs_data_create();

        // Apply existing settings
        obs_data_apply(newSettings, existingSettings);
        // Then apply the settings from the request
        obs_data_apply(newSettings, requestSettings);

        obs_service_update(service, newSettings);

        obs_data_release(newSettings);
        obs_data_release(existingSettings);
    }

    //if save is specified we should immediately save the streaming service
    if (obs_data_get_bool(req->data, "save")) {
        obs_frontend_save_streaming_service();
    }

    obs_data_t* serviceSettings = obs_service_get_settings(service);

    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "type", requestedType.toUtf8());
    obs_data_set_obj(response, "settings", serviceSettings);

    req->SendOKResponse(response);

    obs_data_release(response);
    obs_data_release(serviceSettings);
    obs_data_release(requestSettings);
    obs_service_release(service);
}

/**
 * Get the current streaming server settings.
 *
 * @return {String} `type` The type of streaming service configuration. Possible values: 'rtmp_custom' or 'rtmp_common'.
 * @return {Object} `settings` Stream settings object.
 * @return {String} `settings.server` The publish URL.
 * @return {String} `settings.key` The publish key of the stream.
 * @return {boolean} `settings.use-auth` Indicates whether audentication should be used when connecting to the streaming server.
 * @return {String} `settings.username` The username to use when accessing the streaming server. Only present if `use-auth` is `true`.
 * @return {String} `settings.password` The password to use when accessing the streaming server. Only present if `use-auth` is `true`.
 *
 * @api requests
 * @name GetStreamSettings
 * @category settings
 * @since 4.1.0
 */
void WSRequestHandler::HandleGetStreamSettings(WSRequestHandler* req) {
    obs_service_t* service = obs_frontend_get_streaming_service();
    // get_streaming_service doesn't addref, so let's do it ourselves
    obs_service_addref(service);

    const char* serviceType = obs_service_get_type(service);
    obs_data_t* settings = obs_service_get_settings(service);

    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "type", serviceType);
    obs_data_set_obj(response, "settings", settings);

    req->SendOKResponse(response);

    obs_data_release(response);
    obs_data_release(settings);
    obs_service_release(service);
}

/**
 * Save the current streaming server settings to disk.
 *
 * @api requests
 * @name SaveStreamSettings
 * @category settings
 * @since 4.1.0
 */
void WSRequestHandler::HandleSaveStreamSettings(WSRequestHandler* req) {
    obs_frontend_save_streaming_service();
    req->SendOKResponse();
}

/**
 * Get a list of available profiles.
 *
 * @return {Object|Array} `profiles` List of available profiles.
 *
 * @api requests
 * @name ListProfiles
 * @category profiles
 * @since 4.0.0
 */
void WSRequestHandler::HandleListProfiles(WSRequestHandler* req) {
    obs_data_array_t* profiles = Utils::GetProfiles();

    obs_data_t* response = obs_data_create();
    obs_data_set_array(response, "profiles", profiles);

    req->SendOKResponse(response);
    obs_data_release(response);
    obs_data_array_release(profiles);
}

/**
 * Indicates if Studio Mode is currently enabled.
 *
 * @return {boolean} `studio-mode` Indicates if Studio Mode is enabled.
 *
 * @api requests
 * @name GetStudioModeStatus
 * @category studio mode
 * @since 4.1.0
 */
void WSRequestHandler::HandleGetStudioModeStatus(WSRequestHandler* req) {
    bool previewActive = obs_frontend_preview_program_mode_active();

    obs_data_t* response = obs_data_create();
    obs_data_set_bool(response, "studio-mode", previewActive);

    req->SendOKResponse(response);
    obs_data_release(response);
}

/**
 * Get the name of the currently previewed scene and its list of sources.
 * Will return an `error` if Studio Mode is not enabled.
 *
 * @return {String} `name` The name of the active preview scene.
 * @return {Source|Array} `sources`
 *
 * @api requests
 * @name GetPreviewScene
 * @category studio mode
 * @since 4.1.0
 */
void WSRequestHandler::HandleGetPreviewScene(WSRequestHandler* req) {
    if (!obs_frontend_preview_program_mode_active()) {
        req->SendErrorResponse("studio mode not enabled");
        return;
    }

    obs_source_t* scene = obs_frontend_get_current_preview_scene();
    obs_data_array_t* scene_items = Utils::GetSceneItems(scene);

    obs_data_t* data = obs_data_create();
    obs_data_set_string(data, "name", obs_source_get_name(scene));
    obs_data_set_array(data, "sources", scene_items);

    req->SendOKResponse(data);
    obs_data_release(data);
    obs_data_array_release(scene_items);
}

/**
 * Set the active preview scene.
 * Will return an `error` if Studio Mode is not enabled.
 *
 * @param {String} `scene-name` The name of the scene to preview.
 *
 * @api requests
 * @name SetPreviewScene
 * @category studio mode
 * @since 4.1.0
 */
void WSRequestHandler::HandleSetPreviewScene(WSRequestHandler* req) {
    if (!obs_frontend_preview_program_mode_active()) {
        req->SendErrorResponse("studio mode not enabled");
        return;
    }

    if (!req->hasField("scene-name")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* scene_name = obs_data_get_string(req->data, "scene-name");
    obs_source_t* scene = Utils::GetSceneFromNameOrCurrent(scene_name);

    if (scene) {
        obs_frontend_set_current_preview_scene(scene);
        req->SendOKResponse();
    } else {
        req->SendErrorResponse("specified scene doesn't exist");
    }

    obs_source_release(scene);
}

/**
 * Transitions the currently previewed scene to the main output.
 * Will return an `error` if Studio Mode is not enabled.
 *
 * @param {Object (optional)} `with-transition` Change the active transition before switching scenes. Defaults to the active transition. 
 * @param {String} `with-transition.name` Name of the transition.
 * @param {int (optional)} `with-transition.duration` Transition duration (in milliseconds).
 *
 * @api requests
 * @name TransitionToProgram
 * @category studio mode
 * @since 4.1.0
 */
void WSRequestHandler::HandleTransitionToProgram(WSRequestHandler* req) {
    if (!obs_frontend_preview_program_mode_active()) {
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

/**
 * Enables Studio Mode.
 *
 * @api requests
 * @name EnableStudioMode
 * @category studio mode
 * @since 4.1.0
 */
void WSRequestHandler::HandleEnableStudioMode(WSRequestHandler* req) {
    obs_frontend_set_preview_program_mode(true);
    req->SendOKResponse();
}

/**
 * Disables Studio Mode.
 *
 * @api requests
 * @name DisableStudioMode
 * @category studio mode
 * @since 4.1.0
 */
void WSRequestHandler::HandleDisableStudioMode(WSRequestHandler* req) {
    obs_frontend_set_preview_program_mode(false);
    req->SendOKResponse();
}

/**
 * Toggles Studio Mode.
 *
 * @api requests
 * @name ToggleStudioMode
 * @category studio mode
 * @since 4.1.0
 */
void WSRequestHandler::HandleToggleStudioMode(WSRequestHandler* req) {
    bool previewProgramMode = obs_frontend_preview_program_mode_active();
    obs_frontend_set_preview_program_mode(!previewProgramMode);
    req->SendOKResponse();
}

/**
 * Get configured special sources like Desktop Audio and Mic/Aux sources.
 *
 * @return {String (optional)} `desktop-1` Name of the first Desktop Audio capture source.
 * @return {String (optional)} `desktop-2` Name of the second Desktop Audio capture source.
 * @return {String (optional)} `mic-1` Name of the first Mic/Aux input source.
 * @return {String (optional)} `mic-2` Name of the second Mic/Aux input source.
 * @return {String (optional)} `mic-3` NAme of the third Mic/Aux input source.
 *
 * @api requests
 * @name GetSpecialSources
 * @category studio mode
 * @since 4.1.0
 */
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

/**
 * Change the current recording folder.
 *
 * @param {String} `rec-folder` Path of the recording folder.
 *
 * @api requests
 * @name SetRecordingFolder
 * @category recording
 * @since 4.1.0
 */
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

/**
 * Get the path of  the current recording folder.
 *
 * @return {String} `rec-folder` Path of the recording folder.
 *
 * @api requests
 * @name GetRecordingFolder
 * @category recording
 * @since 4.1.0
 */
void WSRequestHandler::HandleGetRecordingFolder(WSRequestHandler* req) {
    const char* recFolder = Utils::GetRecordingFolder();

    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "rec-folder", recFolder);

    req->SendOKResponse(response);
    obs_data_release(response);
}

/**
 * Get the current properties of a Text GDI Plus source.
 *
 * @param {String (optional)} `scene-name` Name of the scene to retrieve. Defaults to the current scene.
 * @param {String} `source` Name of the source.
 *
 * @return {String} `align` Text Alignment ("left", "center", "right").
 * @return {int} `bk-color` Background color.
 * @return {int} `bk-opacity` Background opacity (0-100).
 * @return {boolean} `chatlog` Chat log.
 * @return {int} `chatlog_lines` Chat log lines.
 * @return {int} `color` Text color.
 * @return {boolean} `extents` Extents wrap.
 * @return {int} `extents_cx` Extents cx.
 * @return {int} `extents_cy` Extents cy.
 * @return {String} `file` File path name.
 * @return {boolean} `read_from_file` Read text from the specified file.
 * @return {Object} `font` Holds data for the font. Ex: `"font": { "face": "Arial", "flags": 0, "size": 150, "style": "" }`
 * @return {String} `font.face` Font face.
 * @return {int} `font.flags` Font text styling flag. `Bold=1, Italic=2, Bold Italic=3, Underline=5, Strikeout=8`
 * @return {int} `font.size` Font text size.
 * @return {String} `font.style` Font Style (unknown function).
 * @return {boolean} `gradient` Gradient enabled.
 * @return {int} `gradient_color` Gradient color.
 * @return {float} `gradient_dir` Gradient direction.
 * @return {int} `gradient_opacity` Gradient opacity (0-100).
 * @return {boolean} `outline` Outline.
 * @return {int} `outline_color` Outline color.
 * @return {int} `outline_size` Outline size.
 * @return {int} `outline_opacity` Outline opacity (0-100).
 * @return {String} `text` Text content to be displayed.
 * @return {String} `valign` Text vertical alignment ("top", "center", "bottom").
 * @return {boolean} `vertical` Vertical text enabled.
 * @return {boolean} `render` Visibility of the scene item.
 *
 * @api requests
 * @name GetTextGDIPlusProperties
 * @category sources
 * @since 4.1.0
 */
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

/**
 * Get the current properties of a Text GDI Plus source.
 *
 * @param {String (optional)} `scene-name` Name of the scene to retrieve. Defaults to the current scene.
 * @param {String} `source` Name of the source.
 * @param {String (optional)} `align` Text Alignment ("left", "center", "right").
 * @param {int (optional)} `bk-color` Background color.
 * @param {int (optional)} `bk-opacity` Background opacity (0-100).
 * @param {boolean (optional)} `chatlog` Chat log.
 * @param {int (optional)} `chatlog_lines` Chat log lines.
 * @param {int (optional)} `color` Text color.
 * @param {boolean (optional)} `extents` Extents wrap.
 * @param {int (optional)} `extents_cx` Extents cx.
 * @param {int (optional)} `extents_cy` Extents cy.
 * @param {String (optional)} `file` File path name.
 * @param {boolean (optional)} `read_from_file` Read text from the specified file.
 * @param {Object (optional)} `font` Holds data for the font. Ex: `"font": { "face": "Arial", "flags": 0, "size": 150, "style": "" }`
 * @param {String (optional)} `font.face` Font face.
 * @param {int (optional)} `font.flags` Font text styling flag. `Bold=1, Italic=2, Bold Italic=3, Underline=5, Strikeout=8`
 * @param {int (optional)} `font.size` Font text size.
 * @param {String (optional)} `font.style` Font Style (unknown function).
 * @param {boolean (optional)} `gradient` Gradient enabled.
 * @param {int (optional)} `gradient_color` Gradient color.
 * @param {float (optional)} `gradient_dir` Gradient direction.
 * @param {int (optional)} `gradient_opacity` Gradient opacity (0-100).
 * @param {boolean (optional)} `outline` Outline.
 * @param {int (optional)} `outline_color` Outline color.
 * @param {int (optional)} `outline_size` Outline size.
 * @param {int (optional)} `outline_opacity` Outline opacity (0-100).
 * @param {String (optional)} `text` Text content to be displayed.
 * @param {String (optional)} `valign` Text vertical alignment ("top", "center", "bottom").
 * @param {boolean (optional)} `vertical` Vertical text enabled.
 * @param {boolean (optional)} `render` Visibility of the scene item.
 *
 * @api requests
 * @name SetTextGDIPlusProperties
 * @category sources
 * @since 4.1.0
 */
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

/**
 * Get current properties for a Browser Source.
 *
 * @param {String (optional)} `scene-name` Name of the scene that the source belongs to. Defaults to the current scene.
 * @param {String} `source` Name of the source.
 *
 * @return {boolean} `is_local_file` Indicates that a local file is in use.
 * @return {String} `url` Url or file path.
 * @return {String} `css` CSS to inject.
 * @return {int} `width` Width.
 * @return {int} `height` Height.
 * @return {int} `fps` Framerate.
 * @return {boolean} `shutdown` Indicates whether the source should be shutdown when not visible.
 * @return {boolean (optional)} `render` Visibility of the scene item.
 *
 * @api requests
 * @name GetBrowserSourceProperties
 * @category sources
 * @since 4.1.0
 */
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

/**
 * Set current properties for a Browser Source.
 *
 * @param {String (optional)} `scene-name` Name of the scene that the source belongs to. Defaults to the current scene.
 * @param {String} `source` Name of the source.
 * @param {boolean (optional)} `is_local_file` Indicates that a local file is in use.
 * @param {String (optional)} `url` Url or file path.
 * @param {String (optional)} `css` CSS to inject.
 * @param {int (optional)} `width` Width.
 * @param {int (optional)} `height` Height.
 * @param {int (optional)} `fps` Framerate.
 * @param {boolean (optional)} `shutdown` Indicates whether the source should be shutdown when not visible.
 * @param {boolean (optional)} `render` Visibility of the scene item.
 *
 * @api requests
 * @name SetBrowserSourceProperties
 * @category sources
 * @since 4.1.0
 */
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

/**
 * Reset a source item.
 *
 * @param {String (optional)} `scene-name` Name of the scene the source belogns to. Defaults to the current scene.
 * @param {String} `item` Name of the source item.
 *
 * @api requests
 * @name ResetSceneItem
 * @category sources
 * @since 4.2.0
 */
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

/**
* List all sources available in the running OBS instance
*
* @return {Array of Objects} `sources` Array of sources as objects
* @return {String} `sources.*.name` Source name
* @return {String} `sources.*.type` Source type
*
* @api requests
* @name GetSourcesList
* @category sources
* @since unreleased
*/
void WSRequestHandler::HandleGetSourcesList(WSRequestHandler* req) {
    obs_data_array_t* sourcesArray = obs_data_array_create();

    auto source_enum = [](void* privateData, obs_source_t* source) -> bool {
        obs_data_array_t* sourcesArray = (obs_data_array_t*)privateData;

        obs_data_t* sourceSettings = obs_source_get_settings(source);

        obs_data_t* sourceData = obs_data_create();
        obs_data_set_string(sourceData, "name", obs_source_get_name(source));
        obs_data_set_string(sourceData, "type", obs_source_get_id(source));

        obs_data_array_push_back(sourcesArray, sourceData);

        obs_data_release(sourceSettings);
        obs_data_release(sourceData);

        return true;
    };
    obs_enum_sources(source_enum, sourcesArray);


    obs_data_t* response = obs_data_create();
    obs_data_set_array(response, "sources", sourcesArray);
    req->SendOKResponse(response);

    obs_data_release(response);
    obs_data_array_release(sourcesArray);
}

/**
* Get settings of the specified source
*
* @param {String} `sourceName` Name of the source item.
* @param {String (optional) `sourceType` Type of the specified source. Useful for type-checking if you expect a specific settings schema.
*
* @return {String} `sourceName` Source name
* @return {String} `sourceType` Type of the specified source
* @return {Object} `sourceSettings` Source settings. Varying between source types.
*
* @api requests
* @name GetSourceSettings
* @category sources
* @since unreleased
*/
void WSRequestHandler::HandleGetSourceSettings(WSRequestHandler* req) {
    if (!req->hasField("sourceName")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* sourceName = obs_data_get_string(req->data, "sourceName");
    obs_source_t* source = obs_get_source_by_name(sourceName);
    if (!source) {
        req->SendErrorResponse("specified source doesn't exist");
        return;
    }

    if (req->hasField("sourceType")) {
        QString actualSourceType = obs_source_get_id(source);
        QString requestedType = obs_data_get_string(req->data, "sourceType");

        if (actualSourceType != requestedType) {
            req->SendErrorResponse("specified source exists but is not of expected type");
            obs_source_release(source);
            return;
        }
    }

    obs_data_t* sourceSettings = obs_source_get_settings(source);

    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "sourceName", obs_source_get_name(source));
    obs_data_set_string(response, "sourceType", obs_source_get_id(source));
    obs_data_set_obj(response, "sourceSettings", sourceSettings);
    req->SendOKResponse(response);

    obs_data_release(response);
    obs_data_release(sourceSettings);
    obs_source_release(source);
}

/**
* Set settings of the specified source.
*
* @param {String} `sourceName` Name of the source item.
* @param {String (optional)} `sourceType` Type of the specified source. Useful for type-checking to avoid settings a set of settings incompatible with the actual source's type.
* @param {Object} `sourceSettings` Source settings. Varying between source types.
*
* @return {String} `sourceName` Source name
* @return {String} `sourceType` Type of the specified source
* @return {Object} `sourceSettings` Source settings. Varying between source types.
*
* @api requests
* @name SetSourceSettings
* @category sources
* @since unreleased
*/
void WSRequestHandler::HandleSetSourceSettings(WSRequestHandler* req) {
    if (!req->hasField("sourceName") || !req->hasField("sourceSettings")) {
        req->SendErrorResponse("missing request parameters");
        return;
    }

    const char* sourceName = obs_data_get_string(req->data, "sourceName");
    obs_source_t* source = obs_get_source_by_name(sourceName);
    if (!source) {
        req->SendErrorResponse("specified source doesn't exist");
        return;
    }

    if (req->hasField("sourceType")) {
        QString actualSourceType = obs_source_get_id(source);
        QString requestedType = obs_data_get_string(req->data, "sourceType");

        if (actualSourceType != requestedType) {
            req->SendErrorResponse("specified source exists but is not of expected type");
            obs_source_release(source);
            return;
        }
    }

    obs_data_t* currentSettings = obs_source_get_settings(source);
    obs_data_t* newSettings = obs_data_get_obj(req->data, "sourceSettings");

    obs_data_t* sourceSettings = obs_data_create();
    obs_data_apply(sourceSettings, currentSettings);
    obs_data_apply(sourceSettings, newSettings);

    obs_source_update(source, sourceSettings);
    obs_source_update_properties(source);

    obs_data_t* response = obs_data_create();
    obs_data_set_string(response, "sourceName", obs_source_get_name(source));
    obs_data_set_string(response, "sourceType", obs_source_get_id(source));
    obs_data_set_obj(response, "sourceSettings", sourceSettings);
    req->SendOKResponse(response);

    obs_data_release(response);
    obs_data_release(sourceSettings);
    obs_data_release(newSettings);
    obs_data_release(currentSettings);
    obs_source_release(source);
}
