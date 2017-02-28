obs-websocket protocol reference
================================

## General Introduction
Messages exchanged between the client and the server are JSON objects.  
The protocol in general is based on the OBS Remote protocol created by Bill Hamilton, with new commands specific to OBS Studio.

### Table of contents
* [Events](#events)
  - [Description](#description)
  - [Event Types](#event-types)
    - ["SwitchScenes"](#switchscenes)
    - ["ScenesChanged"](#sceneschanged)
    - ["SourceOrderChanged"](#sourceorderchanged)
    - ["SceneItemAdded"](#sceneitemadded)
    - ["SceneItemRemoved"](#sceneitemremoved)
    - ["SceneCollectionChanged"](#scenecollectionchanged)
    - ["SceneCollectionListChanged"](#scenecollectionlistchanged)
    - ["SwitchTransition"](#switchtransition)
    - ["TransitionDurationChanged"](#transitiondurationchanged)
    - ["TransitionListChanged"](#transitionlistchanged)
    - ["TransitionBegin"](#transitionbegin)
    - ["ProfileChanged"](#profilechanged)
    - ["ProfileListChanged"](#profilelistchanged)
    - ["StreamStarting"](#streamstarting)
    - ["StreamStarted"](#streamstarted)
    - ["StreamStopping"](#streamstopping)
    - ["StreamStopped"](#streamstopped)
    - ["RecordingStarting"](#recordingstarting)
    - ["RecordingStarted"](#recordingstarted)
    - ["RecordingStopping"](#recordingstopping)
    - ["RecordingStopped"](#recordingstopped)
    - ["StreamStatus"](#streamstatus)
    - ["Exiting"](#exiting)
* [Requests](#requests)
  - [Description](#description-1)
  - [Request Types](#request-types)
    - ["GetVersion"](#getversion)
    - ["GetAuthRequired"](#getauthrequired)
    - ["Authenticate"](#authenticate)
    - ["GetCurrentScene"](#getcurrentscene)
    - ["SetCurrentScene"](#setcurrentscene)
    - ["GetSceneList"](#getscenelist)
    - ["SetSourceRender"](#setsourcerender)
    - ["StartStopStreaming"](#startstopstreaming)
    - ["StartStopRecording"](#startstoprecording)
    - ["GetStreamingStatus"](#getstreamingstatus)
    - ["GetTransitionList"](#gettransitionlist)
    - ["GetCurrentTransition"](#getcurrenttransition)
    - ["SetCurrentTransition"](#setcurrenttransition)
    - ["SetTransitionDuration"](#settransitionduration)
    - ["SetVolume"](#setvolume)
    - ["GetVolume"](#getvolume)
    - ["SetMute"](#setmute)
    - ["ToggleMute"](#togglemute)
    - ["SetSceneItemPosition"](#setsceneitemposition)
    - ["SetSceneItemTransform"](#setsceneitemtransform)
    - ["SetCurrentSceneCollection"](#setcurrentscenecollection)
    - ["GetCurrentSceneCollection"](#getcurrentscenecollection)
    - ["ListSceneCollections"](#listscenecollections)
    - ["SetCurrentProfile"](#setcurrentprofile)
    - ["GetCurrentProfile"](#getcurrentprofile)
    - ["ListProfiles"](#listprofiles)
* [Authentication](#authentication)

## Events
### Description
Events are sent exclusively by the server and broadcast to each connected client.  
An event message will contain at least one field :
- **update-type** (string) : the type of event  

Additional fields will be present in the event message depending on the event type.

### Event Types
#### "SwitchScenes"
OBS is switching to another scene (called at the end of the transition).  
- **scene-name** (string) : The name of the scene being switched to.

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
*New in OBS Studio*

---

#### "StreamStopping"
A request to stop streaming has been issued.  
- **preview-only** (bool) : Always false.

---

#### "StreamStopped"  
Streaming stopped successfully.  
*New in OBS Studio*

---

#### "RecordingStarting"  
A request to start recording has been issued.  
*New in OBS Studio*

---

#### "RecordingStarted"  
Recording started successfully.  
*New in OBS Studio*

---

#### "RecordingStopping"
A request to stop streaming has been issued.  
*New in OBS Studio*

---

#### "RecordingStopped"  
Recording stopped successfully.  
*New in OBS Studio*

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
*New in OBS Studio*  

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

#### "StartStopStreaming"
Toggle streaming on or off.

__Request fields__ : none  
__Response__ : always OK. No additional fields.

---

#### "StartStopRecording"
Toggle recording on or off.

__Request fields__ : none  
__Response__ : always OK. No additional fields.  
*New in OBS Studio*

---

#### "GetStreamingStatus"
Get current streaming and recording status.

__Request fields__ : none  
__Response__ : always OK, with these additional fields :
- **"streaming"** (bool) : streaming status (active or not)
- **"recording"** (bool) : recording status (active or not)
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

*New in OBS Studio*  

---

#### "GetCurrentTransition"
Get the name of the currently selected transition in the frontend's dropdown menu.

__Request fields__ : none  
__Response__ : always OK, with these additional fields :
- **"name"** (string) : name of the selected transition
- **"duration"** (integer, only if transition supports this) : transition duration

*New in OBS Studio*  

---

#### "SetCurrentTransition"
__Request fields__ :
- **"transition-name"** (string) : The name of the transition.

__Response__ : OK if specified transition exists, error otherwise.

*New in OBS Studio*  

---

#### "SetTransitionDuration"
Set the duration of the currently selected transition.

__Request fields__ :
- **"duration"** (integer) : desired transition duration in milliseconds

__Response__ : always OK.

*New in OBS Studio*

---

#### "SetVolume"
Set the volume of a specific source.

__Request fields__ :
- **"source"** (string) : the name of the source
- **"volume"** (double) : the desired volume

__Response__ : OK if specified source exists, error otherwise.

*Updated for OBS Studio*

---

#### "GetVolume"
Get the volume of a specific source.

__Request fields__ :
- **"source"** (string) : name of the source

__Response__ : OK if source exists, with these additional fields :
- **"name"** (string) : name of the requested source
- **"volume"** (double) : volume of the requested source, on a linear scale (0.0 to 1.0)
- **"muted"** (bool) : mute status of the requested source

*Updated for OBS Studio*

---

#### "SetMute"
Mutes or unmutes a specific source.

__Request fields__ :
- **"source"** (string) : the name of the source
- **"mute"** (bool) : the desired mute status

__Response__ : OK if specified source exists, error otherwise.

*Updated for OBS Studio*

---

#### "ToggleMute"
Inverts the mute status of a specific source.

__Request fields__ :
- **"source"** (string) : the name of the source

__Response__ : OK if specified source exists, error otherwise.

*Updated for OBS Studio*

---

#### "SetSceneItemPosition"
__Request fields__ :
- **"item"** (string) : The name of the scene item.
- **"x"** (float) : x coordinate
- **"y"** (float) : y coordinate
- **"scene_name"** (string) : scene the item belongs to.  defaults to current scene.

__Response__ : OK if specified item exists, error otherwise.

*New in OBS Studio*

---

#### "SetSceneItemTransform"
__Request fields__ :
- **"item"** (string) : The name of the scene item.
- **"x-scale"** (float) : width scale factor
- **"y-scale"** (float) : height scale factor
- **"rotation"** (float) : item rotation (in degrees)
- **"scene_name"** (string) : scene the item belongs to.  defaults to current scene.

__Response__ : OK if specified item exists, error otherwise.

*New in OBS Studio*

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

### Authentication
A call to `GetAuthRequired` gives the client two elements :
- A challenge : a random string that will be used to generate the auth response
- A salt : applied to the password when generating the auth response

The client knows a password and must it to authenticate itself to the server.  
However, it must keep this password secret, and it is the purpose of the authentication mecanism used by obs-websocket.

After a call to `GetAuthRequired`, the client knows a password (kept secret), a challenge and a salt (sent by the server).
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
