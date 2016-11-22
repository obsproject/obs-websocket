obs-websocket protocol reference
================================

## General Introduction
**This document is still a WIP. Some things are missing (but won't stay like this for long).**

Messages exchanged between the client and the server are JSON objects.  
The protocol in general is based on the OBS Remote protocol created by Bill Hamilton, with new commands specific to OBS Studio.

## Events

### Description
Events are sent exclusively by the server and broadcast to each connected client.  
An event message will contain at least one field :
- **update-type** (string) : the type of event  

Additional fields will be present in the event message depending on the event type.

### Event Types
#### "SwitchScenes"
OBS is switching to another scene.
- **scene-name** (string) : The name of the scene being switched to.

#### "ScenesChanged"
The scene list has been modified (Scenes have been added, removed, or renamed).

#### "StreamStarting"
A request to start streaming has been issued.
- **preview-only** (bool) : Always false.

#### "StreamStarted"
*New in OBS Studio*  
Streaming started successfully.

#### "StreamStopping"
A request to stop streaming has been issued.
- **preview-only** (bool) : Always false.

#### "StreamStopped"
*New in OBS Studio*  
Streaming stopped successfully.

#### "RecordingStarting"
*New in OBS Studio*  
A request to start recording has been issued.

#### "RecordingStarted"
*New in OBS Studio*  
Recording started successfully.

#### "RecordingStopping"
*New in OBS Studio*  
A request to stop streaming has been issued.  

#### "RecordingStopped"
*New in OBS Studio*  
Recording stopped successfully.

#### "StreamStatus"
Sent each second with the following information :  
- **streaming** (bool) : Current Streaming state.
- **recording** (bool) : Current Recording state.
- **preview-only** (bool) : Always false.
- **bytes-per-sec** (integer) : Amount of data (in bytes) transmitted by the stream encoder.
- **strain** (double) : i have no idea what this is
- **total-stream-time** (integer) : Total time (in seconds) since the stream started.
- **num-total-frames** (integer) : Total number of frames transmitted since the stream started.
- **num-dropped-frames** (integer) : Number of frames dropped by the encoder since the stream started.
- **fps** (double) : Current framerate.

#### "Exiting"
*New in OBS Studio*  
OBS is exiting.

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
#### "GetAuthRequired"
#### "Authenticate"
- **auth** (string) : Authentication credentials.

#### "GetCurrentScene"
#### "SetCurrentScene"
- **scene-name** (string) : The name of the scene to switch to.

#### "GetSceneList"
#### "SetSourceRender"
- **source** (string) : The name of the source in the currently active scene.
- **render** (bool) : Render the specified source.

#### "StartStopStreaming"
#### "StartStopRecording"
*New in OBS Studio*  
#### "GetStreamingStatus"
#### "GetTransitionList"
*New in OBS Studio*  
#### "GetCurrentTransition"
*New in OBS Studio*  
#### "SetCurrentTransition"
*New in OBS Studio*  
- **transition-name** (string) : The name of the transition.
