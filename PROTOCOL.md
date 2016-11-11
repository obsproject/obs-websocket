obs-websocket protocol reference
================================

## General introduction
**This document is still a WIP. Some things are missing (but won't stay like this for long).**

Messages exchanged between the client and the server are JSON objects.
The protocol in general in based on the OBS Remote protoctol created by Bill Hamilton, with new commands specific to OBS Studio. 

## Events

### Description
Events are sent exclusively by the server and broadcasted to every client connected to the server.
An event message has at least one field :
- **update-type** (string) : the type of event
Additional fields will be present in the event message if the event type requires it.

### Event types
#### "SwitchScenes" 
OBS is switching to another scene.  
Additional fields :
- **scene-name** : the name of the scene being switched to

#### "ScenesChanged"
The scene list has been modified (scenes added, removed or moved).

#### "StreamStarting"
Streaming is starting but isn't completely started yet.

#### "StreamStarted"
*New in OBS Studio*
Streaming has been started successfully.

#### "StreamStopping"
Streaming is stopping but isn't completely stopped yet.

#### "StreamStopped"
*New in OBS Studio*  
Streaming has been stopped successfully.

#### "RecordingStarting"
*New in OBS Studio*  
Recording is starting but isn't completely started yet.

#### "RecordingStarted"
*New in OBS Studio*  
Recording has been started successfully.

#### "RecordingStopping"
*New in OBS Studio*  
Recording is stopping but isn't completely stopped yet.

#### "RecordingStopped"
*New in OBS Studio*  
Recording has been stopped successfully.

#### "Exiting"
*New in OBS Studio*  
OBS is exiting.

## Requests

### Description
Requests are sent by the client and have at least two fields :
- **"request-type"** (string) : one of the request types listed in the sub-section "Requests".
- **"message-id"** (unsigned integer) : an integer number defined by the client that will be embedded in the response from the server.
Depending on the request type, additional fields are needed in the request message (see the "Request types" section below for more informations).

Once a request is sent, the server processes it and sends a JSON response to the client with the following fields in it :
- **"message-id"** (unsigned integer) : the unsigned integer you specified in the request.
- **"status"** (string) : two possible values : "ok" or "error".
- **"error"** (string) : the error message associated with an error reponse (when "status" equals "error").
Additional fields can be sent in the response if a request type requires it.

### Request types
#### "GetVersion"
#### "GetAuthRequired"
#### "Authenticate"
#### "GetCurrentScene"
#### "SetCurrentScene"
#### "GetSceneList"
#### "SetSourceRender"
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