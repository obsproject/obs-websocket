<!-- This file was generated based on handlebars templates. Do not edit directly! -->

# obs-websocket 5.0.0 protocol reference


## General Introduction
obs-websocket provides a feature-rich RPC communication protocol, giving access to much of OBS's feature set. This document contains everything you should know in order to make a connection and use obs-websocket's functionality to the fullest.

### Design Goals
- Abstraction of identification, events, requests, and batch requests into dedicated message types
- Conformity of request naming using similar terms like `Get`, `Set`, `Get[x]List`, `Start[x]`, `Toggle[x]`
- Conformity of OBS data key names like `sourceName`, `sourceKind`, `sourceType`, `sceneName`, `sceneItemName`
- Error code response system - integer corrosponds to type of error, with optional comment
- Possible support for multiple message encoding options: JSON and MessagePack
- PubSub system - Allow clients to specify which events they do or don't want to receive from OBS
- RPC versioning - Client and server negotiate the latest version of the obs-websocket protocol to communicate with.


## Table of Contents
- [Connecting to obs-websocket](#connecting-to-obs-websocket)
  - [Connection steps](#connection-steps)
  - [Creating an authentication string](#creating-an-authentication-string)
  - [Enumerations](#enumerations)
- [Base message types](#message-types)
  - [OpCode 0 Hello](#hello-opcode-0)
  - [OpCode 1 Identify](#identify-opcode-1)
  - [OpCode 2 Identified](#identified-opcode-2)
  - [OpCode 3 Reidentify](#reidentify-opcode-3)
  - [OpCode 5 Event](#event-opcode-5)
  - [OpCode 6 Request](#request-opcode-6)
  - [OpCode 7 RequestResponse](#requestresponse-opcode-7)
  - [OpCode 8 RequestBatch](#requestbatch-opcode-8)
  - [OpCode 9 RequestBatchResponse](#requestbatchresponse-opcode-9)
- [Events](#events)
- [Requests](#requests)


## Connecting to obs-websocket
Here's info on how to connect to obs-websocket

---

### Connection steps
These steps should be followed precisely. Failure to connect to the server as instructed will likely result in your client being treated in an undefined way.

- Initial HTTP request made to the obs-websocket server.
  - The `Sec-WebSocket-Protocol` header can be used to tell obs-websocket which kind of message encoding to use. By default, obs-websocket uses JSON over text. Available subprotocols:
    - `obswebsocket.json` - JSON over text frames
    - `obswebsocket.msgpack` - MsgPack over binary frames

- Once the connection is upgraded, the websocket server will immediately send an [OpCode 0 `Hello`](#hello-opcode-0) message to the client.

- The client listens for the `Hello` and responds with an [OpCode 1 `Identify`](#identify-opcode-1) containing all appropriate session parameters.
  - If there is an `authentication` key in the `messageData` object, the server requires authentication, and the steps in [Creating an authentication string](#creating-an-authentication-string) should be followed.
  - If there is no `authentication` key, the resulting `Identify` object sent to the server does not require an `authentication` string.
  - The client determines if the server's `rpcVersion` is supported, and if not it provides its closest supported version in `Identify`.

- The server receives and processes the `Identify` sent by the client.
  - If authentication is required and the `Identify` message data does not contain an `authentication` string, or the string is not correct, the connection is closed with [`WebSocketCloseCode::AuthenticationFailed`](#websocketclosecode-enum)
  - If the client has requested an `rpcVersion` which the server cannot use, the connection is closed with [`WebSocketCloseCode::UnsupportedRpcVersion`](#websocketclosecode-enum). This system allows both the server and client to have seamless backwards compatability.
  - If any other parameters are malformed (invalid type, etc), the connection is closed with an appropriate close code.

- Once identification is processed on the server, the server responds to the client with an [OpCode 2 `Identified`](#identified-opcode-2).

- The client will begin receiving events from obs-websocket and may now make requests to obs-websocket.

- At any time after a client has been identified, it may send an [OpCode 3 `Reidentify`](#reidentify-opcode-3) message to update certain allowed session parameters. The server will respond in the same way it does during initial identification.

#### Connection Notes
- If a binary frame is received when using the `obswebsocket.json` (default) subprotocol, or a text frame is received while using the `obswebsocket.msgpack` subprotocol, the connection is closed with [`WebSocketCloseCode::MessageDecodeError`](#websocketclosecode-enum).
- The obs-websocket server listens for any messages containing a `request-type` key in the first level JSON from unidentified clients. If a message matches, the connection is closed with [`WebSocketCloseCode::UnsupportedRpcVersion`](#websocketclosecode-enum) and a warning is logged.
- If a message with a `messageType` is not recognized to the obs-websocket server, the connection is closed with [`WebSocketCloseCode::UnknownOpCode`](#websocketclosecode-enum).
- At no point may the client send any message other than a single `Identify` before it has received an `Identified`. Doing so will result in the connection being closed with [`WebSocketCloseCode::NotIdentified`](#websocketclosecode-enum).

---

### Creating an authentication string
obs-websocket uses SHA256 to transmit authentication credentials. The server starts by sending an object in the `authentication` key of its `Hello` message data. The client processes the authentication challenge and responds via the `authentication` string in the `Identify` message data.

For this guide, we'll be using `supersecretpassword` as the password.

The `authentication` object in `Hello` looks like this (example):
```json
{
    "challenge": "+IxH4CnCiqpX1rM9scsNynZzbOe4KhDeYcTNS3PDaeY=",
    "salt": "lM1GncleQOaCu9lT1yeUZhFYnqhsLLP1G5lAGo3ixaI="
}
```

To generate the authentication string, follow these steps:
- Concatenate the websocket password with the `salt` provided by the server (`password + salt`)
- Generate an SHA256 binary hash of the result and base64 encode it, known as a base64 secret.
- Concatenate the base64 secret with the `challenge` sent by the server (`base64_secret + challenge`)
- Generate a binary SHA256 hash of that result and base64 encode it. You now have your `authentication` string.

For real-world examples of the `authentication` string creation, refer to the obs-websocket client libraries listed on the [README](README.md).

---

### Enumerations
These are the enumeration definitions for various codes used by obs-websocket.

#### WebSocketOpCode Enum
```cpp
enum WebSocketOpCode {
    Hello = 0,
    Identify = 1,
    Identified = 2,
    Reidentify = 3,
    Event = 5,
    Request = 6,
    RequestResponse = 7,
    RequestBatch = 8,
    RequestBatchResponse = 9,
};
```

#### WebSocketCloseCode Enum
```cpp
enum WebSocketCloseCode {
    // Internal only
    DontClose = 0,
    // Reserved
    UnknownReason = 4000,
    // The server was unable to decode the incoming websocket message
    MessageDecodeError = 4002,
    // A data key is missing but required
    MissingDataKey = 4003,
    // A data key has an invalid type
    InvalidDataKeyType = 4004,
    // The specified `op` was invalid or missing
    UnknownOpCode = 4005,
    // The client sent a websocket message without first sending `Identify` message
    NotIdentified = 4006,
    // The client sent an `Identify` message while already identified
    AlreadyIdentified = 4007,
    // The authentication attempt (via `Identify`) failed
    AuthenticationFailed = 4008,
    // The server detected the usage of an old version of the obs-websocket protocol.
    UnsupportedRpcVersion = 4009,
    // The websocket session has been invalidated by the obs-websocket server.
    SessionInvalidated = 4010,
};
```

#### EventSubscriptions Enum
```cpp
enum EventSubscription {
    // Set subscriptions to 0 to disable all events
    None = 0,
    // Receive events in the `General` category
    General = (1 << 0),
    // Receive events in the `Config` category
    Config = (1 << 1),
    // Receive events in the `Scenes` category
    Scenes = (1 << 2),
    // Receive events in the `Inputs` category
    Inputs = (1 << 3),
    // Receive events in the `Transitions` category
    Transitions = (1 << 4),
    // Receive events in the `Filters` category
    Filters = (1 << 5),
    // Receive events in the `Outputs` category
    Outputs = (1 << 6),
    // Receive events in the `Scene Items` category
    SceneItems = (1 << 7),
    // Receive events in the `MediaInputs` category
    MediaInputs = (1 << 8),
    // Receive all event categories
    All = (General | Config | Scenes | Inputs | Transitions | Filters | Outputs | SceneItems | MediaInputs),
    // InputVolumeMeters event (high-volume)
    InputVolumeMeters = (1 << 9),
    // InputActiveStateChanged event (high-volume)
    InputActiveStateChanged = (1 << 10),
    // InputShowStateChanged event (high-volume)
    InputShowStateChanged = (1 << 11),
};
```
Subscriptions are a bitmask system. In many languages, to generate a bitmask that subscribes to `General` and `Scenes`, you would do: `subscriptions = ((1 << 0) | (1 << 2))`

#### RequestStatus Enum
```cpp
enum RequestStatus {
    Unknown = 0,

    // For internal use to signify a successful parameter check
    NoError = 10,

    Success = 100,

    // The `requestType` field is missing from the request data
    MissingRequestType = 203,
    // The request type is invalid or does not exist
    UnknownRequestType = 204,
    // Generic error code (comment required)
    GenericError = 205,

    // A required request parameter is missing
    MissingRequestParameter = 300,
    // The request does not have a valid requestData object.
    MissingRequestData = 301,

    // Generic invalid request parameter message (comment required)
    InvalidRequestParameter = 400,
    // A request parameter has the wrong data type
    InvalidRequestParameterType = 401,
    // A request parameter (float or int) is out of valid range
    RequestParameterOutOfRange = 402,
    // A request parameter (string or array) is empty and cannot be
    RequestParameterEmpty = 403,
    // There are too many request parameters (eg. a request takes two optionals, where only one is allowed at a time)
    TooManyRequestParameters = 404,

    // An output is running and cannot be in order to perform the request (generic)
    OutputRunning = 500,
    // An output is not running and should be
    OutputNotRunning = 501,
    // An output is paused and should not be
    OutputPaused = 502,
    // An output is disabled and should not be
    OutputDisabled = 503,
    // Studio mode is active and cannot be
    StudioModeActive = 504,
    // Studio mode is not active and should be
    StudioModeNotActive = 505,

    // The resource was not found
    ResourceNotFound = 600,
    // The resource already exists
    ResourceAlreadyExists = 601,
    // The type of resource found is invalid
    InvalidResourceType = 602,
    // There are not enough instances of the resource in order to perform the request
    NotEnoughResources = 603,
    // The state of the resource is invalid. For example, if the resource is blocked from being accessed
    InvalidResourceState = 604,
    // The specified input (obs_source_t-OBS_SOURCE_TYPE_INPUT) had the wrong kind
    InvalidInputKind = 605,

    // Creating the resource failed
    ResourceCreationFailed = 700,
    // Performing an action on the resource failed
    ResourceActionFailed = 701,
    // Processing the request failed unexpectedly (comment required)
    RequestProcessingFailed = 702,
    // The combination of request parameters cannot be used to perform an action
    CannotAct = 703,
};
```


## Message Types (OpCodes)
The following message types are the low-level message types which may be sent to and from obs-websocket. 

Messages sent from the obs-websocket server or client may contain these first-level keys, known as the base object:
```
{
  "op": number,
  "d": object
}
```
- `op` is a [`WebSocketOpCode` OpCode.](#websocketopcode-enum)
- `d` is an object of the data keys associated with the operation.

---

### Hello (OpCode 0)
- Sent from: obs-websocket
- Sent to: Freshly connected websocket client
- Description: First message sent from the server immediately on client connection. Contains authentication information if auth is required. Also contains RPC version for version negotiation.

**Data Keys:**
```
{
  "obsWebSocketVersion": string,
  "rpcVersion": number,
  "authentication": object(optional)
}
```
- `rpcVersion` is a version number which gets incremented on each **breaking change** to the obs-websocket protocol. Its usage in this context is to provide the current rpc version that the server would like to use.

**Example Messages:**
Authentication is required
```json
{
  "op": 0,
  "d": {
    "obsWebSocketVersion": "5.0.0",
    "rpcVersion": 1,
    "authentication": {
      "challenge": "+IxH4CnCiqpX1rM9scsNynZzbOe4KhDeYcTNS3PDaeY=",
      "salt": "lM1GncleQOaCu9lT1yeUZhFYnqhsLLP1G5lAGo3ixaI="
    }
  }
}
```

Authentication is not required
```json
{
  "op": 0,
  "d": {
    "obsWebSocketVersion": "5.0.0",
    "rpcVersion": 1
  }
}
```

---

### Identify (OpCode 1)
- Sent from: Freshly connected websocket client
- Sent to: obs-websocket
- Description: Response to `Hello` message, should contain authentication string if authentication is required, along with PubSub subscriptions and other session parameters.

**Data Keys:**
```
{
  "rpcVersion": number,
  "authentication": string(optional),
  "ignoreInvalidMessages": bool(optional) = false,
  "ignoreNonFatalRequestChecks": bool(optional) = false,
  "eventSubscriptions": number(optional) = (EventSubscription::All)
}
```
- `rpcVersion` is the version number that the client would like the obs-websocket server to use.
- When `ignoreInvalidMessages` is true, the socket will not be closed for [`WebSocketCloseCode`](#websocketclosecode-enum): `MessageDecodeError`, `UnknownOpCode`, or `MissingDataKey`. Instead, the message will be logged and ignored.
- When `ignoreNonFatalRequestChecks` is true, requests will ignore checks which are not critical to the function of the request. Eg calling `DeleteScene` when the target scene does not exist would still return [`RequestStatus::Success`](#requeststatus-enum) if this flag is enabled.
- `eventSubscriptions` is a bitmask of [`EventSubscriptions`](#eventsubscriptions-enum) items to subscribe to events and event categories at will. By default, all event categories are subscribed, except for events marked as high volume. High volume events must be explicitly subscribed to.

**Example Message:**
```json
{
  "op": 1,
  "d": {
    "rpcVersion": 1,
    "authentication": "Dj6cLS+jrNA0HpCArRg0Z/Fc+YHdt2FQfAvgD1mip6Y=",
    "eventSubscriptions": 33
  }
}
```

---

### Identified (OpCode 2)
- Sent from: obs-websocket
- Sent to: Freshly identified client
- Description: The identify request was received and validated, and the connection is now ready for normal operation.

**Data Keys:**
```
{
  "negotiatedRpcVersion": number
}
```
- If rpc version negotiation succeeds, the server determines the RPC version to be used and gives it to the client as `negotiatedRpcVersion`

**Example Message:**
```json
{
  "op": 2,
  "d": {
    "negotiatedRpcVersion": 1
  }
}
```

---

### Reidentify (OpCode 3)
- Sent from: Identified client
- Sent to: obs-websocket
- Description: Sent at any time after initial identification to update the provided session parameters.

**Data Keys:**
```
{
  "ignoreInvalidMessages": bool(optional) = false,
  "ignoreNonFatalRequestChecks": bool(optional) = false,
  "eventSubscriptions": number(optional) = (EventSubscription::All)
}
```
- Only the listed parameters may be changed after initial identification. To change a parameter not listed, you must reconnect to the obs-websocket server.

---

### Event (OpCode 5)
- Sent from: obs-websocket
- Sent to: All subscribed and identified clients
- Description: An event coming from OBS has occured. Eg scene switched, source muted.

**Data Keys:**
```
{
  "eventType": string,
  "eventIntent": number,
  "eventData": object(optional)
}
```
- `eventIntent` is the original intent required to be subscribed to in order to receive the event.

**Example Message:**
```json
{
  "op": 2,
  "d": {
    "eventType": "StudioModeStateChanged",
    "eventIntent": 1,
    "eventData": {
      "studioModeEnabled": true
    }
  }
}
```

---

### Request (OpCode 6)
- Sent from: Identified client
- Sent to: obs-websocket
- Description: Client is making a request to obs-websocket. Eg get current scene, create source.

**Data Keys:**
```
{
  "requestType": string,
  "requestId": string,
  "requestData": object(optional),
  
}
```

**Example Message:**
```json
{
  "op": 6,
  "d": {
    "requestType": "SetCurrentScene",
    "requestId": "f819dcf0-89cc-11eb-8f0e-382c4ac93b9c",
    "requestData": {
      "sceneName": "Scene 12"
    }
  }
}
```

---

### RequestResponse (OpCode 7)
- Sent from: obs-websocket
- Sent to: Identified client which made the request
- Description: obs-websocket is responding to a request coming from a client.

**Data Keys:**
```
{
  "requestType": string,
  "requestId": string,
  "requestStatus": object,
  "responseData": object(optional)
}
```
- The `requestType` and `requestId` are simply mirrors of what was sent by the client.

`requestStatus` object:
```
{
  "result": bool,
  "code": number,
  "comment": string(optional)
}
```
- `result` is `true` if the request resulted in [`RequestStatus::Success`](#requeststatus-enum). False if otherwise.
- `code` is a [`RequestStatus`](#requeststatus-enum) code.
- `comment` may be provided by the server on errors to offer further details on why a request failed.

**Example Messages:**
Successful Response
```json
{
  "op": 7,
  "d": {
    "requestType": "SetCurrentScene",
    "requestId": "f819dcf0-89cc-11eb-8f0e-382c4ac93b9c",
    "requestStatus": {
      "result": true,
      "code": 100
    }
  }
}
```

Failure Response
```json
{
  "op": 7,
  "d": {
    "requestType": "SetCurrentScene",
    "requestId": "f819dcf0-89cc-11eb-8f0e-382c4ac93b9c",
    "requestStatus": {
      "result": false,
      "code": 608,
      "comment": "Parameter: sceneName"
    }
  }
}
```

---

### RequestBatch (OpCode 8)
- Sent from: Identified client
- Sent to: obs-websocket
- Description: Client is making a batch of requests for obs-websocket. Requests are processed serially (in order) by the server.

**Data Keys:**
```
{
  "requestId": string,
  "haltOnFailure": bool(optional) = false,
  "requests": array<object>
}
```
- When `haltOnFailure` is `true`, the processing of requests will be halted on first failure. Returns only the processed requests in [`RequestBatchResponse`](#requestbatchresponse-opcode-9).
- Requests in the `requests` array follow the same structure as the `Request` payload data format, however `requestId` is an optional key.

---

### RequestBatchResponse (OpCode 9)
- Sent from: obs-websocket
- Sent to: Identified client which made the request
- Description: obs-websocket is responding to a request batch coming from the client.

**Data Keys:**
```
{
  "requestId": string,
  "results": array<object>
}
```




## Requests/Events Table of Contents

<!-- toc -->

- [Events](#events)
- [Requests](#requests)

<!-- tocstop -->

## Events





## Requests


