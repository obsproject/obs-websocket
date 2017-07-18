obs-websocket 4.1 protocol reference
================================
**This is the reference for the latest 4.1 developement build. [See here for obs-websocket 4.0.0!](https://github.com/Palakis/obs-websocket/blob/4.0.0/PROTOCOL.md)**

## General Introduction
Messages exchanged between the client and the server are JSON objects.  
The protocol in general is based on the OBS Remote protocol created by Bill Hamilton, with new commands specific to OBS Studio.

### Table of contents
* [Authentication](#authentication)
* [Events](#events)
  - [Description](#description)
  - [Event Types](#event-types)
    - **Scenes**
      - ["SwitchScenes"](#switchscenes)
      - ["ScenesChanged"](#sceneschanged)
    - **Scene Items**
      - ["SourceOrderChanged"](#sourceorderchanged)
      - ["SceneItemAdded"](#sceneitemadded)
      - ["SceneItemRemoved"](#sceneitemremoved)
      - ["SceneItemVisibilityChanged"](#sceneitemvisibilitychanged)
    - **Scene Collections**
      - ["SceneCollectionChanged"](#scenecollectionchanged)
      - ["SceneCollectionListChanged"](#scenecollectionlistchanged)
    - **Transitions**
      - ["SwitchTransition"](#switchtransition)
      - ["TransitionDurationChanged"](#transitiondurationchanged)
      - ["TransitionListChanged"](#transitionlistchanged)
      - ["TransitionBegin"](#transitionbegin)
    - **Studio Mode**
      - ["PreviewSceneChanged"](#previewscenechanged)
      - ["StudioModeSwitched"](#studiomodeswitched)
    - **Profiles**
      - ["ProfileChanged"](#profilechanged)
      - ["ProfileListChanged"](#profilelistchanged)
    - **Streaming**
      - ["StreamStarting"](#streamstarting)
      - ["StreamStarted"](#streamstarted)
      - ["StreamStopping"](#streamstopping)
      - ["StreamStopped"](#streamstopped)
      - ["StreamStatus"](#streamstatus)
    - **Recording**
      - ["RecordingStarting"](#recordingstarting)
      - ["RecordingStarted"](#recordingstarted)
      - ["RecordingStopping"](#recordingstopping)
      - ["RecordingStopped"](#recordingstopped)
    - **Other**
      - ["Exiting"](#exiting)
* [Requests](#requests)
  - [Description](#description-1)
  - [Request Types](#request-types)
    - **General**
      - ["GetVersion"](#getversion)
      - ["GetAuthRequired"](#getauthrequired)
      - ["Authenticate"](#authenticate)
    - **Scenes**
      - ["GetCurrentScene"](#getcurrentscene)
      - ["SetCurrentScene"](#setcurrentscene)
      - ["GetSceneList"](#getscenelist)
    - **Studio Mode**
      - ["GetStudioModeStatus"](#getstudiomodestatus)
      - ["SetPreviewScene"](#setpreviewscene)
      - ["TransitionToProgram"](#transitiontoprogram)
      - ["EnableStudioMode"](#enablestudiomode)
      - ["DisableStudioMode"](#disablestudiomode)
      - ["ToggleStudioMode"](#togglestudiomode)
    - **Streaming**
      - ["StartStopStreaming"](#startstopstreaming)
      - ["StartStreaming"](#startstreaming)
      - ["StopStreaming"](#stopstreaming)
      - ["GetStreamingStatus"](#getstreamingstatus)
    - **Recording**
      - ["StartStopRecording"](#startstoprecording)
      - ["StartRecording"](#startrecording)
      - ["StopRecording"](#stoprecording)
      - ["GetStreamingStatus"](#getstreamingstatus)
      - ["SetRecordingFolder"](#setrecordingfolder)
      - ["GetRecordingFolder"](#getrecordingfolder)
    - **Transitions**
      - ["GetTransitionList"](#gettransitionlist)
      - ["GetCurrentTransition"](#getcurrenttransition)
      - ["SetCurrentTransition"](#setcurrenttransition)
      - ["GetTransitionDuration"](#gettransitionduration)
      - ["SetTransitionDuration"](#settransitionduration)
    - **Sources**
      - ["GetCurrentScene"](#getcurrentscene)
      - ["GetSceneList"](#getscenelist)
      - ["GetSpecialSources"](#getspecialsources)
      - ["GetTextGDIPlusProperties"](#gettextgdiplusproperties)
      - ["SetTextGDIPlusProperties"](#settextgdiplusproperties)
      - ["GetBrowserSourceProperties"](#getbrowsersourceproperties)
      - ["SetBrowserSourceProperties"](#setbrowsersourceproperties)
      - ["SetVolume"](#setvolume)
      - ["GetVolume"](#getvolume)
      - ["SetMute"](#setmute)
      - ["GetMute"](#getmute)
      - ["ToggleMute"](#togglemute)
    - **Scene Items**
      - ["SetSceneItemRender"](#setsourcerender) (a.k.a `SetSourceRender`)
      - ["SetSceneItemPosition"](#setsceneitemposition)
      - ["SetSceneItemTransform"](#setsceneitemtransform)
      - ["SetSceneItemCrop"](#setsceneitemcrop)
      - ["ResetSceneItem"](#resetsceneitem)
    - **Scene Collections**
      - ["ListSceneCollections"](#listscenecollections)
      - ["SetCurrentSceneCollection"](#setcurrentscenecollection)
      - ["GetCurrentSceneCollection"](#getcurrentscenecollection)
    - **Streaming Server Settings**
      - ["GetStreamSettings"](#getstreamsettings)
      - ["SetStreamSettings"](#setstreamsettings)
      - ["SaveStreamSettings"](#savestreamsettings)
    - **Profiles**
      - ["ListProfiles"](#listprofiles)
      - ["SetCurrentProfile"](#setcurrentprofile)
      - ["GetCurrentProfile"](#getcurrentprofile)

## Authentication
A call to [`GetAuthRequired`](#getauthrequired) gives the client two elements :
- A challenge : a random string that will be used to generate the auth response
- A salt : applied to the password when generating the auth response

The client knows a password and must it to authenticate itself to the server.  
However, it must keep this password secret, and it is the purpose of the authentication mecanism used by obs-websocket.

After a call to [`GetAuthRequired`](#getauthrequired), the client knows a password (kept secret), a challenge and a salt (sent by the server).
To generate the answer to the auth challenge, follow this procedure :
- Concatenate the password with the salt sent by the server (in this order : password + server salt), then generate a binary SHA256 hash of the result and encode the resulting SHA256 binary hash to base64.
- Concatenate the base64 secret with the challenge sent by the server (in this order : base64 secret + server challenge), then generate a binary SHA256 hash of the result and encode it to base64.
- Voilà, this last base64 string is the auth response. You may now use it to authenticate to the server with the `Authenticate` request.

Here's how it looks in pseudocode :
```
password = "supersecretpassword"
challenge = "ztTBnnuqrqaKDzRM3xcVdbYm"
salt = "PZVbYpvAnZut2SS6JNJytDm9"

secret_string = password + salt
secret_hash = binary_sha256(secret_string)
secret = base64_encode(secret_hash)

auth_response_string = secret + challenge
auth_response_hash = binary_sha256(auth_response_string)
auth_response = base64_encode(auth_response_hash)
```

A client can then authenticate to the server by calling [`Authenticate`](#authenticate) with the computed challenge response.

## Events
### Description
Events are sent exclusively by the server and broadcast to each connected client.  
An event message will contain at least one field :
- **update-type** (string) : the type of event
- **stream-timecode** (string, optional) : time elapsed between now and stream start (only present if OBS Studio is streaming)
- **rec-timecode** (string, optional) : time elapsed between now and recording start (only present if OBS Studio is recording)

Timecodes are in the following format : HH:MM:SS.mmm

Additional fields will be present in the event message depending on the event type.

### Event Types
#### "SwitchScenes"
OBS is switching to another scene (called at the end of the transition).  
- **scene-name** (string) : The name of the scene being switched to.
- **sources** (array of objects) : List of sources composing the scene. Same specification as [`GetCurrentScene`](#getcurrentscene).

---

#### "ScenesChanged"
The scene list has been modified (Scenes have been added, removed, or renamed).

---

#### "SourceOrderChanged"
Scene items have been reordered.
- **"scene-name"** (string) : name of the scene where items have been reordered

---

#### "SceneItemAdded"
An item has been added to the current scene.
- **"scene-name"** (string) :  name of the scene
- **"item-name"** (string) : name of the item added to **scene-name**

---

#### "SceneItemRemoved"
An item has been removed from the current scene.
- **"scene-name"** (string) :  name of the scene
- **"item-name"** (string) : name of the item removed from **scene-name**

---

#### "SceneItemVisibilityChanged"
An item's visibility has been toggled.
- **"scene-name"** (string) :  name of the scene
- **"item-name"** (string) : name of the item in **scene-name**
- **"item-visible"** (bool) : new visibility of item **item-name**

---

#### "SceneCollectionChanged"
Triggered when switching to another scene collection or when renaming the current scene collection.

---

#### "SceneCollectionListChanged"
Triggered when a scene collection is created, added, renamed or removed.

---

#### "SwitchTransition"
The active transition has been changed.  
- **transition-name** (string) : The name of the active transition.

---

#### "TransitionDurationChanged"
Triggered when the transition duration has changed.
- **"new-duration"** (integer) : new transition duration

---

#### "TransitionListChanged"
The list of available transitions has been modified (Transitions have been added, removed, or renamed).

---

#### "TransitionBegin"
A transition other than "Cut" has begun.

---

#### "PreviewSceneChanged"
The selected Preview scene changed (only in Studio Mode).
- **scene-name** (string) : Name of the scene being previewed.
- **sources** (array of objects) : List of sources composing the scene. Same specification as [`GetCurrentScene`](#getcurrentscene).

---

#### "StudioModeSwitched"
Studio Mode has been switched on or off.
- **"new-state"** (bool) : new state of Studio Mode: true if enabled, false if disabled.

---

#### "ProfileChanged"
Triggered when switching to another profile or when renaming the current profile.

---

#### "ProfileListChanged"
Triggered when a profile is created, added, renamed or removed.

---

#### "StreamStarting"
A request to start streaming has been issued.  
- **preview-only** (bool) : Always false.

---

#### "StreamStarted"  
Streaming started successfully.  

---

#### "StreamStopping"
A request to stop streaming has been issued.  
- **preview-only** (bool) : Always false.

---

#### "StreamStopped"  
Streaming stopped successfully.  

---

#### "RecordingStarting"  
A request to start recording has been issued.  

---

#### "RecordingStarted"  
Recording started successfully.  

---

#### "RecordingStopping"
A request to stop streaming has been issued.  

---

#### "RecordingStopped"  
Recording stopped successfully.  

---

#### "StreamStatus"
Sent every 2 seconds with the following information :  
- **streaming** (bool) : Current Streaming state.
- **recording** (bool) : Current Recording state.
- **preview-only** (bool) : Always false.
- **bytes-per-sec** (integer) : Amount of data per second (in bytes) transmitted by the stream encoder.
- **kbits-per-sec** (integer) : "bytes-per-sec" converted to kilobits per second
- **strain** (double) : Percentage of dropped frames
- **total-stream-time** (integer) : Total time (in seconds) since the stream started.
- **num-total-frames** (integer) : Total number of frames transmitted since the stream started.
- **num-dropped-frames** (integer) : Number of frames dropped by the encoder since the stream started.
- **fps** (double) : Current framerate.

---

#### "Exiting"
OBS is exiting.  

---

## Requests

### Description
Requests are sent by the client and must have at least the following two fields :  
- **"request-type"** (string) : One of the request types listed in the sub-section "[Requests Types](#request-types)".
- **"message-id"** (string) : An identifier defined by the client which will be embedded in the server response.  

Depending on the request type additional fields may be required (see the "[Request Types](#request-types)" section below for more information).

Once a request is sent, the server will return a JSON response with the following fields :  
- **"message-id"** (string) : The identifier specified in the request.
- **"status"** (string) : Response status, will be one of the following : "ok", "error"
- **"error"** (string) : The error message associated with an "error" status.  

Depending on the request type additional fields may be present (see the "[Request Types](#request-types)" section below for more information).

### Request Types
#### "GetVersion"
Returns the latest version of the plugin and the API.

__Request fields__ : none  
__Response__ : always OK, with these additional fields :  
- **"version"** (double) : OBSRemote API version. Fixed to 1.1 for retrocompatibility.
- **"obs-websocket-version"** (string) : obs-websocket version string
- **"obs-studio-version"** (string) : OBS Studio version string

---

#### "GetAuthRequired"
Tells the client if authentication is required. If it is, authentication parameters "challenge" and "salt" are passed in the response fields (see "Authentication").

__Request fields__ : none  
__Response__ : always OK, with these additional fields :  
- **"authRequired"** (bool)
- **"challenge"** (string)
- **"salt"** (string)

---

#### "Authenticate"
Try to authenticate the client on the server.

__Request fields__ :  
- **"auth"** (string) : response to the auth challenge (see "Authentication").

__Response__ : OK if auth succeeded, error if invalid credentials. No additional fields.

---

#### "GetCurrentScene"
Get the current scene's name and items.

__Request fields__ : none  
__Response__ : always OK, with these additional fields :  
- **"name"** (string) : name of the current scene
- **"sources"** (array of objects) : ordered list of the current scene's items descriptions

Objects in the "sources" array have the following fields :
- **"name"** (string) : name of the source associated with the scene item
- **"type"** (string) : internal source type name
- **"volume"** (double) : audio volume of the source, ranging from 0.0 to 1.0
- **"x"** (double) : X coordinate of the top-left corner of the item in the scene
- **"y"** (double) : Y coordinate of the top-left corner of the item in the scene
- **"source_cx"** (integer) : width of the item (without scale applied)
- **"source_cy"** (integer) : height of the item (without scale applied)
- **"cx"** (double) : width of the item (with scale applied)
- **"cy"** (double) : height of the item (with scale applied)
- **"render"** (bool) : visibility of the source in the scene

---

#### "SetCurrentScene"
Switch to the scene specified in "scene-name".

__Request fields__ :  
- **"scene-name"** (string) : name of the scene to switch to.

__Response__ : always OK if scene exists, error if it doesn't. No additional fields

---

#### "GetSceneList"
List OBS' scenes.

__Request fields__ : none  
__Response__ : always OK, with these additional fields :  
- **"current-scene"** (string) : name of the currently active scene
- **"scenes"** (array of objects) : ordered list of scene descriptions (see `GetCurrentScene` for reference)

---

#### "SetSourceRender"
Show or hide a specific source in the current scene.

__Request fields__ :  
- **"source"** (string) : name of the source in the currently active scene.
- **"render"** (bool) : desired visibility
- **"scene-name"** (string; optional) : name of the scene the source belongs to.  defaults to current scene.

__Response__ : OK if source exists in the current scene, error otherwise.

---

#### "GetStudioModeStatus"
Tells if Studio Mode is currently enabled or disabled.

__Request fields__ : none  
__Response__ : always OK, with these additional fields :  
- **"studio-mode"** (bool) : true if OBS is in Studio Mode, false otherwise.

---

#### "GetPreviewScene"
Studio Mode only. Gets the name of the currently Previewed scene, along with a list of its sources.

__Request fields__ : none  
__Response__ : OK if Studio Mode is enabled, with the same fields as [`GetCurrentScene`](#getcurrentscene), error otherwise.

---

#### "SetPreviewScene"
Studio Mode only. Sets the specified scene as the Previewed scene in Studio Mode.

__Request fields__ :  
- **"scene-name"** (string) : name of the scene to selected as the preview of Studio Mode

__Response__ : OK if Studio Mode is enabled and specified scene exists, error otherwise.

---

#### "TransitionToProgram"
Studio Mode only. Transitions the currently previewed scene to Program (main output).

__Request fields__ :  
- **"with-transition"** (object, optional) : if specified, use this transition when switching from preview to program. This will change the current transition in the frontend to this one.

__Response__ : OK if studio mode is enabled and optional transition exists, error otherwise.

An object passed as `"with-transition"` in a request must have the following fields :  
- **"name"** (string, optional) : transition name
- **"duration"** (integer, optional) : transition duration in milliseconds

---

#### "EnableStudioMode"
Enables Studio Mode.

__Request fields__ : none  
__Response__ : always OK. No additional fields.

---

#### "DisableStudioMode"
Disables Studio Mode.

__Request fields__ : none  
__Response__ : always OK. No additional fields.

---

#### "ToggleStudioMode"
Toggles Studio Mode on or off.

__Request fields__ : none  
__Response__ : always OK. No additional fields.

---

#### "StartStopStreaming"
Toggles streaming on or off.

__Request fields__ : none  
__Response__ : always OK. No additional fields.

---

#### "StartStopRecording"
Toggles recording on or off.

__Request fields__ : 
- **"stream"** (object; optional) : See 'stream' parameter in 'StartStreaming'.  Ignored if stream is already started.

__Response__ : always OK. No additional fields.  

---

#### "StartStreaming"
Start streaming.

__Request fields__ : 
- **"stream"** (object; optional) : If specified allows for special configuration of the stream
 
The 'stream' object has the following fields:
- **"settings"** (object; optional) : The settings for the stream
- **"type"** (string; optional) : If specified ensures the type of the stream matches the given type (usually 'rtmp\_custom' or 'rtmp\_common').  If the currently configured stream type does not match the given stream type, all settings must be specified in the 'settings' object or an error will occur starting the stream.
- **"metadata"** (object; optional) : Adds the given object parameters as encoded query string parameters to the 'key' of the RTMP stream.  Used to pass data to the RTMP service about the stream.

The 'settings' object has the following fields:
- **"server"** (string; optional) : The publish URL
- **"key"** (string; optional) : The publish key of the stream
- **"use-auth"** (bool; optional) : should authentication be used when connecting to the streaming server
- **"username"** (string; optional) : if authentication is enabled, the username for access to the streaming server. Ignored if 'use-auth' is not specified as 'true'.
- **"password"** (string; optional) : if authentication is enabled, the password for access to the streaming server. Ignored if 'use-auth' is not specified as 'true'.

The 'metadata' object supports passing any string, numeric or boolean field.
  
__Response__ : Error if streaming is already active, OK otherwise. No additional fields.  

---

#### "StopStreaming"
Stop streaming.

__Request fields__ : none  
__Response__ : Error if streaming is already inactive, OK otherwise. No additional fields.  

---

#### "StartRecording"
Start recording.

__Request fields__ : none  
__Response__ : Error if recording is already active, OK otherwise. No additional fields.  

---

#### "StopRecording"
Stop recording.

__Request fields__ : none  
__Response__ : Error if recording is already inactive, OK otherwise. No additional fields.  

---

#### "SetRecordingFolder"
Change the current recording folder.

__Request fields__ :
- **"rec-folder"** (string) : path of the desired recording folder

__Response__ : OK if path is valid, error otherwise.

---

#### "GetRecordingFolder"
Get the path of the current recording folder.

__Request fields__ : none

__Response__ : OK with these additional fields :
- **"rec-folder"** (string) : path of the current recording folder

---

#### "GetStreamingStatus"
Get current streaming and recording status.

__Request fields__ : none  
__Response__ : always OK, with these additional fields :
- **"streaming"** (bool) : streaming status (active or not)
- **"recording"** (bool) : recording status (active or not)
- **stream-timecode** (string, optional) : time elapsed between now and stream start (only present if OBS Studio is streaming)
- **rec-timecode** (string, optional) : time elapsed between now and recording start (only present if OBS Studio is recording)
- **"preview-only"** (bool) : always false. Retrocompat with OBSRemote.

---

#### "GetTransitionList"
List all transitions available in the frontend's dropdown menu.

__Request fields__ : none  
__Response__ : always OK, with these additional fields :
- **"current-transition"** (string) : name of the current transition
- **"transitions"** (array of objects) : list of transition descriptions

Objects in the "transitions" array have only one field :
- **"name"** (string) : name of the transition

---

#### "GetCurrentTransition"
Get the name of the currently selected transition in the frontend's dropdown menu.

__Request fields__ : none  
__Response__ : always OK, with these additional fields :
- **"name"** (string) : name of the selected transition
- **"duration"** (integer, only if transition supports this) : transition duration

---

#### "SetCurrentTransition"
__Request fields__ :
- **"transition-name"** (string) : The name of the transition.

__Response__ : OK if specified transition exists, error otherwise. 

---

#### "SetTransitionDuration"
Set the duration of the currently selected transition.

__Request fields__ :
- **"duration"** (integer) : desired transition duration in milliseconds

__Response__ : always OK.

---

#### "GetTransitionDuration"
Set the duration of the currently selected transition.

__Request fields__ : none  
__Response__ : always OK, with these additional fields :
- **"transition-duration"** (integer) : current transition duration, in milliseconds

---

#### "SetVolume"
Set the volume of a specific source.

__Request fields__ :
- **"source"** (string) : the name of the source
- **"volume"** (double) : the desired volume

__Response__ : OK if specified source exists, error otherwise.

---

#### "GetVolume"
Get the volume of a specific source.

__Request fields__ :
- **"source"** (string) : name of the source

__Response__ : OK if source exists, with these additional fields :
- **"name"** (string) : source name
- **"volume"** (double) : source volume, on a linear scale (0.0 to 1.0)
- **"muted"** (bool) : source mute status

---

#### "SetMute"
Mutes or unmutes a specific source.

__Request fields__ :
- **"source"** (string) : the name of the source
- **"mute"** (bool) : the desired mute status

__Response__ : OK if specified source exists, error otherwise.

---

#### "GetMute"
Get mute status of a specific source.

__Request fields__ :
- **"source"** (string) : the name of the source

__Response__ : OK if source exists, with these additional fields :
- **"name"** (string) : source name
- **"muted"** (bool) : source mute status

---

#### "ToggleMute"
Inverts the mute status of a specific source.

__Request fields__ :
- **"source"** (string) : the name of the source

__Response__ : OK if specified source exists, error otherwise.

---

#### "GetSpecialSources"
Get configured special sources like Desktop Audio and Mic/Aux sources.

__Request fields__ : none

__Response__ : always OK, with these additional fields :  
- **"desktop-1"** (string, optional) : Name of the first Desktop Audio capture source
- **"desktop-1"** (string, optional) : Name of the second Desktop Audio capture source
- **"mic-1"** (string, optional) : Name of the first Mic/Aux input source
- **"mic-2"** (string, optional) : Name of the second Mic/Aux input source
- **"mic-3"** (string, optional) : Name of the third Mic/Aux input source

---

#### "SetSceneItemPosition"
__Request fields__ :
- **"item"** (string) : The name of the scene item.
- **"x"** (float) : x coordinate
- **"y"** (float) : y coordinate
- **"scene-name"** (string) : scene the item belongs to.  defaults to current scene.

__Response__ : OK if specified item exists, error otherwise.

---

#### "SetSceneItemTransform"
__Request fields__ :
- **"item"** (string) : The name of the scene item.
- **"x-scale"** (float) : width scale factor
- **"y-scale"** (float) : height scale factor
- **"rotation"** (float) : item rotation (in degrees)
- **"scene-name"** (string) : scene the item belongs to.  defaults to current scene.

__Response__ : OK if specified item exists, error otherwise.

---

#### "SetSceneItemCrop"
__Request fields__ :
- **"item"** (string) : Name of the scene item
- **"scene-name"** (string, optional) : Scene the item belongs to. Default : current scene.
- **"top"** (integer)
- **"bottom"** (integer)
- **"left"** (integer)
- **"right"** (integer)

__Response__ : OK if specified item exists, error otherwise.

---

#### "ResetSceneItem"
__Request fields__ :
- **"item"** (string) : Name of the scene item
- **"scene-name"** (string, optional) : Scene the item belongs to. Default : current scene.

__Response__ : OK if specified item exists, error otherwise.

---

#### "SetCurrentSceneCollection"
Change the current scene collection.

__Request fields__ :
- **"sc-name"** (string) : name of the desired scene collection

__Response__ : OK if scene collection exists, error otherwise.

---

#### "GetCurrentSceneCollection"
Get the name of the current scene collection.

__Request fields__ : none

__Response__ : OK with these additional fields :
- **"sc-name"** (string) : name of the current scene collection

---

#### "ListSceneCollections"
Get a list of available scene collections.

__Request fields__ : none

__Response__ : OK with these additional fields :
- **"scene-collections"** (array of objects) : names of available scene collections

---

#### "GetStreamSettings"
Gets the current streaming server settings

__Request fields__ : none

__Response__ : OK with these additional fields :
- **"type"** (string) : The type of streaming service configuration usually 'rtmp\_custom' or 'rtmp\_common'
- **"settings"** (object) : The actual settings of the stream (i.e. server, key, use-auth, username, password)

The 'settings' object has the following fields however they may vary by 'type':
- **"server"** (string) : The publish URL
- **"key"** (string) : The publish key of the stream
- **"use-auth"** (bool) : should authentication be used when connecting to the streaming server
- **"username"** (string) : if authentication is enabled, the username for access to the streaming server
- **"password"** (string) : if authentication is enabled, the password for access to the streaming server

--

#### "SetStreamSettings"
Sets one or more attributes of the current streaming server settings. Any options not passed will remain unchanged. Returns the updated settings in response. 
If 'type' is different than the current streaming service type, all settings are required.
Returns the full settings of the stream (i.e. the same as GetStreamSettings)

__Request fields__ :
- **"type"** (string) : The type of streaming service configuration usually 'rtmp\_custom' or 'rtmp\_common'
- **"settings"** (object) : The actual settings of the stream (i.e. server, key, use-auth, username, password)
- **"save"** (bool) : If specified as true, saves the settings to disk

The 'settings' object has the following fields however they may vary by 'type':
- **"server"** (string; optional) : The publish URL
- **"key"** (string; optional) : The publish key of the stream
- **"use-auth"** (bool; optional) : should authentication be used when connecting to the streaming server
- **"username"** (string; optional) : if authentication is enabled, the username for access to the streaming server
- **"password"** (string; optional) : if authentication is enabled, the password for access to the streaming server

__Response__ : OK with the same fields as the request (except 'save')

---

#### "SaveStreamSettings"
Saves the current streaming server settings to disk

__Request fields__ : none

__Response__ : OK


#### "SetCurrentProfile"
Change the current profile.

__Request fields__ :
- **"profile-name"** (string) : name of the desired profile

__Response__ : OK if profile exists, error otherwise.

---

#### "GetCurrentProfile"
Get the name of the current profile.

__Request fields__ : none

__Response__ : OK with these additional fields :
- **"profile-name"** (string) : name of the current profile

---

#### "ListProfiles"
Get a list of available profiles.

__Request fields__ : none

__Response__ : OK with the additional fields :
- **"profiles"** (array of objects) : names of available profiles

---

#### "GetTextGDIPlusProperties"
Gets current properties for Text GDI Plus source.

__Request fields__ :  
- **"source"** (string) : name of the source in the currently active scene.
- **"scene-name"** (string; optional) : name of the scene the source belongs to.  defaults to current scene.

__Response__ : OK if source exists in the current scene with these additional fields when fields are set, error otherwise.
- **"align"** (string) : "left","center","right" : text alignment
- **"bk_color"** (integer) : background color
- **"bk_opacity"** (integer) : background opacity range 0 to 100
- **"chatlog"** (bool) : chat log 
- **"chatlog_lines"** (integer) : chat log lines
- **"color"** (integer) : text color
- **"extents"** (bool) : extents 
- **"extents_wrap"** (bool) : extents wrap 
- **"extents_cx"** (integer) : extents cx
- **"extents_cy"** (integer) : extents cy
- **"file"** (string) : file path name
- **"read_from_file"** (bool) : read text from file specified
- **"font"** (object) : holds font data for face, flags, size and style
-- Example: "font": {"face": "Arial","flags": 0,"size": 150,"style": ""}
- **"face"** (string) : font face i.e. Arial
- **"flags"** (integer) : font text style flag i.e. Bold 1, Italic 2, Bold Italic 3, Underline 5, Strikeout 8
- **"size"** (integer) : font text size
- **"style"** (string) : font style (unknown function)
- **"gradient"** (bool) : gradient 
- **"gradient_color"** (integer) : gradient color
- **"gradient_dir"** (float) : gradient direction
- **"gradient_opacity"** (integer) : gradient opacity range 0 to 100
- **"outline"** (bool) : outline 
- **"outline_color"** (integer) : outline color
- **"outline_size"** (integer) : outline size
- **"outline_opacity"** (integer) : outline opacity range 0 to 100
- **"text"** (string) : text to be displayed
- **"valign"** (string) : "top","center","bottom" : text vertical alignment
- **"vertical"** (bool) : vertical text 
- **"render"** (bool) : visibility of the scene item

---

#### "SetTextGDIPlusProperties"
Sets current properties for Text GDI Plus source.

__Request fields__ :  
- **"source"** (string) : name of the source in the currently active scene.
- **"scene-name"** (string; optional) : name of the scene the source belongs to.  defaults to current scene.
- **"align"** (string; optional) : "left","center","right" : text alignment
- **"bk_color"** (integer; optional) : background color
- **"bk_opacity"** (integer; optional) : background opacity range 0 to 100
- **"chatlog"** (bool; optional) : chat log 
- **"chatlog_lines"** (integer; optional) : chat log lines
- **"color"** (integer; optional) : text color
- **"extents"** (bool; optional) : extents 
- **"extents_wrap"** (bool; optional) : extents wrap 
- **"extents_cx"** (integer; optional) : extents cx
- **"extents_cy"** (integer; optional) : extents cy
- **"file"** (string; optional) : file path name
- **"read_from_file"** (bool; optional) : read text from file specified
- **"font"** (object; optional) : holds font data for face, flags, size and style
-- Example: "font":{"face": "Arial","flags": 0,"size": 150,"style": ""}
- **"face"** (string; optional) : font face i.e. Arial
-- Example: "font":{"face": "Arial"}
- **"flags"** (integer; optional) : font text style flag i.e. Bold 1, Italic 2, Bold Italic 3, Underline 5, Strikeout 8
- **"size"** (integer; optional) : font text size 
-- Example: "font": {"size":125}
- **"style"** (string; optional) : font style (unknown function)
- **"gradient"** (bool; optional) : gradient 
- **"gradient_color"** (integer; optional) : gradient color
- **"gradient_dir"** (float; optional) : gradient direction
- **"gradient_opacity"** (integer; optional) : gradient opacity range 0 to 100
- **"outline"** (bool; optional) : outline 
- **"outline_color"** (integer; optional) : outline color
- **"outline_size"** (integer; optional) : outline size
- **"outline_opacity"** (integer; optional) : outline opacity range 0 to 100
- **"text"** (string; optional) : text to be displayed
- **"valign"** (string; optional) : "top","center","bottom" : text vertical alignment
- **"vertical"** (bool; optional) : vertical text 
- **"render"** (bool; optional) : visibility of the scene item

__Response__ : OK if source exists in the current scene, error otherwise.

---

#### "GetBrowserSourceProperties"
Gets current properties for Browser Source.

__Request fields__ :  
- **"source"** (string) : name of the source in the currently active scene.
- **"scene-name"** (string; optional) : name of the scene the source belongs to.  defaults to current scene.

__Response__ : OK if source exists in the current scene with these additional fields when fields are set, error otherwise.

- **"is_local_file"** (bool) : use local file
- **"url"** (string) : url or file path
- **"css"** (string) : cascading style sheet code
- **"width"** (integer) : width
- **"height"** (integer) : height
- **"fps"** (integer) : frames per second 
- **"shutdown"** (bool) : shutdown when sorce is not visible
- **"render"** (bool; optional) : visibility of the scene item

---

#### "SetBrowserSourceProperties"
Sets current properties for Browser Source.

__Request fields__ :  
- **"source"** (string) : name of the source in the currently active scene.
- **"scene-name"** (string; optional) : name of the scene the source belongs to.  defaults to current scene.
- **"is_local_file"** (bool; optional) : use local file
- **"url"** (string; optional) : url or file path
- **"css"** (string; optional) : cascading style sheet code
- **"width"** (integer; optional) : width
- **"height"** (integer; optional) : height
- **"fps"** (integer; optional) : frames per second 
- **"shutdown"** (bool; optional) : shutdown when sorce is not visible
- **"render"** (bool; optional) : visibility of the scene item

---
