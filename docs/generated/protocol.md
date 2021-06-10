<!-- This file was generated based on handlebars templates. Do not edit directly! -->

# obs-websocket 4.9.1 protocol reference

# General Introduction
Messages are exchanged between the client and the server as JSON objects.
This protocol is based on the original OBS Remote protocol created by Bill Hamilton, with new commands specific to OBS Studio. As of v5.0.0, backwards compatability with the protocol will not be kept.

# Authentication
**Starting with obs-websocket 4.9, authentication is enabled by default and users are encouraged to configure a password on first run.**

`obs-websocket` uses SHA256 to transmit credentials.

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

You can also refer to any of the [client libraries](https://github.com/Palakis/obs-websocket#for-developers) listed on the README for examples of how to authenticate.




# Table of Contents

<!-- toc -->

- [Typedefs](#typedefs)
  * [SceneItem](#sceneitem)
  * [SceneItemTransform](#sceneitemtransform)
  * [OBSStats](#obsstats)
  * [Output](#output)
  * [ScenesCollection](#scenescollection)
  * [Scene](#scene)
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
    + [TransitionEnd](#transitionend)
    + [TransitionVideoEnd](#transitionvideoend)
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
    + [RecordingPaused](#recordingpaused)
    + [RecordingResumed](#recordingresumed)
  * [Virtual Cam](#virtual-cam)
    + [VirtualCamStarted](#virtualcamstarted)
    + [VirtualCamStopped](#virtualcamstopped)
  * [Replay Buffer](#replay-buffer)
    + [ReplayStarting](#replaystarting)
    + [ReplayStarted](#replaystarted)
    + [ReplayStopping](#replaystopping)
    + [ReplayStopped](#replaystopped)
  * [Other](#other)
    + [Exiting](#exiting)
  * [General](#general)
    + [Heartbeat](#heartbeat)
    + [BroadcastCustomMessage](#broadcastcustommessage)
  * [Sources](#sources)
    + [SourceCreated](#sourcecreated)
    + [SourceDestroyed](#sourcedestroyed)
    + [SourceVolumeChanged](#sourcevolumechanged)
    + [SourceMuteStateChanged](#sourcemutestatechanged)
    + [SourceAudioDeactivated](#sourceaudiodeactivated)
    + [SourceAudioActivated](#sourceaudioactivated)
    + [SourceAudioSyncOffsetChanged](#sourceaudiosyncoffsetchanged)
    + [SourceAudioMixersChanged](#sourceaudiomixerschanged)
    + [SourceRenamed](#sourcerenamed)
    + [SourceFilterAdded](#sourcefilteradded)
    + [SourceFilterRemoved](#sourcefilterremoved)
    + [SourceFilterVisibilityChanged](#sourcefiltervisibilitychanged)
    + [SourceFiltersReordered](#sourcefiltersreordered)
  * [Media](#media)
    + [MediaPlaying](#mediaplaying)
    + [MediaPaused](#mediapaused)
    + [MediaRestarted](#mediarestarted)
    + [MediaStopped](#mediastopped)
    + [MediaNext](#medianext)
    + [MediaPrevious](#mediaprevious)
    + [MediaStarted](#mediastarted)
    + [MediaEnded](#mediaended)
  * [Scene Items](#scene-items)
    + [SourceOrderChanged](#sourceorderchanged)
    + [SceneItemAdded](#sceneitemadded)
    + [SceneItemRemoved](#sceneitemremoved)
    + [SceneItemVisibilityChanged](#sceneitemvisibilitychanged)
    + [SceneItemLockChanged](#sceneitemlockchanged)
    + [SceneItemTransformChanged](#sceneitemtransformchanged)
    + [SceneItemSelected](#sceneitemselected)
    + [SceneItemDeselected](#sceneitemdeselected)
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
    + [GetStats](#getstats)
    + [BroadcastCustomMessage](#broadcastcustommessage-1)
    + [GetVideoInfo](#getvideoinfo)
    + [OpenProjector](#openprojector)
    + [TriggerHotkeyByName](#triggerhotkeybyname)
    + [TriggerHotkeyBySequence](#triggerhotkeybysequence)
    + [ExecuteBatch](#executebatch)
    + [Sleep](#sleep)
  * [Media Control](#media-control)
    + [PlayPauseMedia](#playpausemedia)
    + [RestartMedia](#restartmedia)
    + [StopMedia](#stopmedia)
    + [NextMedia](#nextmedia)
    + [PreviousMedia](#previousmedia)
    + [GetMediaDuration](#getmediaduration)
    + [GetMediaTime](#getmediatime)
    + [SetMediaTime](#setmediatime)
    + [ScrubMedia](#scrubmedia)
    + [GetMediaState](#getmediastate)
  * [Sources](#sources-1)
    + [GetMediaSourcesList](#getmediasourceslist)
    + [CreateSource](#createsource)
    + [GetSourcesList](#getsourceslist)
    + [GetSourceTypesList](#getsourcetypeslist)
    + [GetVolume](#getvolume)
    + [SetVolume](#setvolume)
    + [SetTracks](#settracks)
    + [GetTracks](#gettracks)
    + [GetMute](#getmute)
    + [SetMute](#setmute)
    + [ToggleMute](#togglemute)
    + [GetSourceActive](#getsourceactive)
    + [GetAudioActive](#getaudioactive)
    + [SetSourceName](#setsourcename)
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
    + [GetSpecialSources](#getspecialsources)
    + [GetSourceFilters](#getsourcefilters)
    + [GetSourceFilterInfo](#getsourcefilterinfo)
    + [AddFilterToSource](#addfiltertosource)
    + [RemoveFilterFromSource](#removefilterfromsource)
    + [ReorderSourceFilter](#reordersourcefilter)
    + [MoveSourceFilter](#movesourcefilter)
    + [SetSourceFilterSettings](#setsourcefiltersettings)
    + [SetSourceFilterVisibility](#setsourcefiltervisibility)
    + [GetAudioMonitorType](#getaudiomonitortype)
    + [SetAudioMonitorType](#setaudiomonitortype)
    + [GetSourceDefaultSettings](#getsourcedefaultsettings)
    + [TakeSourceScreenshot](#takesourcescreenshot)
    + [RefreshBrowserSource](#refreshbrowsersource)
  * [Outputs](#outputs)
    + [ListOutputs](#listoutputs)
    + [GetOutputInfo](#getoutputinfo)
    + [StartOutput](#startoutput)
    + [StopOutput](#stopoutput)
  * [Profiles](#profiles-1)
    + [SetCurrentProfile](#setcurrentprofile)
    + [GetCurrentProfile](#getcurrentprofile)
    + [ListProfiles](#listprofiles)
  * [Recording](#recording-1)
    + [GetRecordingStatus](#getrecordingstatus)
    + [StartStopRecording](#startstoprecording)
    + [StartRecording](#startrecording)
    + [StopRecording](#stoprecording)
    + [PauseRecording](#pauserecording)
    + [ResumeRecording](#resumerecording)
    + [SetRecordingFolder](#setrecordingfolder)
    + [GetRecordingFolder](#getrecordingfolder)
  * [Replay Buffer](#replay-buffer-1)
    + [GetReplayBufferStatus](#getreplaybufferstatus)
    + [StartStopReplayBuffer](#startstopreplaybuffer)
    + [StartReplayBuffer](#startreplaybuffer)
    + [StopReplayBuffer](#stopreplaybuffer)
    + [SaveReplayBuffer](#savereplaybuffer)
  * [Scene Collections](#scene-collections)
    + [SetCurrentSceneCollection](#setcurrentscenecollection)
    + [GetCurrentSceneCollection](#getcurrentscenecollection)
    + [ListSceneCollections](#listscenecollections)
  * [Scene Items](#scene-items-1)
    + [GetSceneItemList](#getsceneitemlist)
    + [GetSceneItemProperties](#getsceneitemproperties)
    + [SetSceneItemProperties](#setsceneitemproperties)
    + [ResetSceneItem](#resetsceneitem)
    + [SetSceneItemRender](#setsceneitemrender)
    + [SetSceneItemPosition](#setsceneitemposition)
    + [SetSceneItemTransform](#setsceneitemtransform)
    + [SetSceneItemCrop](#setsceneitemcrop)
    + [DeleteSceneItem](#deletesceneitem)
    + [AddSceneItem](#addsceneitem)
    + [DuplicateSceneItem](#duplicatesceneitem)
  * [Scenes](#scenes-1)
    + [SetCurrentScene](#setcurrentscene)
    + [GetCurrentScene](#getcurrentscene)
    + [GetSceneList](#getscenelist)
    + [CreateScene](#createscene)
    + [ReorderSceneItems](#reordersceneitems)
    + [SetSceneTransitionOverride](#setscenetransitionoverride)
    + [RemoveSceneTransitionOverride](#removescenetransitionoverride)
    + [GetSceneTransitionOverride](#getscenetransitionoverride)
  * [Streaming](#streaming-1)
    + [GetStreamingStatus](#getstreamingstatus)
    + [StartStopStreaming](#startstopstreaming)
    + [StartStreaming](#startstreaming)
    + [StopStreaming](#stopstreaming)
    + [SetStreamSettings](#setstreamsettings)
    + [GetStreamSettings](#getstreamsettings)
    + [SaveStreamSettings](#savestreamsettings)
    + [SendCaptions](#sendcaptions)
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
    + [GetTransitionPosition](#gettransitionposition)
    + [GetTransitionSettings](#gettransitionsettings)
    + [SetTransitionSettings](#settransitionsettings)
    + [ReleaseTBar](#releasetbar)
    + [SetTBarPosition](#settbarposition)
  * [Virtual Cam](#virtual-cam-1)
    + [GetVirtualCamStatus](#getvirtualcamstatus)
    + [StartStopVirtualCam](#startstopvirtualcam)
    + [StartVirtualCam](#startvirtualcam)
    + [StopVirtualCam](#stopvirtualcam)

<!-- tocstop -->

# Typedefs
These are complex types, such as `Source` and `Scene`, which are used as arguments or return values in multiple requests and/or events. 


## SceneItem
| Name | Type  | Description |
| ---- | :---: | ------------|
| `cy` | _Number_ |  |
| `cx` | _Number_ |  |
| `alignment` | _Number_ | The point on the source that the item is manipulated from. The sum of 1=Left or 2=Right, and 4=Top or 8=Bottom, or omit to center on that axis. |
| `name` | _String_ | The name of this Scene Item. |
| `id` | _int_ | Scene item ID |
| `render` | _Boolean_ | Whether or not this Scene Item is set to "visible". |
| `muted` | _Boolean_ | Whether or not this Scene Item is muted. |
| `locked` | _Boolean_ | Whether or not this Scene Item is locked and can't be moved around |
| `source_cx` | _Number_ |  |
| `source_cy` | _Number_ |  |
| `type` | _String_ | Source type. Value is one of the following: "input", "filter", "transition", "scene" or "unknown" |
| `volume` | _Number_ |  |
| `x` | _Number_ |  |
| `y` | _Number_ |  |
| `parentGroupName` | _String (optional)_ | Name of the item's parent (if this item belongs to a group) |
| `groupChildren` | _Array&lt;SceneItem&gt; (optional)_ | List of children (if this item is a group) |
## SceneItemTransform
| Name | Type  | Description |
| ---- | :---: | ------------|
| `position.x` | _double_ | The x position of the scene item from the left. |
| `position.y` | _double_ | The y position of the scene item from the top. |
| `position.alignment` | _int_ | The point on the scene item that the item is manipulated from. |
| `rotation` | _double_ | The clockwise rotation of the scene item in degrees around the point of alignment. |
| `scale.x` | _double_ | The x-scale factor of the scene item. |
| `scale.y` | _double_ | The y-scale factor of the scene item. |
| `scale.filter` | _String_ | The scale filter of the source. Can be "OBS_SCALE_DISABLE", "OBS_SCALE_POINT", "OBS_SCALE_BICUBIC", "OBS_SCALE_BILINEAR", "OBS_SCALE_LANCZOS" or "OBS_SCALE_AREA". |
| `crop.top` | _int_ | The number of pixels cropped off the top of the scene item before scaling. |
| `crop.right` | _int_ | The number of pixels cropped off the right of the scene item before scaling. |
| `crop.bottom` | _int_ | The number of pixels cropped off the bottom of the scene item before scaling. |
| `crop.left` | _int_ | The number of pixels cropped off the left of the scene item before scaling. |
| `visible` | _bool_ | If the scene item is visible. |
| `locked` | _bool_ | If the scene item is locked in position. |
| `bounds.type` | _String_ | Type of bounding box. Can be "OBS_BOUNDS_STRETCH", "OBS_BOUNDS_SCALE_INNER", "OBS_BOUNDS_SCALE_OUTER", "OBS_BOUNDS_SCALE_TO_WIDTH", "OBS_BOUNDS_SCALE_TO_HEIGHT", "OBS_BOUNDS_MAX_ONLY" or "OBS_BOUNDS_NONE". |
| `bounds.alignment` | _int_ | Alignment of the bounding box. |
| `bounds.x` | _double_ | Width of the bounding box. |
| `bounds.y` | _double_ | Height of the bounding box. |
| `sourceWidth` | _int_ | Base width (without scaling) of the source |
| `sourceHeight` | _int_ | Base source (without scaling) of the source |
| `width` | _double_ | Scene item width (base source width multiplied by the horizontal scaling factor) |
| `height` | _double_ | Scene item height (base source height multiplied by the vertical scaling factor) |
| `parentGroupName` | _String (optional)_ | Name of the item's parent (if this item belongs to a group) |
| `groupChildren` | _Array&lt;SceneItemTransform&gt; (optional)_ | List of children (if this item is a group) |
## OBSStats
| Name | Type  | Description |
| ---- | :---: | ------------|
| `fps` | _double_ | Current framerate. |
| `render-total-frames` | _int_ | Number of frames rendered |
| `render-missed-frames` | _int_ | Number of frames missed due to rendering lag |
| `output-total-frames` | _int_ | Number of frames outputted |
| `output-skipped-frames` | _int_ | Number of frames skipped due to encoding lag |
| `average-frame-time` | _double_ | Average frame render time (in milliseconds) |
| `cpu-usage` | _double_ | Current CPU usage (percentage) |
| `memory-usage` | _double_ | Current RAM usage (in megabytes) |
| `free-disk-space` | _double_ | Free recording disk space (in megabytes) |
## Output
| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | Output name |
| `type` | _String_ | Output type/kind |
| `width` | _int_ | Video output width |
| `height` | _int_ | Video output height |
| `flags` | _Object_ | Output flags |
| `flags.rawValue` | _int_ | Raw flags value |
| `flags.audio` | _boolean_ | Output uses audio |
| `flags.video` | _boolean_ | Output uses video |
| `flags.encoded` | _boolean_ | Output is encoded |
| `flags.multiTrack` | _boolean_ | Output uses several audio tracks |
| `flags.service` | _boolean_ | Output uses a service |
| `settings` | _Object_ | Output settings |
| `active` | _boolean_ | Output status (active or not) |
| `reconnecting` | _boolean_ | Output reconnection status (reconnecting or not) |
| `congestion` | _double_ | Output congestion |
| `totalFrames` | _int_ | Number of frames sent |
| `droppedFrames` | _int_ | Number of frames dropped |
| `totalBytes` | _int_ | Total bytes sent |
## ScenesCollection
| Name | Type  | Description |
| ---- | :---: | ------------|
| `sc-name` | _String_ | Name of the scene collection |
## Scene
| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | Name of the currently active scene. |
| `sources` | _Array&lt;SceneItem&gt;_ | Ordered list of the current scene's source items. |



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
| `sources` | _Array&lt;SceneItem&gt;_ | List of scene items in the new scene. Same specification as [`GetCurrentScene`](#getcurrentscene). |


---

### ScenesChanged


- Added in v0.3



Note: This event is not fired when the scenes are reordered.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scenes` | _Array&lt;Scene&gt;_ | Scenes list. |


---

### SceneCollectionChanged


- Added in v4.0.0

Triggered when switching to another scene collection or when renaming the current scene collection.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sceneCollection` | _String_ | Name of the new current scene collection. |


---

### SceneCollectionListChanged


- Added in v4.0.0

Triggered when a scene collection is created, added, renamed, or removed.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sceneCollections` | _Array&lt;Object&gt;_ | Scene collections list. |
| `sceneCollections.*.name` | _String_ | Scene collection name. |


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

| Name | Type  | Description |
| ---- | :---: | ------------|
| `transitions` | _Array&lt;Object&gt;_ | Transitions list. |
| `transitions.*.name` | _String_ | Transition name. |


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
| `type` | _String_ | Transition type. |
| `duration` | _int_ | Transition duration (in milliseconds). Will be -1 for any transition with a fixed duration, such as a Stinger, due to limitations of the OBS API. |
| `from-scene` | _String (optional)_ | Source scene of the transition |
| `to-scene` | _String_ | Destination scene of the transition |


---

### TransitionEnd


- Added in v4.8.0

A transition (other than "cut") has ended.
Note: The `from-scene` field is not available in TransitionEnd.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | Transition name. |
| `type` | _String_ | Transition type. |
| `duration` | _int_ | Transition duration (in milliseconds). |
| `to-scene` | _String_ | Destination scene of the transition |


---

### TransitionVideoEnd


- Added in v4.8.0

A stinger transition has finished playing its video.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | Transition name. |
| `type` | _String_ | Transition type. |
| `duration` | _int_ | Transition duration (in milliseconds). |
| `from-scene` | _String (optional)_ | Source scene of the transition |
| `to-scene` | _String_ | Destination scene of the transition |


---

## Profiles

### ProfileChanged


- Added in v4.0.0

Triggered when switching to another profile or when renaming the current profile.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `profile` | _String_ | Name of the new current profile. |


---

### ProfileListChanged


- Added in v4.0.0

Triggered when a profile is created, added, renamed, or removed.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `profiles` | _Array&lt;Object&gt;_ | Profiles list. |
| `profiles.*.name` | _String_ | Profile name. |


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

Emitted every 2 seconds when stream is active.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `streaming` | _boolean_ | Current streaming state. |
| `recording` | _boolean_ | Current recording state. |
| `replay-buffer-active` | _boolean_ | Replay Buffer status |
| `bytes-per-sec` | _int_ | Amount of data per second (in bytes) transmitted by the stream encoder. |
| `kbits-per-sec` | _int_ | Amount of data per second (in kilobits) transmitted by the stream encoder. |
| `strain` | _double_ | Percentage of dropped frames. |
| `total-stream-time` | _int_ | Total time (in seconds) since the stream started. |
| `num-total-frames` | _int_ | Total number of frames transmitted since the stream started. |
| `num-dropped-frames` | _int_ | Number of frames dropped by the encoder since the stream started. |
| `fps` | _double_ | Current framerate. |
| `render-total-frames` | _int_ | Number of frames rendered |
| `render-missed-frames` | _int_ | Number of frames missed due to rendering lag |
| `output-total-frames` | _int_ | Number of frames outputted |
| `output-skipped-frames` | _int_ | Number of frames skipped due to encoding lag |
| `average-frame-time` | _double_ | Average frame time (in milliseconds) |
| `cpu-usage` | _double_ | Current CPU usage (percentage) |
| `memory-usage` | _double_ | Current RAM usage (in megabytes) |
| `free-disk-space` | _double_ | Free recording disk space (in megabytes) |
| `preview-only` | _boolean_ | Always false (retrocompatibility). |


---

## Recording

### RecordingStarting


- Added in v0.3



Note: `recordingFilename` is not provided in this event because this information
is not available at the time this event is emitted.

**Response Items:**

_No additional response items._

---

### RecordingStarted


- Added in v0.3

Recording started successfully.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `recordingFilename` | _String_ | Absolute path to the file of the current recording. |


---

### RecordingStopping


- Added in v0.3

A request to stop recording has been issued.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `recordingFilename` | _String_ | Absolute path to the file of the current recording. |


---

### RecordingStopped


- Added in v0.3

Recording stopped successfully.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `recordingFilename` | _String_ | Absolute path to the file of the current recording. |


---

### RecordingPaused


- Added in v4.7.0

Current recording paused

**Response Items:**

_No additional response items._

---

### RecordingResumed


- Added in v4.7.0

Current recording resumed

**Response Items:**

_No additional response items._

---

## Virtual Cam

### VirtualCamStarted


- Added in v4.9.1

Virtual cam started successfully.

**Response Items:**

_No additional response items._

---

### VirtualCamStopped


- Added in v4.9.1

Virtual cam stopped successfully.

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

A request to stop the replay buffer has been issued.

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


- Added in vv0.3

Emitted every 2 seconds after enabling it by calling SetHeartbeat.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `pulse` | _boolean_ | Toggles between every JSON message as an "I am alive" indicator. |
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
| `stats` | _OBSStats_ | OBS Stats |


---

### BroadcastCustomMessage


- Added in v4.7.0

A custom broadcast message, sent by the server, requested by one of the websocket clients.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `realm` | _String_ | Identifier provided by the sender |
| `data` | _Object_ | User-defined data |


---

## Sources

### SourceCreated


- Added in v4.6.0

A source has been created. A source can be an input, a scene or a transition.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceType` | _String_ | Source type. Can be "input", "scene", "transition" or "filter". |
| `sourceKind` | _String_ | Source kind. |
| `sourceSettings` | _Object_ | Source settings |


---

### SourceDestroyed


- Added in v4.6.0

A source has been destroyed/removed. A source can be an input, a scene or a transition.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceType` | _String_ | Source type. Can be "input", "scene", "transition" or "filter". |
| `sourceKind` | _String_ | Source kind. |


---

### SourceVolumeChanged


- Added in v4.6.0

The volume of a source has changed.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `volume` | _float_ | Source volume |
| `volumeDb` | _float_ | Source volume in Decibel |


---

### SourceMuteStateChanged


- Added in v4.6.0

A source has been muted or unmuted.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `muted` | _boolean_ | Mute status of the source |


---

### SourceAudioDeactivated


- Added in v4.9.0

A source has removed audio.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |


---

### SourceAudioActivated


- Added in v4.9.0

A source has added audio.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |


---

### SourceAudioSyncOffsetChanged


- Added in v4.6.0

The audio sync offset of a source has changed.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `syncOffset` | _int_ | Audio sync offset of the source (in nanoseconds) |


---

### SourceAudioMixersChanged


- Added in v4.6.0

Audio mixer routing changed on a source.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `mixers` | _Array&lt;Object&gt;_ | Routing status of the source for each audio mixer (array of 6 values) |
| `mixers.*.id` | _int_ | Mixer number |
| `mixers.*.enabled` | _boolean_ | Routing status |
| `hexMixersValue` | _String_ | Raw mixer flags (little-endian, one bit per mixer) as an hexadecimal value |


---

### SourceRenamed


- Added in v4.6.0

A source has been renamed.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `previousName` | _String_ | Previous source name |
| `newName` | _String_ | New source name |
| `sourceType` | _String_ | Type of source (input, scene, filter, transition) |


---

### SourceFilterAdded


- Added in v4.6.0

A filter was added to a source.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `filterName` | _String_ | Filter name |
| `filterType` | _String_ | Filter type |
| `filterSettings` | _Object_ | Filter settings |


---

### SourceFilterRemoved


- Added in v4.6.0

A filter was removed from a source.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `filterName` | _String_ | Filter name |
| `filterType` | _String_ | Filter type |


---

### SourceFilterVisibilityChanged


- Added in v4.7.0

The visibility/enabled state of a filter changed

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `filterName` | _String_ | Filter name |
| `filterEnabled` | _Boolean_ | New filter state |


---

### SourceFiltersReordered


- Added in v4.6.0

Filters in a source have been reordered.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `filters` | _Array&lt;Object&gt;_ | Ordered Filters list |
| `filters.*.name` | _String_ | Filter name |
| `filters.*.type` | _String_ | Filter type |
| `filters.*.enabled` | _boolean_ | Filter visibility status |


---

## Media

### MediaPlaying


- Added in v4.9.0



Note: This event is only emitted when something actively controls the media/VLC source. In other words, the source will never emit this on its own naturally.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceKind` | _String_ | The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`) |


---

### MediaPaused


- Added in v4.9.0



Note: This event is only emitted when something actively controls the media/VLC source. In other words, the source will never emit this on its own naturally.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceKind` | _String_ | The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`) |


---

### MediaRestarted


- Added in v4.9.0



Note: This event is only emitted when something actively controls the media/VLC source. In other words, the source will never emit this on its own naturally.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceKind` | _String_ | The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`) |


---

### MediaStopped


- Added in v4.9.0



Note: This event is only emitted when something actively controls the media/VLC source. In other words, the source will never emit this on its own naturally.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceKind` | _String_ | The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`) |


---

### MediaNext


- Added in v4.9.0



Note: This event is only emitted when something actively controls the media/VLC source. In other words, the source will never emit this on its own naturally.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceKind` | _String_ | The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`) |


---

### MediaPrevious


- Added in v4.9.0



Note: This event is only emitted when something actively controls the media/VLC source. In other words, the source will never emit this on its own naturally.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceKind` | _String_ | The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`) |


---

### MediaStarted


- Added in v4.9.0



Note: These events are emitted by the OBS sources themselves. For example when the media file starts playing. The behavior depends on the type of media source being used.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceKind` | _String_ | The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`) |


---

### MediaEnded


- Added in v4.9.0



Note: These events are emitted by the OBS sources themselves. For example when the media file ends. The behavior depends on the type of media source being used.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceKind` | _String_ | The ID type of the source (Eg. `vlc_source` or `ffmpeg_source`) |


---

## Scene Items

### SourceOrderChanged


- Added in v4.0.0

Scene items within a scene have been reordered.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene where items have been reordered. |
| `scene-items` | _Array&lt;Object&gt;_ | Ordered list of scene items |
| `scene-items.*.source-name` | _String_ | Item source name |
| `scene-items.*.item-id` | _int_ | Scene item unique ID |


---

### SceneItemAdded


- Added in v4.0.0

A scene item has been added to a scene.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene. |
| `item-name` | _String_ | Name of the item added to the scene. |
| `item-id` | _int_ | Scene item ID |


---

### SceneItemRemoved


- Added in v4.0.0

A scene item has been removed from a scene.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene. |
| `item-name` | _String_ | Name of the item removed from the scene. |
| `item-id` | _int_ | Scene item ID |


---

### SceneItemVisibilityChanged


- Added in v4.0.0

A scene item's visibility has been toggled.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene. |
| `item-name` | _String_ | Name of the item in the scene. |
| `item-id` | _int_ | Scene item ID |
| `item-visible` | _boolean_ | New visibility state of the item. |


---

### SceneItemLockChanged


- Added in v4.8.0

A scene item's locked status has been toggled.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene. |
| `item-name` | _String_ | Name of the item in the scene. |
| `item-id` | _int_ | Scene item ID |
| `item-locked` | _boolean_ | New locked state of the item. |


---

### SceneItemTransformChanged


- Added in v4.6.0

A scene item's transform has been changed.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene. |
| `item-name` | _String_ | Name of the item in the scene. |
| `item-id` | _int_ | Scene item ID |
| `transform` | _SceneItemTransform_ | Scene item transform properties |


---

### SceneItemSelected


- Added in v4.6.0

A scene item is selected.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene. |
| `item-name` | _String_ | Name of the item in the scene. |
| `item-id` | _int_ | Name of the item in the scene. |


---

### SceneItemDeselected


- Added in v4.6.0

A scene item is deselected.

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene. |
| `item-name` | _String_ | Name of the item in the scene. |
| `item-id` | _int_ | Name of the item in the scene. |


---

## Studio Mode

### PreviewSceneChanged


- Added in v4.1.0

The selected preview scene has changed (only available in Studio Mode).

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String_ | Name of the scene being previewed. |
| `sources` | _Array&lt;SceneItem&gt;_ | List of sources composing the scene. Same specification as [`GetCurrentScene`](#getcurrentscene). |


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
- `error` _String (Optional)_: An error message accompanying an `error` status.

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
| `supported-image-export-formats` | _String_ | List of supported formats for features that use image export (like the TakeSourceScreenshot request type) formatted as a comma-separated list string |


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

- **⚠️ Deprecated. Since 4.9.0. Please poll the appropriate data using requests. Will be removed in v5.0.0. ⚠️**

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

### GetStats


- Added in v4.6.0

Get OBS stats (almost the same info as provided in OBS' stats window)

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `stats` | _OBSStats_ | [OBS stats](#obsstats) |


---

### BroadcastCustomMessage


- Added in v4.7.0

Broadcast custom message to all connected WebSocket clients

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `realm` | _String_ | Identifier to be choosen by the client |
| `data` | _Object_ | User-defined data |


**Response Items:**

_No additional response items._

---

### GetVideoInfo


- Added in v4.6.0

Get basic OBS video information

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `baseWidth` | _int_ | Base (canvas) width |
| `baseHeight` | _int_ | Base (canvas) height |
| `outputWidth` | _int_ | Output width |
| `outputHeight` | _int_ | Output height |
| `scaleType` | _String_ | Scaling method used if output size differs from base size |
| `fps` | _double_ | Frames rendered per second |
| `videoFormat` | _String_ | Video color format |
| `colorSpace` | _String_ | Color space for YUV |
| `colorRange` | _String_ | Color range (full or partial) |


---

### OpenProjector


- Added in v4.8.0

Open a projector window or create a projector on a monitor. Requires OBS v24.0.4 or newer.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `type` | _String (Optional)_ | Type of projector: `Preview` (default), `Source`, `Scene`, `StudioProgram`, or `Multiview` (case insensitive). |
| `monitor` | _int (Optional)_ | Monitor to open the projector on. If -1 or omitted, opens a window. |
| `geometry` | _String (Optional)_ | Size and position of the projector window (only if monitor is -1). Encoded in Base64 using [Qt's geometry encoding](https://doc.qt.io/qt-5/qwidget.html#saveGeometry). Corresponds to OBS's saved projectors. |
| `name` | _String (Optional)_ | Name of the source or scene to be displayed (ignored for other projector types). |


**Response Items:**

_No additional response items._

---

### TriggerHotkeyByName


- Added in v4.9.0

Executes hotkey routine, identified by hotkey unique name

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `hotkeyName` | _String_ | Unique name of the hotkey, as defined when registering the hotkey (e.g. "ReplayBuffer.Save") |


**Response Items:**

_No additional response items._

---

### TriggerHotkeyBySequence


- Added in v4.9.0

Executes hotkey routine, identified by bound combination of keys. A single key combination might trigger multiple hotkey routines depending on user settings

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `keyId` | _String_ | Main key identifier (e.g. `OBS_KEY_A` for key "A"). Available identifiers [here](https://github.com/obsproject/obs-studio/blob/master/libobs/obs-hotkeys.h) |
| `keyModifiers` | _Object (Optional)_ | Optional key modifiers object. False entries can be ommitted |
| `keyModifiers.shift` | _boolean_ | Trigger Shift Key |
| `keyModifiers.alt` | _boolean_ | Trigger Alt Key |
| `keyModifiers.control` | _boolean_ | Trigger Control (Ctrl) Key |
| `keyModifiers.command` | _boolean_ | Trigger Command Key (Mac) |


**Response Items:**

_No additional response items._

---

### ExecuteBatch


- Added in v4.9.0

Executes a list of requests sequentially (one-by-one on the same thread).

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `requests` | _Array&lt;Object&gt;_ | Array of requests to perform. Executed in order. |
| `requests.*.request-type` | _String_ | Request type. Eg. `GetVersion`. |
| `requests.*.message-id` | _String (Optional)_ | ID of the individual request. Can be any string and not required to be unique. Defaults to empty string if not specified. |
| `abortOnFail` | _boolean (Optional)_ | Stop processing batch requests if one returns a failure. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `results` | _Array&lt;Object&gt;_ | Batch requests results, ordered sequentially. |
| `results.*.message-id` | _String_ | ID of the individual request which was originally provided by the client. |
| `results.*.status` | _String_ | Status response as string. Either `ok` or `error`. |
| `results.*.error` | _String (Optional)_ | Error message accompanying an `error` status. |


---

### Sleep


- Added in v4.9.1

Waits for the specified duration. Designed to be used in `ExecuteBatch` operations.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sleepMillis` | _int_ | Delay in milliseconds to wait before continuing. |


**Response Items:**

_No additional response items._

---

## Media Control

### PlayPauseMedia


- Added in v4.9.0

Pause or play a media source. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
Note :Leaving out `playPause` toggles the current pause state

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |
| `playPause` | _boolean_ | (optional) Whether to pause or play the source. `false` for play, `true` for pause. |


**Response Items:**

_No additional response items._

---

### RestartMedia


- Added in v4.9.0

Restart a media source. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |


**Response Items:**

_No additional response items._

---

### StopMedia


- Added in v4.9.0

Stop a media source. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |


**Response Items:**

_No additional response items._

---

### NextMedia


- Added in v4.9.0

Skip to the next media item in the playlist. Supports only vlc media source (as of OBS v25.0.8)

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |


**Response Items:**

_No additional response items._

---

### PreviousMedia


- Added in v4.9.0

Go to the previous media item in the playlist. Supports only vlc media source (as of OBS v25.0.8)

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |


**Response Items:**

_No additional response items._

---

### GetMediaDuration


- Added in v4.9.0

Get the length of media in milliseconds. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
Note: For some reason, for the first 5 or so seconds that the media is playing, the total duration can be off by upwards of 50ms.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `mediaDuration` | _int_ | The total length of media in milliseconds.. |


---

### GetMediaTime


- Added in v4.9.0

Get the current timestamp of media in milliseconds. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `timestamp` | _int_ | The time in milliseconds since the start of the media. |


---

### SetMediaTime


- Added in v4.9.0

Set the timestamp of a media source. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |
| `timestamp` | _int_ | Milliseconds to set the timestamp to. |


**Response Items:**

_No additional response items._

---

### ScrubMedia


- Added in v4.9.0

Scrub media using a supplied offset. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)
Note: Due to processing/network delays, this request is not perfect. The processing rate of this request has also not been tested.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |
| `timeOffset` | _int_ | Millisecond offset (positive or negative) to offset the current media position. |


**Response Items:**

_No additional response items._

---

### GetMediaState


- Added in v4.9.0

Get the current playing state of a media source. Supports ffmpeg and vlc media sources (as of OBS v25.0.8)

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `mediaState` | _String_ | The media state of the provided source. States: `none`, `playing`, `opening`, `buffering`, `paused`, `stopped`, `ended`, `error`, `unknown` |


---

## Sources

### GetMediaSourcesList


- Added in v4.9.0

List the media state of all media sources (vlc and media source)

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `mediaSources` | _Array&lt;Object&gt;_ | Array of sources |
| `mediaSources.*.sourceName` | _String_ | Unique source name |
| `mediaSources.*.sourceKind` | _String_ | Unique source internal type (a.k.a `ffmpeg_source` or `vlc_source`) |
| `mediaSources.*.mediaState` | _String_ | The current state of media for that source. States: `none`, `playing`, `opening`, `buffering`, `paused`, `stopped`, `ended`, `error`, `unknown` |


---

### CreateSource


- Added in v4.9.0

Create a source and add it as a sceneitem to a scene.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |
| `sourceKind` | _String_ | Source kind, Eg. `vlc_source`. |
| `sceneName` | _String_ | Scene to add the new source to. |
| `sourceSettings` | _Object (optional)_ | Source settings data. |
| `setVisible` | _boolean (optional)_ | Set the created SceneItem as visible or not. Defaults to true |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `itemId` | _int_ | ID of the SceneItem in the scene. |


---

### GetSourcesList


- Added in v4.3.0

List all sources available in the running OBS instance

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sources` | _Array&lt;Object&gt;_ | Array of sources |
| `sources.*.name` | _String_ | Unique source name |
| `sources.*.typeId` | _String_ | Non-unique source internal type (a.k.a kind) |
| `sources.*.type` | _String_ | Source type. Value is one of the following: "input", "filter", "transition", "scene" or "unknown" |


---

### GetSourceTypesList


- Added in v4.3.0

Get a list of all available sources types

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `types` | _Array&lt;Object&gt;_ | Array of source types |
| `types.*.typeId` | _String_ | Non-unique internal source type ID |
| `types.*.displayName` | _String_ | Display name of the source type |
| `types.*.type` | _String_ | Type. Value is one of the following: "input", "filter", "transition" or "other" |
| `types.*.defaultSettings` | _Object_ | Default settings of this source type |
| `types.*.caps` | _Object_ | Source type capabilities |
| `types.*.caps.isAsync` | _Boolean_ | True if source of this type provide frames asynchronously |
| `types.*.caps.hasVideo` | _Boolean_ | True if sources of this type provide video |
| `types.*.caps.hasAudio` | _Boolean_ | True if sources of this type provide audio |
| `types.*.caps.canInteract` | _Boolean_ | True if interaction with this sources of this type is possible |
| `types.*.caps.isComposite` | _Boolean_ | True if sources of this type composite one or more sub-sources |
| `types.*.caps.doNotDuplicate` | _Boolean_ | True if sources of this type should not be fully duplicated |
| `types.*.caps.doNotSelfMonitor` | _Boolean_ | True if sources of this type may cause a feedback loop if it's audio is monitored and shouldn't be |


---

### GetVolume


- Added in v4.0.0

Get the volume of the specified source. Default response uses mul format, NOT SLIDER PERCENTAGE.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Source name. |
| `useDecibel` | _boolean (optional)_ | Output volume in decibels of attenuation instead of amplitude/mul. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | Source name. |
| `volume` | _double_ | Volume of the source. Between `0.0` and `20.0` if using mul, under `26.0` if using dB. |
| `muted` | _boolean_ | Indicates whether the source is muted. |


---

### SetVolume


- Added in v4.0.0

Set the volume of the specified source. Default request format uses mul, NOT SLIDER PERCENTAGE.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Source name. |
| `volume` | _double_ | Desired volume. Must be between `0.0` and `20.0` for mul, and under 26.0 for dB. OBS will interpret dB values under -100.0 as Inf. Note: The OBS volume sliders only reach a maximum of 1.0mul/0.0dB, however OBS actually supports larger values. |
| `useDecibel` | _boolean (optional)_ | Interperet `volume` data as decibels instead of amplitude/mul. |


**Response Items:**

_No additional response items._

---

### SetTracks


- Added in v4.9.1

Changes whether an audio track is active for a source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |
| `track` | _int_ | Audio tracks 1-6. |
| `active` | _boolean_ | Whether audio track is active or not. |


**Response Items:**

_No additional response items._

---

### GetTracks


- Added in v4.9.1

Gets whether an audio track is active for a source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `track1` | _boolean_ |  |
| `track2` | _boolean_ |  |
| `track3` | _boolean_ |  |
| `track4` | _boolean_ |  |
| `track5` | _boolean_ |  |
| `track6` | _boolean_ |  |


---

### GetMute


- Added in v4.0.0

Get the mute status of a specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Source name. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | Source name. |
| `muted` | _boolean_ | Mute status of the source. |


---

### SetMute


- Added in v4.0.0

Sets the mute status of a specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Source name. |
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
| `source` | _String_ | Source name. |


**Response Items:**

_No additional response items._

---

### GetSourceActive


- Added in v4.9.1

Get the source's active status of a specified source (if it is showing in the final mix).

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceActive` | _boolean_ | Source active status of the source. |


---

### GetAudioActive


- Added in v4.9.0

Get the audio's active status of a specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `audioActive` | _boolean_ | Audio active status of the source. |


---

### SetSourceName


- Added in v4.8.0



Note: If the new name already exists as a source, obs-websocket will return an error.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |
| `newName` | _String_ | New source name. |


**Response Items:**

_No additional response items._

---

### SetSyncOffset


- Added in v4.2.0

Set the audio sync offset of a specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Source name. |
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
| `source` | _String_ | Source name. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | Source name. |
| `offset` | _int_ | The audio sync offset (in nanoseconds). |


---

### GetSourceSettings


- Added in v4.3.0

Get settings of the specified source

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |
| `sourceType` | _String (optional)_ | Type of the specified source. Useful for type-checking if you expect a specific settings schema. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceType` | _String_ | Type of the specified source |
| `sourceSettings` | _Object_ | Source settings (varies between source types, may require some probing around). |


---

### SetSourceSettings


- Added in v4.3.0

Set settings of the specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |
| `sourceType` | _String (optional)_ | Type of the specified source. Useful for type-checking to avoid settings a set of settings incompatible with the actual source's type. |
| `sourceSettings` | _Object_ | Source settings (varies between source types, may require some probing around). |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `sourceType` | _String_ | Type of the specified source |
| `sourceSettings` | _Object_ | Updated source settings |


---

### GetTextGDIPlusProperties


- Added in v4.1.0

Get the current properties of a Text GDI Plus source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Source name. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Source name. |
| `align` | _String_ | Text Alignment ("left", "center", "right"). |
| `bk_color` | _int_ | Background color. |
| `bk_opacity` | _int_ | Background opacity (0-100). |
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


---

### SetTextGDIPlusProperties


- Added in v4.1.0

Set the current properties of a Text GDI Plus source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Name of the source. |
| `align` | _String (optional)_ | Text Alignment ("left", "center", "right"). |
| `bk_color` | _int (optional)_ | Background color. |
| `bk_opacity` | _int (optional)_ | Background opacity (0-100). |
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


- Added in v4.5.0

Get the current properties of a Text Freetype 2 source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Source name. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Source name |
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


---

### SetTextFreetype2Properties


- Added in v4.5.0

Set the current properties of a Text Freetype 2 source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Source name. |
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


**Response Items:**

_No additional response items._

---

### GetBrowserSourceProperties

- **⚠️ Deprecated. Since 4.8.0. Prefer the use of GetSourceSettings. Will be removed in v5.0.0 ⚠️**

- Added in v4.1.0

Get current properties for a Browser Source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Source name. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `source` | _String_ | Source name. |
| `is_local_file` | _boolean_ | Indicates that a local file is in use. |
| `local_file` | _String_ | file path. |
| `url` | _String_ | Url. |
| `css` | _String_ | CSS to inject. |
| `width` | _int_ | Width. |
| `height` | _int_ | Height. |
| `fps` | _int_ | Framerate. |
| `shutdown` | _boolean_ | Indicates whether the source should be shutdown when not visible. |


---

### SetBrowserSourceProperties

- **⚠️ Deprecated. Since 4.8.0. Prefer the use of SetSourceSettings. Will be removed in v5.0.0 ⚠️**

- Added in v4.1.0

Set current properties for a Browser Source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
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

### GetSourceFilters


- Added in v4.5.0

List filters applied to a source

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `filters` | _Array&lt;Object&gt;_ | List of filters for the specified source |
| `filters.*.enabled` | _Boolean_ | Filter status (enabled or not) |
| `filters.*.type` | _String_ | Filter type |
| `filters.*.name` | _String_ | Filter name |
| `filters.*.settings` | _Object_ | Filter settings |


---

### GetSourceFilterInfo


- Added in v4.7.0

List filters applied to a source

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `filterName` | _String_ | Source filter name |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `enabled` | _Boolean_ | Filter status (enabled or not) |
| `type` | _String_ | Filter type |
| `name` | _String_ | Filter name |
| `settings` | _Object_ | Filter settings |


---

### AddFilterToSource


- Added in v4.5.0

Add a new filter to a source. Available source types along with their settings properties are available from `GetSourceTypesList`.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Name of the source on which the filter is added |
| `filterName` | _String_ | Name of the new filter |
| `filterType` | _String_ | Filter type |
| `filterSettings` | _Object_ | Filter settings |


**Response Items:**

_No additional response items._

---

### RemoveFilterFromSource


- Added in v4.5.0

Remove a filter from a source

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Name of the source from which the specified filter is removed |
| `filterName` | _String_ | Name of the filter to remove |


**Response Items:**

_No additional response items._

---

### ReorderSourceFilter


- Added in v4.5.0

Move a filter in the chain (absolute index positioning)

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Name of the source to which the filter belongs |
| `filterName` | _String_ | Name of the filter to reorder |
| `newIndex` | _Integer_ | Desired position of the filter in the chain |


**Response Items:**

_No additional response items._

---

### MoveSourceFilter


- Added in v4.5.0

Move a filter in the chain (relative positioning)

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Name of the source to which the filter belongs |
| `filterName` | _String_ | Name of the filter to reorder |
| `movementType` | _String_ | How to move the filter around in the source's filter chain. Either "up", "down", "top" or "bottom". |


**Response Items:**

_No additional response items._

---

### SetSourceFilterSettings


- Added in v4.5.0

Update settings of a filter

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Name of the source to which the filter belongs |
| `filterName` | _String_ | Name of the filter to reconfigure |
| `filterSettings` | _Object_ | New settings. These will be merged to the current filter settings. |


**Response Items:**

_No additional response items._

---

### SetSourceFilterVisibility


- Added in v4.7.0

Change the visibility/enabled state of a filter

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `filterName` | _String_ | Source filter name |
| `filterEnabled` | _Boolean_ | New filter state |


**Response Items:**

_No additional response items._

---

### GetAudioMonitorType


- Added in v4.8.0

Get the audio monitoring type of the specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `monitorType` | _String_ | The monitor type in use. Options: `none`, `monitorOnly`, `monitorAndOutput`. |


---

### SetAudioMonitorType


- Added in v4.8.0

Set the audio monitoring type of the specified source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |
| `monitorType` | _String_ | The monitor type to use. Options: `none`, `monitorOnly`, `monitorAndOutput`. |


**Response Items:**

_No additional response items._

---

### GetSourceDefaultSettings


- Added in v4.9.0

Get the default settings for a given source type.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceKind` | _String_ | Source kind. Also called "source id" in libobs terminology. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceKind` | _String_ | Source kind. Same value as the `sourceKind` parameter. |
| `defaultSettings` | _Object_ | Settings object for source. |


---

### TakeSourceScreenshot


- Added in v4.6.0



At least `embedPictureFormat` or `saveToFilePath` must be specified.

Clients can specify `width` and `height` parameters to receive scaled pictures. Aspect ratio is
preserved if only one of these two parameters is specified.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String (optional)_ | Source name. Note: Since scenes are also sources, you can also provide a scene name. If not provided, the currently active scene is used. |
| `embedPictureFormat` | _String (optional)_ | Format of the Data URI encoded picture. Can be "png", "jpg", "jpeg" or "bmp" (or any other value supported by Qt's Image module) |
| `saveToFilePath` | _String (optional)_ | Full file path (file extension included) where the captured image is to be saved. Can be in a format different from `pictureFormat`. Can be a relative path. |
| `fileFormat` | _String (optional)_ | Format to save the image file as (one of the values provided in the `supported-image-export-formats` response field of `GetVersion`). If not specified, tries to guess based on file extension. |
| `compressionQuality` | _int (optional)_ | Compression ratio between -1 and 100 to write the image with. -1 is automatic, 1 is smallest file/most compression, 100 is largest file/least compression. Varies with image type. |
| `width` | _int (optional)_ | Screenshot width. Defaults to the source's base width. |
| `height` | _int (optional)_ | Screenshot height. Defaults to the source's base height. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name |
| `img` | _String_ | Image Data URI (if `embedPictureFormat` was specified in the request) |
| `imageFile` | _String_ | Absolute path to the saved image file (if `saveToFilePath` was specified in the request) |


---

### RefreshBrowserSource


- Added in v4.9.0

Refreshes the specified browser source.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sourceName` | _String_ | Source name. |


**Response Items:**

_No additional response items._

---

## Outputs

### ListOutputs


- Added in v4.7.0

List existing outputs

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `outputs` | _Array&lt;Output&gt;_ | Outputs list |


---

### GetOutputInfo


- Added in v4.7.0

Get information about a single output

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `outputName` | _String_ | Output name |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `outputInfo` | _Output_ | Output info |


---

### StartOutput


- Added in v4.7.0



Note: Controlling outputs is an experimental feature of obs-websocket. Some plugins which add outputs to OBS may not function properly when they are controlled in this way.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `outputName` | _String_ | Output name |


**Response Items:**

_No additional response items._

---

### StopOutput


- Added in v4.7.0



Note: Controlling outputs is an experimental feature of obs-websocket. Some plugins which add outputs to OBS may not function properly when they are controlled in this way.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `outputName` | _String_ | Output name |
| `force` | _boolean (optional)_ | Force stop (default: false) |


**Response Items:**

_No additional response items._

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
| `profiles` | _Array&lt;Object&gt;_ | List of available profiles. |
| `profiles.*.profile-name` | _String_ | Filter name |


---

## Recording

### GetRecordingStatus


- Added in v4.9.0

Get current recording status.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `isRecording` | _boolean_ | Current recording status. |
| `isRecordingPaused` | _boolean_ | Whether the recording is paused or not. |
| `recordTimecode` | _String (optional)_ | Time elapsed since recording started (only present if currently recording). |
| `recordingFilename` | _String (optional)_ | Absolute path to the recording file (only present if currently recording). |


---

### StartStopRecording


- Added in v0.3

Toggle recording on or off (depending on the current recording state).

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

### PauseRecording


- Added in v4.7.0

Pause the current recording.
Returns an error if recording is not active or already paused.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### ResumeRecording


- Added in v4.7.0

Resume/unpause the current recording (if paused).
Returns an error if recording is not active or not paused.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### SetRecordingFolder


- Added in v4.1.0



Note: If `SetRecordingFolder` is called while a recording is
in progress, the change won't be applied immediately and will be
effective on the next recording.

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

### GetReplayBufferStatus


- Added in v4.9.0

Get the status of the OBS replay buffer.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `isReplayBufferActive` | _boolean_ | Current recording status. |


---

### StartStopReplayBuffer


- Added in v4.2.0

Toggle the Replay Buffer on/off (depending on the current state of the replay buffer).

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
| `scene-collections` | _Array&lt;ScenesCollection&gt;_ | Scene collections list |


---

## Scene Items

### GetSceneItemList


- Added in v4.9.0

Get a list of all scene items in a scene.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sceneName` | _String (optional)_ | Name of the scene to get the list of scene items from. Defaults to the current scene if not specified. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sceneName` | _String_ | Name of the requested (or current) scene |
| `sceneItems` | _Array&lt;Object&gt;_ | Array of scene items |
| `sceneItems.*.itemId` | _int_ | Unique item id of the source item |
| `sceneItems.*.sourceKind` | _String_ | ID if the scene item's source. For example `vlc_source` or `image_source` |
| `sceneItems.*.sourceName` | _String_ | Name of the scene item's source |
| `sceneItems.*.sourceType` | _String_ | Type of the scene item's source. Either `input`, `group`, or `scene` |


---

### GetSceneItemProperties


- Added in v4.3.0

Gets the scene specific properties of the specified source item.
Coordinates are relative to the item's parent (the scene or group it belongs to).

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | Name of the scene the scene item belongs to. Defaults to the current scene. |
| `item` | _String \| Object_ | Scene Item name (if this field is a string) or specification (if it is an object). |
| `item.name` | _String (optional)_ | Scene Item name (if the `item` field is an object) |
| `item.id` | _int (optional)_ | Scene Item ID (if the `item` field is an object) |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `name` | _String_ | Scene Item name. |
| `itemId` | _int_ | Scene Item ID. |
| `position.x` | _double_ | The x position of the source from the left. |
| `position.y` | _double_ | The y position of the source from the top. |
| `position.alignment` | _int_ | The point on the source that the item is manipulated from. The sum of 1=Left or 2=Right, and 4=Top or 8=Bottom, or omit to center on that axis. |
| `rotation` | _double_ | The clockwise rotation of the item in degrees around the point of alignment. |
| `scale.x` | _double_ | The x-scale factor of the source. |
| `scale.y` | _double_ | The y-scale factor of the source. |
| `scale.filter` | _String_ | The scale filter of the source. Can be "OBS_SCALE_DISABLE", "OBS_SCALE_POINT", "OBS_SCALE_BICUBIC", "OBS_SCALE_BILINEAR", "OBS_SCALE_LANCZOS" or "OBS_SCALE_AREA". |
| `crop.top` | _int_ | The number of pixels cropped off the top of the source before scaling. |
| `crop.right` | _int_ | The number of pixels cropped off the right of the source before scaling. |
| `crop.bottom` | _int_ | The number of pixels cropped off the bottom of the source before scaling. |
| `crop.left` | _int_ | The number of pixels cropped off the left of the source before scaling. |
| `visible` | _bool_ | If the source is visible. |
| `muted` | _bool_ | If the source is muted. |
| `locked` | _bool_ | If the source's transform is locked. |
| `bounds.type` | _String_ | Type of bounding box. Can be "OBS_BOUNDS_STRETCH", "OBS_BOUNDS_SCALE_INNER", "OBS_BOUNDS_SCALE_OUTER", "OBS_BOUNDS_SCALE_TO_WIDTH", "OBS_BOUNDS_SCALE_TO_HEIGHT", "OBS_BOUNDS_MAX_ONLY" or "OBS_BOUNDS_NONE". |
| `bounds.alignment` | _int_ | Alignment of the bounding box. |
| `bounds.x` | _double_ | Width of the bounding box. |
| `bounds.y` | _double_ | Height of the bounding box. |
| `sourceWidth` | _int_ | Base width (without scaling) of the source |
| `sourceHeight` | _int_ | Base source (without scaling) of the source |
| `width` | _double_ | Scene item width (base source width multiplied by the horizontal scaling factor) |
| `height` | _double_ | Scene item height (base source height multiplied by the vertical scaling factor) |
| `parentGroupName` | _String (optional)_ | Name of the item's parent (if this item belongs to a group) |
| `groupChildren` | _Array&lt;SceneItemTransform&gt; (optional)_ | List of children (if this item is a group) |


---

### SetSceneItemProperties


- Added in v4.3.0

Sets the scene specific properties of a source. Unspecified properties will remain unchanged.
Coordinates are relative to the item's parent (the scene or group it belongs to).

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | Name of the scene the source item belongs to. Defaults to the current scene. |
| `item` | _String \| Object_ | Scene Item name (if this field is a string) or specification (if it is an object). |
| `item.name` | _String (optional)_ | Scene Item name (if the `item` field is an object) |
| `item.id` | _int (optional)_ | Scene Item ID (if the `item` field is an object) |
| `position.x` | _double (optional)_ | The new x position of the source. |
| `position.y` | _double (optional)_ | The new y position of the source. |
| `position.alignment` | _int (optional)_ | The new alignment of the source. |
| `rotation` | _double (optional)_ | The new clockwise rotation of the item in degrees. |
| `scale.x` | _double (optional)_ | The new x scale of the item. |
| `scale.y` | _double (optional)_ | The new y scale of the item. |
| `scale.filter` | _String (optional)_ | The new scale filter of the source. Can be "OBS_SCALE_DISABLE", "OBS_SCALE_POINT", "OBS_SCALE_BICUBIC", "OBS_SCALE_BILINEAR", "OBS_SCALE_LANCZOS" or "OBS_SCALE_AREA". |
| `crop.top` | _int (optional)_ | The new amount of pixels cropped off the top of the source before scaling. |
| `crop.bottom` | _int (optional)_ | The new amount of pixels cropped off the bottom of the source before scaling. |
| `crop.left` | _int (optional)_ | The new amount of pixels cropped off the left of the source before scaling. |
| `crop.right` | _int (optional)_ | The new amount of pixels cropped off the right of the source before scaling. |
| `visible` | _bool (optional)_ | The new visibility of the source. 'true' shows source, 'false' hides source. |
| `locked` | _bool (optional)_ | The new locked status of the source. 'true' keeps it in its current position, 'false' allows movement. |
| `bounds.type` | _String (optional)_ | The new bounds type of the source. Can be "OBS_BOUNDS_STRETCH", "OBS_BOUNDS_SCALE_INNER", "OBS_BOUNDS_SCALE_OUTER", "OBS_BOUNDS_SCALE_TO_WIDTH", "OBS_BOUNDS_SCALE_TO_HEIGHT", "OBS_BOUNDS_MAX_ONLY" or "OBS_BOUNDS_NONE". |
| `bounds.alignment` | _int (optional)_ | The new alignment of the bounding box. (0-2, 4-6, 8-10) |
| `bounds.x` | _double (optional)_ | The new width of the bounding box. |
| `bounds.y` | _double (optional)_ | The new height of the bounding box. |


**Response Items:**

_No additional response items._

---

### ResetSceneItem


- Added in v4.2.0

Reset a scene item.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | Name of the scene the scene item belongs to. Defaults to the current scene. |
| `item` | _String \| Object_ | Scene Item name (if this field is a string) or specification (if it is an object). |
| `item.name` | _String (optional)_ | Scene Item name (if the `item` field is an object) |
| `item.id` | _int (optional)_ | Scene Item ID (if the `item` field is an object) |


**Response Items:**

_No additional response items._

---

### SetSceneItemRender


- Added in v0.3

Show or hide a specified source item in a specified scene.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene-name` | _String (optional)_ | Name of the scene the scene item belongs to. Defaults to the currently active scene. |
| `source` | _String (optional)_ | Scene Item name. |
| `item` | _int (optional)_ | Scene Item id |
| `render` | _boolean_ | true = shown ; false = hidden |


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
| `scene-name` | _String (optional)_ | Name of the scene the scene item belongs to. Defaults to the current scene. |
| `item` | _String_ | Scene Item name. |
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
| `scene-name` | _String (optional)_ | Name of the scene the scene item belongs to. Defaults to the current scene. |
| `item` | _String_ | Scene Item name. |
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
| `scene-name` | _String (optional)_ | Name of the scene the scene item belongs to. Defaults to the current scene. |
| `item` | _String_ | Scene Item name. |
| `top` | _int_ | Pixel position of the top of the source item. |
| `bottom` | _int_ | Pixel position of the bottom of the source item. |
| `left` | _int_ | Pixel position of the left of the source item. |
| `right` | _int_ | Pixel position of the right of the source item. |


**Response Items:**

_No additional response items._

---

### DeleteSceneItem


- Added in v4.5.0

Deletes a scene item.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene` | _String (optional)_ | Name of the scene the scene item belongs to. Defaults to the current scene. |
| `item` | _Object_ | Scene item to delete (required) |
| `item.name` | _String_ | Scene Item name (prefer `id`, including both is acceptable). |
| `item.id` | _int_ | Scene Item ID. |


**Response Items:**

_No additional response items._

---

### AddSceneItem


- Added in v4.9.0

Creates a scene item in a scene. In other words, this is how you add a source into a scene.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sceneName` | _String_ | Name of the scene to create the scene item in |
| `sourceName` | _String_ | Name of the source to be added |
| `setVisible` | _boolean (optional)_ | Whether to make the sceneitem visible on creation or not. Default `true` |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `itemId` | _int_ | Numerical ID of the created scene item |


---

### DuplicateSceneItem


- Added in v4.5.0

Duplicates a scene item.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `fromScene` | _String (optional)_ | Name of the scene to copy the item from. Defaults to the current scene. |
| `toScene` | _String (optional)_ | Name of the scene to create the item in. Defaults to the current scene. |
| `item` | _Object_ | Scene Item to duplicate from the source scene (required) |
| `item.name` | _String_ | Scene Item name (prefer `id`, including both is acceptable). |
| `item.id` | _int_ | Scene Item ID. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene` | _String_ | Name of the scene where the new item was created |
| `item` | _Object_ | New item info |
| `item.id` | _int_ | New item ID |
| `item.name` | _String_ | New item name |


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
| `sources` | _Array&lt;SceneItem&gt;_ | Ordered list of the current scene's source items. |


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
| `scenes` | _Array&lt;Scene&gt;_ | Ordered list of the current profile's scenes (See [GetCurrentScene](#getcurrentscene) for more information). |


---

### CreateScene


- Added in v4.9.0

Create a new scene scene.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sceneName` | _String_ | Name of the scene to create. |


**Response Items:**

_No additional response items._

---

### ReorderSceneItems


- Added in v4.5.0

Changes the order of scene items in the requested scene.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `scene` | _String (optional)_ | Name of the scene to reorder (defaults to current). |
| `items` | _Array&lt;Object&gt;_ | Ordered list of objects with name and/or id specified. Id preferred due to uniqueness per scene |
| `items.*.id` | _int (optional)_ | Id of a specific scene item. Unique on a scene by scene basis. |
| `items.*.name` | _String (optional)_ | Name of a scene item. Sufficiently unique if no scene items share sources within the scene. |


**Response Items:**

_No additional response items._

---

### SetSceneTransitionOverride


- Added in v4.8.0

Set a scene to use a specific transition override.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sceneName` | _String_ | Name of the scene to switch to. |
| `transitionName` | _String_ | Name of the transition to use. |
| `transitionDuration` | _int (Optional)_ | Duration in milliseconds of the transition if transition is not fixed. Defaults to the current duration specified in the UI if there is no current override and this value is not given. |


**Response Items:**

_No additional response items._

---

### RemoveSceneTransitionOverride


- Added in v4.8.0

Remove any transition override on a scene.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sceneName` | _String_ | Name of the scene to switch to. |


**Response Items:**

_No additional response items._

---

### GetSceneTransitionOverride


- Added in v4.8.0

Get the current scene transition override.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `sceneName` | _String_ | Name of the scene to switch to. |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `transitionName` | _String_ | Name of the current overriding transition. Empty string if no override is set. |
| `transitionDuration` | _int_ | Transition duration. `-1` if no override is set. |


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
| `recording-paused` | _boolean_ | If recording is paused. |
| `virtualcam` | _boolean_ | Current virtual cam status. |
| `preview-only` | _boolean_ | Always false. Retrocompatibility with OBSRemote. |
| `stream-timecode` | _String (optional)_ | Time elapsed since streaming started (only present if currently streaming). |
| `rec-timecode` | _String (optional)_ | Time elapsed since recording started (only present if currently recording). |
| `virtualcam-timecode` | _String (optional)_ | Time elapsed since virtual cam started (only present if virtual cam currently active). |


---

### StartStopStreaming


- Added in v0.3

Toggle streaming on or off (depending on the current stream state).

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
| `stream` | _Object (optional)_ | Special stream configuration. Note: these won't be saved to OBS' configuration. |
| `stream.type` | _String (optional)_ | If specified ensures the type of stream matches the given type (usually 'rtmp_custom' or 'rtmp_common'). If the currently configured stream type does not match the given stream type, all settings must be specified in the `settings` object or an error will occur when starting the stream. |
| `stream.metadata` | _Object (optional)_ | Adds the given object parameters as encoded query string parameters to the 'key' of the RTMP stream. Used to pass data to the RTMP service about the streaming. May be any String, Numeric, or Boolean field. |
| `stream.settings` | _Object (optional)_ | Settings for the stream. |
| `stream.settings.server` | _String (optional)_ | The publish URL. |
| `stream.settings.key` | _String (optional)_ | The publish key of the stream. |
| `stream.settings.use_auth` | _boolean (optional)_ | Indicates whether authentication should be used when connecting to the streaming server. |
| `stream.settings.username` | _String (optional)_ | If authentication is enabled, the username for the streaming server. Ignored if `use_auth` is not set to `true`. |
| `stream.settings.password` | _String (optional)_ | If authentication is enabled, the password for the streaming server. Ignored if `use_auth` is not set to `true`. |


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
| `settings.use_auth` | _boolean (optional)_ | Indicates whether authentication should be used when connecting to the streaming server. |
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
| `settings.use_auth` | _boolean_ | Indicates whether authentication should be used when connecting to the streaming server. |
| `settings.username` | _String_ | The username to use when accessing the streaming server. Only present if `use_auth` is `true`. |
| `settings.password` | _String_ | The password to use when accessing the streaming server. Only present if `use_auth` is `true`. |


---

### SaveStreamSettings


- Added in v4.1.0

Save the current streaming server settings to disk.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### SendCaptions


- Added in v4.6.0

Send the provided text as embedded CEA-608 caption data.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `text` | _String_ | Captions text |


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
| `sources` | _Array&lt;SceneItem&gt;_ |  |


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

Toggles Studio Mode (depending on the current state of studio mode).

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
| `transitions` | _Array&lt;Object&gt;_ | List of transitions. |
| `transitions.*.name` | _String_ | Name of the transition. |


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

### GetTransitionPosition


- Added in v4.9.0

Get the position of the current transition.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `position` | _double_ | current transition position. This value will be between 0.0 and 1.0. Note: Transition returns 1.0 when not active. |


---

### GetTransitionSettings


- Added in v4.9.0

Get the current settings of a transition

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `transitionName` | _String_ | Transition name |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `transitionSettings` | _Object_ | Current transition settings |


---

### SetTransitionSettings


- Added in v4.9.0

Change the current settings of a transition

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `transitionName` | _String_ | Transition name |
| `transitionSettings` | _Object_ | Transition settings (they can be partial) |


**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `transitionSettings` | _Object_ | Updated transition settings |


---

### ReleaseTBar


- Added in v4.9.0

Release the T-Bar (like a user releasing their mouse button after moving it).
*YOU MUST CALL THIS if you called `SetTBarPosition` with the `release` parameter set to `false`.*

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### SetTBarPosition


- Added in v4.9.0



If your code needs to perform multiple successive T-Bar moves (e.g. : in an animation, or in response to a user moving a T-Bar control in your User Interface), set `release` to false and call `ReleaseTBar` later once the animation/interaction is over.

**Request Fields:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `position` | _double_ | T-Bar position. This value must be between 0.0 and 1.0. |
| `release` | _boolean (optional)_ | Whether or not the T-Bar gets released automatically after setting its new position (like a user releasing their mouse button after moving the T-Bar). Call `ReleaseTBar` manually if you set `release` to false. Defaults to true. |


**Response Items:**

_No additional response items._

---

## Virtual Cam

### GetVirtualCamStatus


- Added in v4.9.1

Get current virtual cam status.

**Request Fields:**

_No specified parameters._

**Response Items:**

| Name | Type  | Description |
| ---- | :---: | ------------|
| `isVirtualCam` | _boolean_ | Current virtual camera status. |
| `virtualCamTimecode` | _String (optional)_ | Time elapsed since virtual cam started (only present if virtual cam currently active). |


---

### StartStopVirtualCam


- Added in v4.9.1

Toggle virtual cam on or off (depending on the current virtual cam state).

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### StartVirtualCam


- Added in v4.9.1

Start virtual cam.
Will return an `error` if virtual cam is already active.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

### StopVirtualCam


- Added in v4.9.1

Stop virtual cam.
Will return an `error` if virtual cam is not active.

**Request Fields:**

_No specified parameters._

**Response Items:**

_No additional response items._

---

