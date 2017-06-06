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

#include "WSRequestHandler.h"
#include "WSEvents.h"
#include "obs-websocket.h"
#include "Config.h"
#include "Utils.h"

bool str_valid(const char* str)
{
	return (str != nullptr && strlen(str) > 0);
}

WSRequestHandler::WSRequestHandler(QWebSocket* client) :
	_messageId(0),
	_requestType(""),
	data(nullptr)
{
	_client = client;

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

	messageMap["GetStreamingStatus"] = WSRequestHandler::HandleGetStreamingStatus;
	messageMap["StartStopStreaming"] = WSRequestHandler::HandleStartStopStreaming;
	messageMap["StartStopRecording"] = WSRequestHandler::HandleStartStopRecording;
	messageMap["StartStreaming"] = WSRequestHandler::HandleStartStreaming;
	messageMap["StopStreaming"] = WSRequestHandler::HandleStopStreaming;
	messageMap["StartRecording"] = WSRequestHandler::HandleStartRecording;
	messageMap["StopRecording"] = WSRequestHandler::HandleStopRecording;

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

	messageMap["GetStudioModeStatus"] = WSRequestHandler::HandleGetStudioModeStatus;
	messageMap["GetPreviewScene"] = WSRequestHandler::HandleGetPreviewScene;
	messageMap["SetPreviewScene"] = WSRequestHandler::HandleSetPreviewScene;
	messageMap["TransitionToProgram"] = WSRequestHandler::HandleTransitionToProgram;
	messageMap["EnableStudioMode"] = WSRequestHandler::HandleEnableStudioMode;
	messageMap["DisableStudioMode"] = WSRequestHandler::HandleDisableStudioMode;
	messageMap["ToggleStudioMode"] = WSRequestHandler::HandleToggleStudioMode;

	authNotRequired.insert("GetVersion");
	authNotRequired.insert("GetAuthRequired");
	authNotRequired.insert("Authenticate");
}

void WSRequestHandler::processIncomingMessage(QString textMessage)
{
	QByteArray msgData = textMessage.toUtf8();
	const char* msg = msgData;

	data = obs_data_create_from_json(msg);
	if (!data)
	{
		if (!msg)
			msg = "<null pointer>";

		blog(LOG_ERROR, "invalid JSON payload received for '%s'", msg);
		SendErrorResponse("invalid JSON payload");
		return;
	}

	if (!hasField("request-type") ||
		!hasField("message-id"))
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

	if (handlerFunc != NULL)
		handlerFunc(this);
	else
		SendErrorResponse("invalid request type");

	obs_data_release(data);
}

WSRequestHandler::~WSRequestHandler()
{
}

void WSRequestHandler::SendOKResponse(obs_data_t* additionalFields)
{
	obs_data_t* response = obs_data_create();
	obs_data_set_string(response, "status", "ok");
	obs_data_set_string(response, "message-id", _messageId);

	if (additionalFields)
		obs_data_apply(response, additionalFields);

	_client->sendTextMessage(obs_data_get_json(response));

	obs_data_release(response);
}

void WSRequestHandler::SendErrorResponse(const char* errorMessage)
{
	obs_data_t* response = obs_data_create();
	obs_data_set_string(response, "status", "error");
	obs_data_set_string(response, "error", errorMessage);
	obs_data_set_string(response, "message-id", _messageId);

	_client->sendTextMessage(obs_data_get_json(response));

	obs_data_release(response);
}

bool WSRequestHandler::hasField(const char* name)
{
	if (!name || !data)
		return false;

	return obs_data_has_user_value(data, name);
}

void WSRequestHandler::HandleGetVersion(WSRequestHandler* req)
{
	const char* obs_version = Utils::OBSVersionString();

	obs_data_t* data = obs_data_create();
	obs_data_set_double(data, "version", API_VERSION);
	obs_data_set_string(data, "obs-websocket-version", OBS_WEBSOCKET_VERSION);
	obs_data_set_string(data, "obs-studio-version", obs_version);

	req->SendOKResponse(data);

	obs_data_release(data);
	bfree((void*)obs_version);
}

void WSRequestHandler::HandleGetAuthRequired(WSRequestHandler* req)
{
	bool authRequired = Config::Current()->AuthRequired;

	obs_data_t* data = obs_data_create();
	obs_data_set_bool(data, "authRequired", authRequired);

	if (authRequired)
	{
		obs_data_set_string(data, "challenge",
			Config::Current()->SessionChallenge);
		obs_data_set_string(data, "salt",
			Config::Current()->Salt);
	}

	req->SendOKResponse(data);

	obs_data_release(data);
}

void WSRequestHandler::HandleAuthenticate(WSRequestHandler* req)
{
	if (!req->hasField("auth"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* auth = obs_data_get_string(req->data, "auth");
	if (!str_valid(auth))
	{
		req->SendErrorResponse("auth not specified!");
		return;
	}

	if ((req->_client->property(PROP_AUTHENTICATED).toBool() == false)
		&& Config::Current()->CheckAuth(auth))
	{
		req->_client->setProperty(PROP_AUTHENTICATED, true);
		req->SendOKResponse();
	}
	else
	{
		req->SendErrorResponse("Authentication Failed.");
	}
}

void WSRequestHandler::HandleSetCurrentScene(WSRequestHandler* req)
{
	if (!req->hasField("scene-name"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* sceneName = obs_data_get_string(req->data, "scene-name");
	obs_source_t* source = obs_get_source_by_name(sceneName);

	if (source)
	{
		obs_frontend_set_current_scene(source);
		req->SendOKResponse();
	}
	else
	{
		req->SendErrorResponse("requested scene does not exist");
	}

	obs_source_release(source);
}

void WSRequestHandler::HandleGetCurrentScene(WSRequestHandler* req)
{
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

void WSRequestHandler::HandleGetSceneList(WSRequestHandler* req)
{
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

void WSRequestHandler::HandleSetSceneItemRender(WSRequestHandler* req)
{
	if (!req->hasField("source") ||
		!req->hasField("render"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* itemName = obs_data_get_string(req->data, "source");
	bool isVisible = obs_data_get_bool(req->data, "render");

	if (!itemName)
	{
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	const char* sceneName = obs_data_get_string(req->data, "scene-name");
	obs_source_t* scene = Utils::GetSceneFromNameOrCurrent(sceneName);
	if (scene == NULL) {
		req->SendErrorResponse("requested scene doesn't exist");
		return;
	}

	obs_sceneitem_t* sceneItem = Utils::GetSceneItemFromName(scene, itemName);
	if (sceneItem != NULL)
	{
		obs_sceneitem_set_visible(sceneItem, isVisible);
		obs_sceneitem_release(sceneItem);
		req->SendOKResponse();
	}
	else
	{
		req->SendErrorResponse("specified scene item doesn't exist");
	}

	obs_source_release(scene);
}

void WSRequestHandler::HandleGetStreamingStatus(WSRequestHandler* req)
{
	obs_data_t* data = obs_data_create();
	obs_data_set_bool(data, "streaming", obs_frontend_streaming_active());
	obs_data_set_bool(data, "recording", obs_frontend_recording_active());
	obs_data_set_bool(data, "preview-only", false);

	const char* tc = nullptr;
	if (obs_frontend_streaming_active())
	{
		tc = WSEvents::Instance->GetStreamingTimecode();
		obs_data_set_string(data, "stream-timecode", tc);
		bfree((void*)tc);
	}

	if (obs_frontend_recording_active())
	{
		tc = WSEvents::Instance->GetRecordingTimecode();
		obs_data_set_string(data, "rec-timecode", tc);
		bfree((void*)tc);
	}

	req->SendOKResponse(data);
	obs_data_release(data);
}

void WSRequestHandler::HandleStartStopStreaming(WSRequestHandler* req)
{
	if (obs_frontend_streaming_active())
		obs_frontend_streaming_stop();
	else
		obs_frontend_streaming_start();

	req->SendOKResponse();
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
	if (obs_frontend_streaming_active() == false)
	{
		obs_frontend_streaming_start();
		req->SendOKResponse();
	}
	else
	{
		req->SendErrorResponse("streaming already active");
	}
}

void WSRequestHandler::HandleStopStreaming(WSRequestHandler* req)
{
	if (obs_frontend_streaming_active() == true)
	{
		obs_frontend_streaming_stop();
		req->SendOKResponse();
	}
	else
	{
		req->SendErrorResponse("streaming not active");
	}
}

void WSRequestHandler::HandleStartRecording(WSRequestHandler* req)
{
	if (obs_frontend_recording_active() == false)
	{
		obs_frontend_recording_start();
		req->SendOKResponse();
	}
	else
	{
		req->SendErrorResponse("recording already active");
	}
}

void WSRequestHandler::HandleStopRecording(WSRequestHandler* req)
{
	if (obs_frontend_recording_active() == true)
	{
		obs_frontend_recording_stop();
		req->SendOKResponse();
	}
	else
	{
		req->SendErrorResponse("recording not active");
	}
}

void WSRequestHandler::HandleGetTransitionList(WSRequestHandler* req)
{
	obs_source_t* current_transition = obs_frontend_get_current_transition();
	obs_frontend_source_list transitionList = {};
	obs_frontend_get_transitions(&transitionList);

	obs_data_array_t* transitions = obs_data_array_create();
	for (size_t i = 0; i < transitionList.sources.num; i++)
	{
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

void WSRequestHandler::HandleGetCurrentTransition(WSRequestHandler* req)
{
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

void WSRequestHandler::HandleSetCurrentTransition(WSRequestHandler* req)
{
	if (!req->hasField("transition-name"))
	{
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

void WSRequestHandler::HandleSetTransitionDuration(WSRequestHandler* req)
{
	if (!req->hasField("duration"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	int ms = obs_data_get_int(req->data, "duration");
	Utils::SetTransitionDuration(ms);
	req->SendOKResponse();
}

void WSRequestHandler::HandleGetTransitionDuration(WSRequestHandler* req)
{
	obs_data_t* response = obs_data_create();
	obs_data_set_int(response, "transition-duration",
		Utils::GetTransitionDuration());

	req->SendOKResponse(response);
	obs_data_release(response);
}

void WSRequestHandler::HandleSetVolume(WSRequestHandler* req)
{
	if (!req->hasField("source") ||
		!req->hasField("volume"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* source_name = obs_data_get_string(req->data, "source");
	float source_volume = obs_data_get_double(req->data, "volume");

	if (source_name == NULL || strlen(source_name) < 1 || 
		source_volume < 0.0 || source_volume > 1.0)
	{
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	obs_source_t* source = obs_get_source_by_name(source_name);
	if (!source)
	{
		req->SendErrorResponse("specified source doesn't exist");
		return;
	}

	obs_source_set_volume(source, source_volume);
	req->SendOKResponse();

	obs_source_release(source);
}

void WSRequestHandler::HandleGetVolume(WSRequestHandler* req)
{
	if (!req->hasField("source"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* source_name = obs_data_get_string(req->data, "source");

	if (str_valid(source_name))
	{
		obs_source_t* source = obs_get_source_by_name(source_name);

		obs_data_t* response = obs_data_create();
		obs_data_set_string(response, "name", source_name);
		obs_data_set_double(response, "volume", obs_source_get_volume(source));
		obs_data_set_bool(response, "muted", obs_source_muted(source));

		req->SendOKResponse(response);

		obs_data_release(response);
		obs_source_release(source);
	}
	else
	{
		req->SendErrorResponse("invalid request parameters");
	}
}

void WSRequestHandler::HandleToggleMute(WSRequestHandler* req)
{
	if (!req->hasField("source"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* source_name = obs_data_get_string(req->data, "source");
	if (!str_valid(source_name))
	{
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	obs_source_t* source = obs_get_source_by_name(source_name);
	if (!source)
	{
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	obs_source_set_muted(source, !obs_source_muted(source));
	req->SendOKResponse();

	obs_source_release(source);
}

void WSRequestHandler::HandleSetMute(WSRequestHandler* req)
{
	if (!req->hasField("source") ||
		!req->hasField("mute"))
	{
		req->SendErrorResponse("mssing request parameters");
		return;
	}

	const char* source_name = obs_data_get_string(req->data, "source");
	bool mute = obs_data_get_bool(req->data, "mute");

	if (!str_valid(source_name))
	{
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	obs_source_t* source = obs_get_source_by_name(source_name);
	if (!source)
	{
		req->SendErrorResponse("specified source doesn't exist");
		return;
	}

	obs_source_set_muted(source, mute);
	req->SendOKResponse();

	obs_source_release(source);
}

void WSRequestHandler::HandleGetMute(WSRequestHandler* req)
{
	if (!req->hasField("source"))
	{
		req->SendErrorResponse("mssing request parameters");
		return;
	}

	const char* source_name = obs_data_get_string(req->data, "source");

	if (!str_valid(source_name))
	{
		req->SendErrorResponse("invalid request parameters");
		return;
	}

	obs_source_t* source = obs_get_source_by_name(source_name);
	if (!source)
	{
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

void WSRequestHandler::HandleSetSceneItemPosition(WSRequestHandler* req)
{
	if (!req->hasField("item") ||
		!req->hasField("x") || !req->hasField("y"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* item_name = obs_data_get_string(req->data, "item");
	if (!str_valid(item_name))
	{
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

	if (scene_item)
	{
		vec2 item_position = { 0 };
		item_position.x = obs_data_get_double(req->data, "x");
		item_position.y = obs_data_get_double(req->data, "y");

		obs_sceneitem_set_pos(scene_item, &item_position);

		obs_sceneitem_release(scene_item);
		req->SendOKResponse();
	}
	else
	{
		req->SendErrorResponse("specified scene item doesn't exist");
	}

	obs_source_release(scene);
}

void WSRequestHandler::HandleSetSceneItemTransform(WSRequestHandler* req)
{
	if (!req->hasField("item") ||
		!req->hasField("x-scale") ||
		!req->hasField("y-scale") ||
		!req->hasField("rotation"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* item_name = obs_data_get_string(req->data, "item");
	if (!str_valid(item_name))
	{
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

	if (scene_item)
	{
		obs_sceneitem_set_scale(scene_item, &scale);
		obs_sceneitem_set_rot(scene_item, rotation);

		obs_sceneitem_release(scene_item);
		req->SendOKResponse();
	}
	else
	{
		req->SendErrorResponse("specified scene item doesn't exist");
	}

	obs_source_release(scene);
}

void WSRequestHandler::HandleSetSceneItemCrop(WSRequestHandler* req)
{
	if (!req->hasField("item"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* item_name = obs_data_get_string(req->data, "item");
	if (!str_valid(item_name))
	{
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

	if (scene_item)
	{
		struct obs_sceneitem_crop crop = { 0 };
		crop.top = obs_data_get_int(req->data, "top");
		crop.bottom = obs_data_get_int(req->data, "bottom");;
		crop.left = obs_data_get_int(req->data, "left");;
		crop.right = obs_data_get_int(req->data, "right");

		obs_sceneitem_set_crop(scene_item, &crop);

		obs_sceneitem_release(scene_item);
		req->SendOKResponse();
	}
	else
	{
		req->SendErrorResponse("specified scene item doesn't exist");
	}

	obs_source_release(scene);
}

void WSRequestHandler::HandleSetCurrentSceneCollection(WSRequestHandler* req)
{
	if (!req->hasField("sc-name"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* scene_collection = obs_data_get_string(req->data, "sc-name");

	if (str_valid(scene_collection))
	{
		// TODO : Check if specified profile exists and if changing is allowed
		obs_frontend_set_current_scene_collection(scene_collection);
		req->SendOKResponse();
	}
	else
	{
		req->SendErrorResponse("invalid request parameters");
	}
}

void WSRequestHandler::HandleGetCurrentSceneCollection(WSRequestHandler* req)
{
	obs_data_t* response = obs_data_create();
	obs_data_set_string(response, "sc-name",
		obs_frontend_get_current_scene_collection());

	req->SendOKResponse(response);

	obs_data_release(response);
}

void WSRequestHandler::HandleListSceneCollections(WSRequestHandler* req)
{
	obs_data_array_t* scene_collections = Utils::GetSceneCollections();

	obs_data_t* response = obs_data_create();
	obs_data_set_array(response, "scene-collections", scene_collections);

	req->SendOKResponse(response);

	obs_data_release(response);
	obs_data_array_release(scene_collections);
}

void WSRequestHandler::HandleSetCurrentProfile(WSRequestHandler* req)
{
	if (!req->hasField("profile-name"))
	{
		req->SendErrorResponse("missing request parameters");
		return;
	}

	const char* profile_name = obs_data_get_string(req->data, "profile-name");

	if (str_valid(profile_name))
	{
		// TODO : check if profile exists
		obs_frontend_set_current_profile(profile_name);
		req->SendOKResponse();
	}
	else
	{
		req->SendErrorResponse("invalid request parameters");
	}
}

void WSRequestHandler::HandleGetCurrentProfile(WSRequestHandler* req)
{
	obs_data_t* response = obs_data_create();
	obs_data_set_string(response, "profile-name",
		obs_frontend_get_current_profile());

	req->SendOKResponse(response);

	obs_data_release(response);
}

void WSRequestHandler::HandleListProfiles(WSRequestHandler* req)
{
	obs_data_array_t* profiles = Utils::GetProfiles();

	obs_data_t* response = obs_data_create();
	obs_data_set_array(response, "profiles", profiles);

	req->SendOKResponse(response);

	obs_data_release(response);
	obs_data_array_release(profiles);
}

void WSRequestHandler::HandleGetStudioModeStatus(WSRequestHandler* req)
{
	bool previewActive = Utils::IsPreviewModeActive();

	obs_data_t* response = obs_data_create();
	obs_data_set_bool(response, "studio-mode", previewActive);

	req->SendOKResponse(response);

	obs_data_release(response);
}

void WSRequestHandler::HandleGetPreviewScene(WSRequestHandler* req)
{
	if (!Utils::IsPreviewModeActive())
	{
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

void WSRequestHandler::HandleSetPreviewScene(WSRequestHandler* req)
{
	if (!Utils::IsPreviewModeActive())
	{
		req->SendErrorResponse("studio mode not enabled");
		return;
	}

	if (!req->hasField("scene-name"))
	{
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

void WSRequestHandler::HandleTransitionToProgram(WSRequestHandler* req)
{
	if (!Utils::IsPreviewModeActive())
	{
		req->SendErrorResponse("studio mode not enabled");
		return;
	}

	if (req->hasField("with-transition"))
	{
		obs_data_t* transitionInfo =
			obs_data_get_obj(req->data, "with-transition");

		if (obs_data_has_user_value(transitionInfo, "name"))
		{
			const char* transitionName =
				obs_data_get_string(transitionInfo, "name");

			if (!str_valid(transitionName))
			{
				req->SendErrorResponse("invalid request parameters");
				return;
			}

			bool success = Utils::SetTransitionByName(transitionName);
			if (!success)
			{
				req->SendErrorResponse("specified transition doesn't exist");
				obs_data_release(transitionInfo);
				return;
			}
		}

		if (obs_data_has_user_value(transitionInfo, "duration"))
		{
			int transitionDuration =
				obs_data_get_int(transitionInfo, "duration");

			Utils::SetTransitionDuration(transitionDuration);
		}

		obs_data_release(transitionInfo);
	}

	Utils::TransitionToProgram();
	req->SendOKResponse();
}

void WSRequestHandler::HandleEnableStudioMode(WSRequestHandler* req)
{
	Utils::EnablePreviewMode();
	req->SendOKResponse();
}

void WSRequestHandler::HandleDisableStudioMode(WSRequestHandler* req)
{
	Utils::DisablePreviewMode();
	req->SendOKResponse();
}

void WSRequestHandler::HandleToggleStudioMode(WSRequestHandler* req)
{
	Utils::TogglePreviewMode();
	req->SendOKResponse();
}

void WSRequestHandler::HandleGetSpecialSources(WSRequestHandler* req)
{
	obs_data_t* response = obs_data_create();

	QMap<const char*, int> sources;
	sources["desktop-1"] = 1;
	sources["desktop-2"] = 2;
	sources["mic-1"] = 3;
	sources["mic-2"] = 4;
	sources["mic-3"] = 5;

	QMapIterator<const char*, int> i(sources);
	while (i.hasNext())
	{
		i.next();

		const char* id = i.key();
		obs_source_t* source = obs_get_output_source(i.value());
		blog(LOG_INFO, "%s : %p", id, source);

		if (source)
		{
			obs_data_set_string(response, id, obs_source_get_name(source));
			obs_source_release(source);
		}
	}

	req->SendOKResponse(response);

	obs_data_release(response);
}