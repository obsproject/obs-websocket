<!-- This file was generated based on handlebars templates. Do not edit directly! -->

# obs-websocket 5.0.0 protocol reference

**This is the reference for the unreleased obs-websocket 5.0.0. See the list below for older versions.**
- [4.3.2 protocol reference](https://github.com/Palakis/obs-websocket/blob/4.3.2/docs/generated/protocol.md)
- [4.2.0 protocol reference](https://github.com/Palakis/obs-websocket/blob/4.2.0/docs/generated/protocol.md)
- [4.1.0 protocol reference](https://github.com/Palakis/obs-websocket/blob/4.1.0/PROTOCOL.md)
- [4.0.0 protocol reference](https://github.com/Palakis/obs-websocket/blob/4.0.0/PROTOCOL.md)

# General Introduction
Messages are exchanged between the client and the server as JSON objects.
This protocol is based on the original OBS Remote protocol created by Bill Hamilton, with new commands specific to OBS Studio.


# Authentication
OBSWebSocket uses SHA256 to transmit credentials.

A request for [`GetAuthRequired`](#getauthrequired) returns two elements:
- A `challenge`: a random string that will be used to generate the auth response.
- A `salt`: applied to the password when generating the auth response.

To generate the answer to the auth challenge, follow this procedure:
- Concatenate the user declared password with the `salt` sent by the server (in this order: `password + server salt`).
- Generate a binary SHA256 hash of the result and encode the resulting SHA256 binary hash to base64, known as a `base64 secret`.
- Concatenate the base64 secret with the `challenge` sent by the server (in this order: `base64 secret + server challenge`).
- Generate a binary SHA256 hash of the result and encode it to base64.
- Voilà, this last base64 string is the `auth response`. You may now use it to authenticate to the server with the [`Authenticate`](#authenticate) request.

Pseudo Code Example:
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




# Table of Contents

<!-- toc -->

- [Events](#events)
  * [Scenes](#scenes)
    + [SwitchScenes](#switchscenes)
    + [ScenesChanged](#sceneschanged)
    + [SceneCollectionChanged](#scenecollectionchanged)
    + [SceneCollectionListChanged](#scenecollectionlistchanged)
  * [Transitions](#transitions)
    + [SwitchTransition](#switchtransition)
    + [TransitionListChanged](#transitionlistchanged)
    + [TransitionDurationChanged](#transitiondurationchanged)
    + [TransitionBegin](#transitionbegin)
  * [Profiles](#profiles)
    + [ProfileChanged](#profilechanged)
    + [ProfileListChanged](#profilelistchanged)
  * [Streaming](#streaming)
    + [StreamStarting](#streamstarting)
    + [StreamStarted](#streamstarted)
    + [StreamStopping](#streamstopping)
    + [StreamStopped](#streamstopped)
    + [StreamStatus](#streamstatus)
  * [Recording](#recording)
    + [RecordingStarting](#recordingstarting)
    + [RecordingStarted](#recordingstarted)
    + [RecordingStopping](#recordingstopping)
    + [RecordingStopped](#recordingstopped)
  * [Replay Buffer](#replay-buffer)
    + [ReplayStarting](#replaystarting)
    + [ReplayStarted](#replaystarted)
    + [ReplayStopping](#replaystopping)
    + [ReplayStopped](#replaystopped)
  * [Other](#other)
    + [Exiting](#exiting)
  * [General](#general)
    + [Heartbeat](#heartbeat)
  * [Sources](#sources)
    + [SourceOrderChanged](#sourceorderchanged)
    + [SceneItemAdded](#sceneitemadded)
    + [SceneItemRemoved](#sceneitemremoved)
    + [SceneItemVisibilityChanged](#sceneitemvisibilitychanged)
  * [Studio Mode](#studio-mode)
    + [PreviewSceneChanged](#previewscenechanged)
    + [StudioModeSwitched](#studiomodeswitched)
- [Requests](#requests)
  * [General](#general-1)
    + [GetVersion](#getversion)
    + [GetAuthRequired](#getauthrequired)
    + [Authenticate](#authenticate)
    + [SetHeartbeat](#setheartbeat)
    + [SetFilenameFormatting](#setfilenameformatting)
    + [GetFilenameFormatting](#getfilenameformatting)
  * [Profiles](#profiles-1)
    + [SetCurrentProfile](#setcurrentprofile)
    + [GetCurrentProfile](#getcurrentprofile)
    + [ListProfiles](#listprofiles)
  * [Recording](#recording-1)
    + [StartStopRecording](#startstoprecording)
    + [StartRecording](#startrecording)
    + [StopRecording](#stoprecording)
    + [SetRecordingFolder](#setrecordingfolder)
    + [GetRecordingFolder](#getrecordingfolder)
  * [Replay Buffer](#replay-buffer-1)
    + [StartStopReplayBuffer](#startstopreplaybuffer)
    + [StartReplayBuffer](#startreplaybuffer)
    + [StopReplayBuffer](#stopreplaybuffer)
    + [SaveReplayBuffer](#savereplaybuffer)
  * [Scene Collections](#scene-collections)
    + [SetCurrentSceneCollection](#setcurrentscenecollection)
    + [GetCurrentSceneCollection](#getcurrentscenecollection)
    + [ListSceneCollections](#listscenecollections)
  * [Scene Items](#scene-items)
    + [GetSceneItemProperties](#getsceneitemproperties)
    + [SetSceneItemProperties](#setsceneitemproperties)
    + [ResetSceneItem](#resetsceneitem)
    + [SetSceneItemRender](#setsceneitemrender)
    + [SetSceneItemPosition](#setsceneitemposition)
    + [SetSceneItemTransform](#setsceneitemtransform)
    + [SetSceneItemCrop](#setsceneitemcrop)
  * [Scenes](#scenes-1)
    + [SetCurrentScene](#setcurrentscene)
    + [GetCurrentScene](#getcurrentscene)
    + [GetSceneList](#getscenelist)
    + [SetSceneItemOrder](#setsceneitemorder)
  * [Sources](#sources-1)
    + [GetSourcesList](#getsourceslist)
    + [GetSourcesTypesList](#getsourcestypeslist)
    + [GetVolume](#getvolume)
    + [SetVolume](#setvolume)
    + [GetMute](#getmute)
    + [SetMute](#setmute)
    + [ToggleMute](#togglemute)
    + [SetSyncOffset](#setsyncoffset)
    + [GetSyncOffset](#getsyncoffset)
    + [GetSourceSettings](#getsourcesettings)
    + [SetSourceSettings](#setsourcesettings)
    + [GetTextGDIPlusProperties](#gettextgdiplusproperties)
    + [SetTextGDIPlusProperties](#settextgdiplusproperties)
    + [GetTextFreetype2Properties](#gettextfreetype2properties)
    + [SetTextFreetype2Properties](#settextfreetype2properties)
    + [GetBrowserSourceProperties](#getbrowsersourceproperties)
    + [SetBrowserSourceProperties](#setbrowsersourceproperties)
    + [DeleteSceneItem](#deletesceneitem)
    + [DuplicateSceneItem](#duplicatesceneitem)
    + [GetSpecialSources](#getspecialsources)
  * [Streaming](#streaming-1)
    + [GetStreamingStatus](#getstreamingstatus)
    + [StartStopStreaming](#startstopstreaming)
    + [StartStreaming](#startstreaming)
    + [StopStreaming](#stopstreaming)
    + [SetStreamSettings](#setstreamsettings)
    + [GetStreamSettings](#getstreamsettings)
    + [SaveStreamSettings](#savestreamsettings)
  * [Studio Mode](#studio-mode-1)
    + [GetStudioModeStatus](#getstudiomodestatus)
    + [GetPreviewScene](#getpreviewscene)
    + [SetPreviewScene](#setpreviewscene)
    + [TransitionToProgram](#transitiontoprogram)
    + [EnableStudioMode](#enablestudiomode)
    + [DisableStudioMode](#disablestudiomode)
    + [ToggleStudioMode](#togglestudiomode)
  * [Transitions](#transitions-1)
    + [GetTransitionList](#gettransitionlist)
    + [GetCurrentTransition](#getcurrenttransition)
    + [SetCurrentTransition](#setcurrenttransition)
    + [SetTransitionDuration](#settransitionduration)
    + [GetTransitionDuration](#gettransitionduration)

<!-- tocstop -->

# Events
Events are broadcast by the server to each connected client when a recognized action occurs within OBS.

An event message will contain at least the following base fields:
- `update-type` _String_: the type of event.
- `stream-timecode` _String (optional)_: time elapsed between now and stream start (only present if OBS Studio is streaming).
- `rec-timecode` _String (optional)_: time elapsed between now and recording start (only present if OBS Studio is recording).

Timecodes are sent using the format: `HH:MM:SS.mmm`

Additional fields may be present in the event message depending on the event type.


## Scenes

### SwitchScenes


- Added in v0.3

Indicates a scene change.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | The new scene. |
| `sources` | _Array_ | List of sources in the new scene. |


---

### ScenesChanged


- Added in v0.3

The scene list has been modified.
Scenes have been added, removed, or renamed.

**Response Items:**

_No additional response items._

---

### SceneCollectionChanged


- Added in v4.0.0

Triggered when switching to another scene collection or when renaming the current scene collection.

**Response Items:**

_No additional response items._

---

### SceneCollectionListChanged


- Added in v4.0.0

Triggered when a scene collection is created, added, renamed, or removed.

**Response Items:**

_No additional response items._

---

## Transitions

### SwitchTransition


- Added in v4.0.0

The active transition has been changed.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `transition-name` | _String_ | The name of the new active transition. |


---

### TransitionListChanged


- Added in v4.0.0

The list of available transitions has been modified.
Transitions have been added, removed, or renamed.

**Response Items:**

_No additional response items._

---

### TransitionDurationChanged


- Added in v4.0.0

The active transition duration has been changed.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `new-duration` | _int_ | New transition duration. |


---

### TransitionBegin


- Added in v4.0.0

A transition (other than "cut") has begun.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | Transition name. |
| `duration` | _int_ | Transition duration (in milliseconds). |


---

## Profiles

### ProfileChanged


- Added in v4.0.0

Triggered when switching to another profile or when renaming the current profile.

**Response Items:**

_No additional response items._

---

### ProfileListChanged


- Added in v4.0.0

Triggered when a profile is created, added, renamed, or removed.

**Response Items:**

_No additional response items._

---

## Streaming

### StreamStarting


- Added in v0.3

A request to start streaming has been issued.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `preview-only` | _boolean_ | Always false (retrocompatibility). |


---

### StreamStarted


- Added in v0.3

Streaming started successfully.

**Response Items:**

_No additional response items._

---

### StreamStopping


- Added in v0.3

A request to stop streaming has been issued.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `preview-only` | _boolean_ | Always false (retrocompatibility). |


---

### StreamStopped


- Added in v0.3

Streaming stopped successfully.

**Response Items:**

_No additional response items._

---

### StreamStatus


- Added in v0.3

Emit every 2 seconds.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `streaming` | _boolean_ | Current streaming state. |
| `recording` | _boolean_ | Current recording state. |
| `preview-only` | _boolean_ | Always false (retrocompatibility). |
| `bytes-per-sec` | _int_ | Amount of data per second (in bytes) transmitted by the stream encoder. |
| `kbits-per-sec` | _int_ | Amount of data per second (in kilobits) transmitted by the stream encoder. |
| `strain` | _double_ | Percentage of dropped frames. |
| `total-stream-time` | _int_ | Total time (in seconds) since the stream started. |
| `num-total-frames` | _int_ | Total number of frames transmitted since the stream started. |
| `num-dropped-frames` | _int_ | Number of frames dropped by the encoder since the stream started. |
| `fps` | _double_ | Current framerate. |


---

## Recording

### RecordingStarting


- Added in v0.3

A request to start recording has been issued.

**Response Items:**

_No additional response items._

---

### RecordingStarted


- Added in v0.3

Recording started successfully.

**Response Items:**

_No additional response items._

---

### RecordingStopping


- Added in v0.3

A request to stop recording has been issued.

**Response Items:**

_No additional response items._

---

### RecordingStopped


- Added in v0.3

Recording stopped successfully.

**Response Items:**

_No additional response items._

---

## Replay Buffer

### ReplayStarting


- Added in v4.2.0

A request to start the replay buffer has been issued.

**Response Items:**

_No additional response items._

---

### ReplayStarted


- Added in v4.2.0

Replay Buffer started successfully

**Response Items:**

_No additional response items._

---

### ReplayStopping


- Added in v4.2.0

A request to start the replay buffer has been issued.

**Response Items:**

_No additional response items._

---

### ReplayStopped


- Added in v4.2.0

Replay Buffer stopped successfully

**Response Items:**

_No additional response items._

---

## Other

### Exiting


- Added in v0.3

OBS is exiting.

**Response Items:**

_No additional response items._

---

## General

### Heartbeat


- Added in v

Emitted every 2 seconds after enabling it by calling SetHeartbeat.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `pulse` | _boolean_ | Toggles between every JSON meassage as an "I am alive" indicator. |
| `current-profile` | _string (optional)_ | Current active profile. |
| `current-scene` | _string (optional)_ | Current active scene. |
| `streaming` | _boolean (optional)_ | Current streaming state. |
| `total-stream-time` | _int (optional)_ | Total time (in seconds) since the stream started. |
| `total-stream-bytes` | _int (optional)_ | Total bytes sent since the stream started. |
| `total-stream-frames` | _int (optional)_ | Total frames streamed since the stream started. |
| `recording` | _boolean (optional)_ | Current recording state. |
| `total-record-time` | _int (optional)_ | Total time (in seconds) since recording started. |
| `total-record-bytes` | _int (optional)_ | Total bytes recorded since the recording started. |
| `total-record-frames` | _int (optional)_ | Total frames recorded since the recording started. |


---

## Sources

### SourceOrderChanged


- Added in v4.0.0

Scene items have been reordered.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | Name of the scene where items have been reordered. |
| `sources` | _Array_ | Array of sources. |


---

### SceneItemAdded


- Added in v4.0.0

An item has been added to the current scene.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene. |
| `item-name` | _String_ | Name of the item added to the scene. |


---

### SceneItemRemoved


- Added in v4.0.0

An item has been removed from the current scene.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene. |
| `item-name` | _String_ | Name of the item removed from the scene. |


---

### SceneItemVisibilityChanged


- Added in v4.0.0

An item's visibility has been toggled.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene. |
| `item-name` | _String_ | Name of the item in the scene. |
| `item-visible` | _boolean_ | New visibility state of the item. |


---

## Studio Mode

### PreviewSceneChanged


- Added in v4.1.0

The selected preview scene has changed (only available in Studio Mode).

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene being previewed. |
| `sources` | _Source\|Array_ | List of sources composing the scene. Same specification as [`GetCurrentScene`](#getcurrentscene). |


---

### StudioModeSwitched


- Added in v4.1.0

Studio Mode has been enabled or disabled.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `new-state` | _boolean_ | The new enabled state of Studio Mode. |


---




# Requests
Requests are sent by the client and require at least the following two fields:
- `request-type` _String_: String name of the request type.
- `message-id` _String_: Client defined identifier for the message, will be echoed in the response.

Once a request is sent, the server will return a JSON response with at least the following fields:
- `message-id` _String_: The client defined identifier specified in the request.
- `status` _String_: Response status, will be one of the following: `ok`, `error`
- `error` _String_: An error message accompanying an `error` status.

Additional information may be required/returned depending on the request type. See below for more information.


## General

### GetVersion


- Added in v0.3

Returns the latest version of the plugin and the API.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `version` | _double_ | OBSRemote compatible API version. Fixed to 1.1 for retrocompatibility. |
| `obs-websocket-version` | _String_ | obs-websocket plugin version. |
| `obs-studio-version` | _String_ | OBS Studio program version. |
| `available-requests` | _String_ | List of available request types, formatted as a comma-separated list string (e.g. : "Method1,Method2,Method3"). |


---

### GetAuthRequired


- Added in v0.3

Tells the client if authentication is required. If so, returns authentication parameters `challenge`
and `salt` (see "Authentication" for more information).

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `authRequired` | _boolean_ | Indicates whether authentication is required. |
| `challenge` | _String (optional)_ |  |
| `salt` | _String (optional)_ |  |


---

### Authenticate


- Added in v0.3

Attempt to authenticate the client to the server.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `auth` | _String_ | Response to the auth challenge (see "Authentication" for more information). |


**Response Items:**

_No additional response items._

---

### SetHeartbeat


- Added in v4.3.0

Enable/disable sending of the Heartbeat event

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `enable` | _boolean_ | Starts/Stops emitting heartbeat messages |


**Response Items:**

_No additional response items._

---

### SetFilenameFormatting


- Added in v4.3.0

Set the filename formatting string

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `filename-formatting` | _String_ | Filename formatting string to set. |


**Response Items:**

_No additional response items._

---

### GetFilenameFormatting


- Added in v4.3.0

Get the filename formatting string

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `filename-formatting` | _String_ | Current filename formatting string. |


---

## Profiles

### SetCurrentProfile


- Added in v4.0.0

Set the currently active profile.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `profile-name` | _String_ | Name of the desired profile. |


**Response Items:**

_No additional response items._

---

### GetCurrentProfile


- Added in v4.0.0

Get the name of the current profile.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `profile-name` | _String_ | Name of the currently active profile. |


---

### ListProfiles


- Added in v4.0.0

Get a list of available profiles.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `profiles` | _Object\|Array_ | List of available profiles. |


---

## Recording

### StartStopRecording


- Added in v0.3

Toggle recording on or off.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### StartRecording


- Added in v4.1.0

Start recording.
Will return an `error` if recording is already active.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### StopRecording


- Added in v4.1.0

Stop recording.
Will return an `error` if recording is not active.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### SetRecordingFolder


- Added in v4.1.0

Change the current recording folder.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `rec-folder` | _String_ | Path of the recording folder. |


**Response Items:**

_No additional response items._

---

### GetRecordingFolder


- Added in v4.1.0

Get the path of  the current recording folder.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `rec-folder` | _String_ | Path of the recording folder. |


---

## Replay Buffer

### StartStopReplayBuffer


- Added in v4.2.0

Toggle the Replay Buffer on/off.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### StartReplayBuffer


- Added in v4.2.0

Start recording into the Replay Buffer.
Will return an `error` if the Replay Buffer is already active or if the
"Save Replay Buffer" hotkey is not set in OBS' settings.
Setting this hotkey is mandatory, even when triggering saves only
through obs-websocket.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### StopReplayBuffer


- Added in v4.2.0

Stop recording into the Replay Buffer.
Will return an `error` if the Replay Buffer is not active.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### SaveReplayBuffer


- Added in v4.2.0

Flush and save the contents of the Replay Buffer to disk. This is
basically the same as triggering the "Save Replay Buffer" hotkey.
Will return an `error` if the Replay Buffer is not active.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

## Scene Collections

### SetCurrentSceneCollection


- Added in v4.0.0

Change the active scene collection.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sc-name` | _String_ | Name of the desired scene collection. |


**Response Items:**

_No additional response items._

---

### GetCurrentSceneCollection


- Added in v4.0.0

Get the name of the current scene collection.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sc-name` | _String_ | Name of the currently active scene collection. |


---

### ListSceneCollections


- Added in v4.0.0

List available scene collections

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-collections` | _Object\|Array_ | Scene collections list |
| `scene-collections.*.` | _String_ |  |


---

## Scene Items

### GetSceneItemProperties


- Added in v4.3.0

Gets the scene specific properties of the specified source item.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene` | _String (optional)_ | the name of the scene that the source item belongs to. Defaults to the current scene. |
| `item.id` | _String_ | The name of the source. |
| `item.name` | _String_ | The name of the source. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene` | _String_ | The name of the scene. |
| `item.name` | _String_ | The name of the source. |
| `item.id` | _String_ | The id of the scene item. |
| `item.position.x` | _int_ | The x position of the source from the left. |
| `item.position.y` | _int_ | The y position of the source from the top. |
| `item.position.alignment` | _int_ | The point on the source that the item is manipulated from. |
| `item.rotation` | _double_ | The clockwise rotation of the item in degrees around the point of alignment. |
| `item.scale.x` | _double_ | The x-scale factor of the source. |
| `item.scale.y` | _double_ | The y-scale factor of the source. |
| `item.crop.top` | _int_ | The number of pixels cropped off the top of the source before scaling. |
| `item.crop.right` | _int_ | The number of pixels cropped off the right of the source before scaling. |
| `item.crop.bottom` | _int_ | The number of pixels cropped off the bottom of the source before scaling. |
| `item.crop.left` | _int_ | The number of pixels cropped off the left of the source before scaling. |
| `item.visible` | _bool_ | If the source is visible. |
| `item.locked` | _bool_ | If the source is locked. |
| `item.bounds.type` | _String_ | Type of bounding box. |
| `item.bounds.alignment` | _int_ | Alignment of the bounding box. |
| `item.bounds.x` | _double_ | Width of the bounding box. |
| `item.bounds.y` | _double_ | Height of the bounding box. |


---

### SetSceneItemProperties


- Added in v4.3.0

Sets the scene specific properties of a source. Unspecified properties will remain unchanged.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene` | _String (optional)_ | the name of the scene that the source item belongs to. Defaults to the current scene. |
| `item.name` | _String_ | The name of the item. |
| `item.id` | _int_ | The id of the item. |
| `item.position.x` | _int_ | The new x position of the item. |
| `item.position.y` | _int_ | The new y position of the item. |
| `item.position.alignment` | _int_ | The new alignment of the item. |
| `item.rotation` | _double_ | The new clockwise rotation of the item in degrees. |
| `item.scale.x` | _double_ | The new x scale of the item. |
| `item.scale.y` | _double_ | The new y scale of the item. |
| `item.crop.top` | _int_ | The new amount of pixels cropped off the top of the source before scaling. |
| `item.crop.bottom` | _int_ | The new amount of pixels cropped off the bottom of the source before scaling. |
| `item.crop.left` | _int_ | The new amount of pixels cropped off the left of the source before scaling. |
| `item.crop.right` | _int_ | The new amount of pixels cropped off the right of the source before scaling. |
| `item.visible` | _bool_ | The new visibility of the item. 'true' shows source, 'false' hides source. |
| `item.locked` | _bool_ | The new locked of the item. 'true' is locked, 'false' is unlocked. |
| `item.bounds.type` | _String_ | The new bounds type of the item. |
| `item.bounds.alignment` | _int_ | The new alignment of the bounding box. (0-2, 4-6, 8-10) |
| `item.bounds.x` | _double_ | The new width of the bounding box. |
| `item.bounds.y` | _double_ | The new height of the bounding box. |


**Response Items:**

_No additional response items._

---

### ResetSceneItem


- Added in v4.2.0

Reset a scene item.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | Name of the scene the source belogns to. Defaults to the current scene. |
| `item` | _String_ | Name of the source item. |


**Response Items:**

_No additional response items._

---

### SetSceneItemRender

- **⚠️ Deprecated. Since 4.3.0. Prefer the use of SetSceneItemProperties. ⚠️**

- Added in v0.3

Show or hide a specified source item in a specified scene.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Scene item name in the specified scene. |
| `render` | _boolean_ | true = shown ; false = hidden |
| `scene-name` | _String (optional)_ | Name of the scene where the source resides. Defaults to the currently active scene. |


**Response Items:**

_No additional response items._

---

### SetSceneItemPosition

- **⚠️ Deprecated. Since 4.3.0. Prefer the use of SetSceneItemProperties. ⚠️**

- Added in v4.0.0

Sets the coordinates of a specified source item.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | The name of the scene that the source item belongs to. Defaults to the current scene. |
| `item` | _String_ | The name of the source item. |
| `x` | _double_ | X coordinate. |
| `y` | _double_ | Y coordinate. |


**Response Items:**

_No additional response items._

---

### SetSceneItemTransform

- **⚠️ Deprecated. Since 4.3.0. Prefer the use of SetSceneItemProperties. ⚠️**

- Added in v4.0.0

Set the transform of the specified source item.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | The name of the scene that the source item belongs to. Defaults to the current scene. |
| `item` | _String_ | The name of the source item. |
| `x-scale` | _double_ | Width scale factor. |
| `y-scale` | _double_ | Height scale factor. |
| `rotation` | _double_ | Source item rotation (in degrees). |


**Response Items:**

_No additional response items._

---

### SetSceneItemCrop

- **⚠️ Deprecated. Since 4.3.0. Prefer the use of SetSceneItemProperties. ⚠️**

- Added in v4.1.0

Sets the crop coordinates of the specified source item.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | the name of the scene that the source item belongs to. Defaults to the current scene. |
| `item` | _String_ | The name of the source. |
| `top` | _int_ | Pixel position of the top of the source item. |
| `bottom` | _int_ | Pixel position of the bottom of the source item. |
| `left` | _int_ | Pixel position of the left of the source item. |
| `right` | _int_ | Pixel position of the right of the source item. |


**Response Items:**

_No additional response items._

---

## Scenes

### SetCurrentScene


- Added in v0.3

Switch to the specified scene.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene to switch to. |


**Response Items:**

_No additional response items._

---

### GetCurrentScene


- Added in v0.3

Get the current scene's name and source items.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | Name of the currently active scene. |
| `sources` | _Source\|Array_ | Ordered list of the current scene's source items. |


---

### GetSceneList


- Added in v0.3

Get a list of scenes in the currently active profile.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `current-scene` | _String_ | Name of the currently active scene. |
| `scenes` | _Scene\|Array_ | Ordered list of the current profile's scenes (See `[GetCurrentScene](#getcurrentscene)` for more information). |


---

### SetSceneItemOrder


- Unreleased

Changes the order of scene items in the requested scene.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene` | _String (optional)_ | Name of the scene to reorder (defaults to current). |
| `items` | _Scene\|Array_ | Ordered list of objects with name and/or id specified. Id prefered due to uniqueness per scene |
| `items[].id` | _int (optional)_ | Id of a specific scene item. Unique on a scene by scene basis. |
| `items[].name` | _String (optional)_ | Name of a scene item. Sufficiently unique if no scene items share sources within the scene. |


**Response Items:**

_No additional response items._

---

## Sources

### GetSourcesList


- Added in v4.3.0

List all sources available in the running OBS instance

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sources` | _Array of Objects_ | Array of sources as objects |
| `sources.*.name` | _String_ | Unique source name |
| `sources.*.typeId` | _String_ | Non-unique source internal type (a.k.a type id) |
| `sources.*.type` | _String_ | Source type. Value is one of the following: "input", "filter", "transition", "scene" or "unknown" |


---

### GetSourcesTypesList


- Added in v4.3.0

Get a list of all available sources types

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `ids` | _Array of Objects_ | Array of sources as objects |
| `ids.*.typeId` | _String_ | Non-unique internal source type ID |
| `ids.*.displayName` | _String_ | Display name of the source type |
| `ids.*.type` | _String_ | Type. Value is one of the following: "input", "filter", "transition" or "other" |
| `ids.*.defaultSettings` | _Object_ | Default settings of this source type |
| `ids.*.caps` | _Object_ | Source type capabilities |
| `ids.*.caps.isAsync` | _Boolean_ | True if source of this type provide frames asynchronously |
| `ids.*.caps.hasVideo` | _Boolean_ | True if sources of this type provide video |
| `ids.*.caps.hasAudio` | _Boolean_ | True if sources of this type provide audio |
| `ids.*.caps.canInteract` | _Boolean_ | True if interaction with this sources of this type is possible |
| `ids.*.caps.isComposite` | _Boolean_ | True if sources of this type composite one or more sub-sources |
| `ids.*.caps.doNotDuplicate` | _Boolean_ | True if sources of this type should not be fully duplicated |
| `ids.*.caps.doNotSelfMonitor` | _Boolean_ | True if sources of this type may cause a feedback loop if it's audio is monitored and shouldn't be |


---

### GetVolume


- Added in v4.0.0

Get the volume of the specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Name of the source. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | Name of the source. |
| `volume` | _double_ | Volume of the source. Between `0.0` and `1.0`. |
| `mute` | _boolean_ | Indicates whether the source is muted. |


---

### SetVolume


- Added in v4.0.0

Set the volume of the specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Name of the source. |
| `volume` | _double_ | Desired volume. Must be between `0.0` and `1.0`. |


**Response Items:**

_No additional response items._

---

### GetMute


- Added in v4.0.0

Get the mute status of a specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | The name of the source. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | The name of the source. |
| `muted` | _boolean_ | Mute status of the source. |


---

### SetMute


- Added in v4.0.0

Sets the mute status of a specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | The name of the source. |
| `mute` | _boolean_ | Desired mute status. |


**Response Items:**

_No additional response items._

---

### ToggleMute


- Added in v4.0.0

Inverts the mute status of a specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | The name of the source. |


**Response Items:**

_No additional response items._

---

### SetSyncOffset


- Added in v4.2.0

Set the audio sync offset of a specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | The name of the source. |
| `offset` | _int_ | The desired audio sync offset (in nanoseconds). |


**Response Items:**

_No additional response items._

---

### GetSyncOffset


- Added in v4.2.0

Get the audio sync offset of a specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | The name of the source. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | The name of the source. |
| `offset` | _int_ | The audio sync offset (in nanoseconds). |


---

### GetSourceSettings


- Added in v4.3.0

Get settings of the specified source

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Name of the source item. |
| `sourceType` | _String (optional)_ | Type of the specified source. Useful for type-checking if you expect a specific settings schema. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceType` | _String_ | Type of the specified source |
| `sourceSettings` | _Object_ | Source settings. Varying between source types. |


---

### SetSourceSettings


- Added in v4.3.0

Set settings of the specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Name of the source item. |
| `sourceType` | _String (optional)_ | Type of the specified source. Useful for type-checking to avoid settings a set of settings incompatible with the actual source's type. |
| `sourceSettings` | _Object_ | Source settings. Varying between source types. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceType` | _String_ | Type of the specified source |
| `sourceSettings` | _Object_ | Source settings. Varying between source types. |


---

### GetTextGDIPlusProperties


- Added in v4.1.0

Get the current properties of a Text GDI Plus source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | Name of the scene to retrieve. Defaults to the current scene. |
| `source` | _String_ | Name of the source. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `align` | _String_ | Text Alignment ("left", "center", "right"). |
| `bk-color` | _int_ | Background color. |
| `bk-opacity` | _int_ | Background opacity (0-100). |
| `chatlog` | _boolean_ | Chat log. |
| `chatlog_lines` | _int_ | Chat log lines. |
| `color` | _int_ | Text color. |
| `extents` | _boolean_ | Extents wrap. |
| `extents_cx` | _int_ | Extents cx. |
| `extents_cy` | _int_ | Extents cy. |
| `file` | _String_ | File path name. |
| `read_from_file` | _boolean_ | Read text from the specified file. |
| `font` | _Object_ | Holds data for the font. Ex: `"font": { "face": "Arial", "flags": 0, "size": 150, "style": "" }` |
| `font.face` | _String_ | Font face. |
| `font.flags` | _int_ | Font text styling flag. `Bold=1, Italic=2, Bold Italic=3, Underline=5, Strikeout=8` |
| `font.size` | _int_ | Font text size. |
| `font.style` | _String_ | Font Style (unknown function). |
| `gradient` | _boolean_ | Gradient enabled. |
| `gradient_color` | _int_ | Gradient color. |
| `gradient_dir` | _float_ | Gradient direction. |
| `gradient_opacity` | _int_ | Gradient opacity (0-100). |
| `outline` | _boolean_ | Outline. |
| `outline_color` | _int_ | Outline color. |
| `outline_size` | _int_ | Outline size. |
| `outline_opacity` | _int_ | Outline opacity (0-100). |
| `text` | _String_ | Text content to be displayed. |
| `valign` | _String_ | Text vertical alignment ("top", "center", "bottom"). |
| `vertical` | _boolean_ | Vertical text enabled. |
| `render` | _boolean_ | Visibility of the scene item. |


---

### SetTextGDIPlusProperties


- Added in v4.1.0

Set the current properties of a Text GDI Plus source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | Name of the scene to retrieve. Defaults to the current scene. |
| `source` | _String_ | Name of the source. |
| `align` | _String (optional)_ | Text Alignment ("left", "center", "right"). |
| `bk-color` | _int (optional)_ | Background color. |
| `bk-opacity` | _int (optional)_ | Background opacity (0-100). |
| `chatlog` | _boolean (optional)_ | Chat log. |
| `chatlog_lines` | _int (optional)_ | Chat log lines. |
| `color` | _int (optional)_ | Text color. |
| `extents` | _boolean (optional)_ | Extents wrap. |
| `extents_cx` | _int (optional)_ | Extents cx. |
| `extents_cy` | _int (optional)_ | Extents cy. |
| `file` | _String (optional)_ | File path name. |
| `read_from_file` | _boolean (optional)_ | Read text from the specified file. |
| `font` | _Object (optional)_ | Holds data for the font. Ex: `"font": { "face": "Arial", "flags": 0, "size": 150, "style": "" }` |
| `font.face` | _String (optional)_ | Font face. |
| `font.flags` | _int (optional)_ | Font text styling flag. `Bold=1, Italic=2, Bold Italic=3, Underline=5, Strikeout=8` |
| `font.size` | _int (optional)_ | Font text size. |
| `font.style` | _String (optional)_ | Font Style (unknown function). |
| `gradient` | _boolean (optional)_ | Gradient enabled. |
| `gradient_color` | _int (optional)_ | Gradient color. |
| `gradient_dir` | _float (optional)_ | Gradient direction. |
| `gradient_opacity` | _int (optional)_ | Gradient opacity (0-100). |
| `outline` | _boolean (optional)_ | Outline. |
| `outline_color` | _int (optional)_ | Outline color. |
| `outline_size` | _int (optional)_ | Outline size. |
| `outline_opacity` | _int (optional)_ | Outline opacity (0-100). |
| `text` | _String (optional)_ | Text content to be displayed. |
| `valign` | _String (optional)_ | Text vertical alignment ("top", "center", "bottom"). |
| `vertical` | _boolean (optional)_ | Vertical text enabled. |
| `render` | _boolean (optional)_ | Visibility of the scene item. |


**Response Items:**

_No additional response items._

---

### GetTextFreetype2Properties


- Added in v4.x.x

Get the current properties of a Text Freetype 2 source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | Name of the scene to retrieve. Defaults to the current scene. |
| `source` | _String_ | Name of the source. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `color1` | _int_ | Gradient top color. |
| `color2` | _int_ | Gradient bottom color. |
| `custom_width` | _int_ | Custom width (0 to disable). |
| `drop_shadow` | _boolean_ | Drop shadow. |
| `font` | _Object_ | Holds data for the font. Ex: `"font": { "face": "Arial", "flags": 0, "size": 150, "style": "" }` |
| `font.face` | _String_ | Font face. |
| `font.flags` | _int_ | Font text styling flag. `Bold=1, Italic=2, Bold Italic=3, Underline=5, Strikeout=8` |
| `font.size` | _int_ | Font text size. |
| `font.style` | _String_ | Font Style (unknown function). |
| `from_file` | _boolean_ | Read text from the specified file. |
| `log_mode` | _boolean_ | Chat log. |
| `outline` | _boolean_ | Outline. |
| `text` | _String_ | Text content to be displayed. |
| `text_file` | _String_ | File path. |
| `word_wrap` | _boolean_ | Word wrap. |
| `render` | _boolean_ | Visibility of the scene item. |


---

### SetTextFreetype2Properties


- Added in v4.x.x

Set the current properties of a Text Freetype 2 source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | Name of the scene to retrieve. Defaults to the current scene. |
| `source` | _String_ | Name of the source. |
| `color1` | _int (optional)_ | Gradient top color. |
| `color2` | _int (optional)_ | Gradient bottom color. |
| `custom_width` | _int (optional)_ | Custom width (0 to disable). |
| `drop_shadow` | _boolean (optional)_ | Drop shadow. |
| `font` | _Object (optional)_ | Holds data for the font. Ex: `"font": { "face": "Arial", "flags": 0, "size": 150, "style": "" }` |
| `font.face` | _String (optional)_ | Font face. |
| `font.flags` | _int (optional)_ | Font text styling flag. `Bold=1, Italic=2, Bold Italic=3, Underline=5, Strikeout=8` |
| `font.size` | _int (optional)_ | Font text size. |
| `font.style` | _String (optional)_ | Font Style (unknown function). |
| `from_file` | _boolean (optional)_ | Read text from the specified file. |
| `log_mode` | _boolean (optional)_ | Chat log. |
| `outline` | _boolean (optional)_ | Outline. |
| `text` | _String (optional)_ | Text content to be displayed. |
| `text_file` | _String (optional)_ | File path. |
| `word_wrap` | _boolean (optional)_ | Word wrap. |
| `render` | _boolean (optional)_ | Visibility of the scene item. |


**Response Items:**

_No additional response items._

---

### GetBrowserSourceProperties


- Added in v4.1.0

Get current properties for a Browser Source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | Name of the scene that the source belongs to. Defaults to the current scene. |
| `source` | _String_ | Name of the source. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `is_local_file` | _boolean_ | Indicates that a local file is in use. |
| `local_file` | _String_ | file path. |
| `url` | _String_ | Url. |
| `css` | _String_ | CSS to inject. |
| `width` | _int_ | Width. |
| `height` | _int_ | Height. |
| `fps` | _int_ | Framerate. |
| `shutdown` | _boolean_ | Indicates whether the source should be shutdown when not visible. |
| `render` | _boolean (optional)_ | Visibility of the scene item. |


---

### SetBrowserSourceProperties


- Added in v4.1.0

Set current properties for a Browser Source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | Name of the scene that the source belongs to. Defaults to the current scene. |
| `source` | _String_ | Name of the source. |
| `is_local_file` | _boolean (optional)_ | Indicates that a local file is in use. |
| `local_file` | _String (optional)_ | file path. |
| `url` | _String (optional)_ | Url. |
| `css` | _String (optional)_ | CSS to inject. |
| `width` | _int (optional)_ | Width. |
| `height` | _int (optional)_ | Height. |
| `fps` | _int (optional)_ | Framerate. |
| `shutdown` | _boolean (optional)_ | Indicates whether the source should be shutdown when not visible. |
| `render` | _boolean (optional)_ | Visibility of the scene item. |


**Response Items:**

_No additional response items._

---

### DeleteSceneItem


- Unreleased

Deletes a scene item.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene` | _String (optional)_ | Name of the scene the source belogns to. Defaults to the current scene. |
| `item` | _Object_ | item to delete (required) |
| `item.name` | _String_ | name of the scene item (prefer `id`, including both is acceptable). |
| `item.id` | _int_ | id of the scene item. |


**Response Items:**

_No additional response items._

---

### DuplicateSceneItem


- Unreleased

Duplicates a scene item.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `fromScene` | _String (optional)_ | Name of the scene to copy the item from. Defaults to the current scene. |
| `toScene` | _String (optional)_ | Name of the scene to create the item in. Defaults to the current scene. |
| `item` | _Object_ | item to delete (required) |
| `item.name` | _String_ | name of the scene item (prefer `id`, including both is acceptable). |
| `item.id` | _int_ | id of the scene item. |


**Response Items:**

_No additional response items._

---

### GetSpecialSources


- Added in v4.1.0

Get configured special sources like Desktop Audio and Mic/Aux sources.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `desktop-1` | _String (optional)_ | Name of the first Desktop Audio capture source. |
| `desktop-2` | _String (optional)_ | Name of the second Desktop Audio capture source. |
| `mic-1` | _String (optional)_ | Name of the first Mic/Aux input source. |
| `mic-2` | _String (optional)_ | Name of the second Mic/Aux input source. |
| `mic-3` | _String (optional)_ | NAme of the third Mic/Aux input source. |


---

## Streaming

### GetStreamingStatus


- Added in v0.3

Get current streaming and recording status.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `streaming` | _boolean_ | Current streaming status. |
| `recording` | _boolean_ | Current recording status. |
| `stream-timecode` | _String (optional)_ | Time elapsed since streaming started (only present if currently streaming). |
| `rec-timecode` | _String (optional)_ | Time elapsed since recording started (only present if currently recording). |
| `preview-only` | _boolean_ | Always false. Retrocompatibility with OBSRemote. |


---

### StartStopStreaming


- Added in v0.3

Toggle streaming on or off.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### StartStreaming


- Added in v4.1.0

Start streaming.
Will return an `error` if streaming is already active.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `stream` | _Object (optional)_ | Special stream configuration. Please note: these won't be saved to OBS' configuration. |
| `stream.type` | _String (optional)_ | If specified ensures the type of stream matches the given type (usually 'rtmp_custom' or 'rtmp_common'). If the currently configured stream type does not match the given stream type, all settings must be specified in the `settings` object or an error will occur when starting the stream. |
| `stream.metadata` | _Object (optional)_ | Adds the given object parameters as encoded query string parameters to the 'key' of the RTMP stream. Used to pass data to the RTMP service about the streaming. May be any String, Numeric, or Boolean field. |
| `stream.settings` | _Object (optional)_ | Settings for the stream. |
| `stream.settings.server` | _String (optional)_ | The publish URL. |
| `stream.settings.key` | _String (optional)_ | The publish key of the stream. |
| `stream.settings.use-auth` | _boolean (optional)_ | Indicates whether authentication should be used when connecting to the streaming server. |
| `stream.settings.username` | _String (optional)_ | If authentication is enabled, the username for the streaming server. Ignored if `use-auth` is not set to `true`. |
| `stream.settings.password` | _String (optional)_ | If authentication is enabled, the password for the streaming server. Ignored if `use-auth` is not set to `true`. |


**Response Items:**

_No additional response items._

---

### StopStreaming


- Added in v4.1.0

Stop streaming.
Will return an `error` if streaming is not active.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### SetStreamSettings


- Added in v4.1.0

Sets one or more attributes of the current streaming server settings. Any options not passed will remain unchanged. Returns the updated settings in response. If 'type' is different than the current streaming service type, all settings are required. Returns the full settings of the stream (the same as GetStreamSettings).

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `type` | _String_ | The type of streaming service configuration, usually `rtmp_custom` or `rtmp_common`. |
| `settings` | _Object_ | The actual settings of the stream. |
| `settings.server` | _String (optional)_ | The publish URL. |
| `settings.key` | _String (optional)_ | The publish key. |
| `settings.use-auth` | _boolean (optional)_ | Indicates whether authentication should be used when connecting to the streaming server. |
| `settings.username` | _String (optional)_ | The username for the streaming service. |
| `settings.password` | _String (optional)_ | The password for the streaming service. |
| `save` | _boolean_ | Persist the settings to disk. |


**Response Items:**

_No additional response items._

---

### GetStreamSettings


- Added in v4.1.0

Get the current streaming server settings.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `type` | _String_ | The type of streaming service configuration. Possible values: 'rtmp_custom' or 'rtmp_common'. |
| `settings` | _Object_ | Stream settings object. |
| `settings.server` | _String_ | The publish URL. |
| `settings.key` | _String_ | The publish key of the stream. |
| `settings.use-auth` | _boolean_ | Indicates whether audentication should be used when connecting to the streaming server. |
| `settings.username` | _String_ | The username to use when accessing the streaming server. Only present if `use-auth` is `true`. |
| `settings.password` | _String_ | The password to use when accessing the streaming server. Only present if `use-auth` is `true`. |


---

### SaveStreamSettings


- Added in v4.1.0

Save the current streaming server settings to disk.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

## Studio Mode

### GetStudioModeStatus


- Added in v4.1.0

Indicates if Studio Mode is currently enabled.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `studio-mode` | _boolean_ | Indicates if Studio Mode is enabled. |


---

### GetPreviewScene


- Added in v4.1.0

Get the name of the currently previewed scene and its list of sources.
Will return an `error` if Studio Mode is not enabled.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | The name of the active preview scene. |
| `sources` | _Source\|Array_ |  |


---

### SetPreviewScene


- Added in v4.1.0

Set the active preview scene.
Will return an `error` if Studio Mode is not enabled.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | The name of the scene to preview. |


**Response Items:**

_No additional response items._

---

### TransitionToProgram


- Added in v4.1.0

Transitions the currently previewed scene to the main output.
Will return an `error` if Studio Mode is not enabled.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `with-transition` | _Object (optional)_ | Change the active transition before switching scenes. Defaults to the active transition. |
| `with-transition.name` | _String_ | Name of the transition. |
| `with-transition.duration` | _int (optional)_ | Transition duration (in milliseconds). |


**Response Items:**

_No additional response items._

---

### EnableStudioMode


- Added in v4.1.0

Enables Studio Mode.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### DisableStudioMode


- Added in v4.1.0

Disables Studio Mode.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### ToggleStudioMode


- Added in v4.1.0

Toggles Studio Mode.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

## Transitions

### GetTransitionList


- Added in v4.1.0

List of all transitions available in the frontend's dropdown menu.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `current-transition` | _String_ | Name of the currently active transition. |
| `transitions` | _Object\|Array_ | List of transitions. |
| `transitions[].name` | _String_ | Name of the transition. |


---

### GetCurrentTransition


- Added in v0.3

Get the name of the currently selected transition in the frontend's dropdown menu.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | Name of the selected transition. |
| `duration` | _int (optional)_ | Transition duration (in milliseconds) if supported by the transition. |


---

### SetCurrentTransition


- Added in v0.3

Set the active transition.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `transition-name` | _String_ | The name of the transition. |


**Response Items:**

_No additional response items._

---

### SetTransitionDuration


- Added in v4.0.0

Set the duration of the currently selected transition if supported.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `duration` | _int_ | Desired duration of the transition (in milliseconds). |


**Response Items:**

_No additional response items._

---

### GetTransitionDuration


- Added in v4.1.0

Get the duration of the currently selected transition if supported.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `transition-duration` | _int_ | Duration of the current transition (in milliseconds). |


---

