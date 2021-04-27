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
  - [Status and close codes](#status-and-close-codes)
- [Base message types](#message-types)
  - [Hello](#hello)
  - [Identify](#identify)
  - [Identified](#identified)
  - [Reidentify](#reidentify)
  - [Event](#event)
  - [Request](#request)
  - [RequestResponse](#requestresponse)
  - [RequestBatch](#requestbatch)
  - [RequestBatchResponse](#requestbatchresponse)
- [Requests](#requests)
- [Events](#events)


## Connecting to obs-websocket
Here's info on how to connect to obs-websocket

---

### Connection steps
These steps should be followed precisely. Failure to connect to the server as instructed will likely result in your client being treated in an undefined way.

- Initial HTTP request made to obs-websocket server.
  - HTTP request headers can be used to set the websocket communication type. The default format is JSON. Example headers:
    - `Content-Type: application/json`
    - `Content-Type: application/msgpack` *Not currently planned for v5.0.0*
  - If an invalid `Content-Type` is specified, the connection will be closed.

- Once the connection is upgraded, the websocket server will immediately send a [`Hello`](#hello) message to the client.

- The client listens for the `Hello` and responds with an [`Identify`](#identify) containing all appropriate session parameters.

- The server receives and processes the `Identify`.
  - If authentication is required and the `Identify` does not contain an `authentication` string, or the string is not correct, the connection is dropped with `WebsocketCloseCode::AuthenticationFailed`
  - If the client has requested an `rpcVersion` which the server cannot use, the connection is dropped with `WebsocketCloseCode::UnsupportedProtocolVersion`
  - If any other parameters are malformed (invalid type, etc), the connection is dropped with `WebsocketCloseCode::InvalidIdentifyParameter`

- Once identification is processed on the server, the server responds to the client with an [`Identified`](#identified).

- The client will begin receiving events from obs-websocket and may now make requests to obs-websocket.

- At any time after a client has been identified, it may send a `Reidentify` message to update certain allowed session parameters. The server will respond in the same way it does during initial identification.

#### Connection Notes
- The obs-websocket server listens for any messages containing a `request-type` field in the first level JSON from unidentified clients. If a message matches, the connection is dropped with `WebsocketCloseCode::UnsupportedProtocolVersion`.
- If a message with a `messageType` is not recognized to the obs-websocket server, the connection is dropped with `WebsocketCloseCode::UnknownMessageType`.
- At no point may the client send any message other than a single `Identify` before it has received an `Identified`. Breaking this rule will result in the connection being dropped by the server with `WebsocketCloseCode::NotIdentified`.
- The `Hello` object contains an `rpcVersion` field, which is the latest RPC version that the server supports.
  - If the server's version is is older than the client's, the client is allowed the capability to support older RPC versions. The client determines which RPC version it hopes to communicate on, and sends it via the `rpcVersion` field in the `Identify`.
  - If the server's version is newer than the client's, the client sends its highest supported version in its `Identify` in hopes that the server is backwards compatible to that version.
- If the `Hello` does not contain an `authentication` object, the resulting `Identify` object sent to the server does not need to have an `authentication` string.

---

### Creating an authentication string
obs-websocket uses SHA256 to transmit authentication credentials. The server starts by sending an object in the `authentication` field of its `Hello`. The client processes the authentication challenge and responds via the `authentication` string in `Identify`.

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
- Generate an SHA256 binary hash of the result and encode it with base64, known as a base64 secret.
- Concatenate the base64 secret with the `challenge` sent by the server (`base64_secret + challenge`)
- Generate a binary SHA256 hash of that result and encode it to base64. You now have your `authentication` string.

For more info on how to create the `authentication` string, refer to the obs-websocket client libraries listed on the [README](README.md).

---

### Status and Close Codes
These are the enumeration definitions for various codes used by obs-websocket.

#### RequestStatus Enum
```cpp
enum RequestStatus: std::uint16_t {
	Unknown = 0,

	// For internal use to signify a successful parameter check
	NoError = 10,

	Success = 100,

	// The request is denied because the client is not authenticated
	AuthenticationMissing = 200,
	// Connection has already been authenticated (for modules utilizing a request to provide authentication)
	AlreadyAuthenticated = 201,
	// Authentication request was denied (for modules utilizing a request to provide authentication)
	AuthenticationDenied = 202,
	// The `requestType` field is missing from the request data
	RequestTypeMissing = 203,
	// The request type is invalid (does not exist)
	InvalidRequestType = 204,
	// Generic error code (comment is expected to be provided)
	GenericError = 205,

	// A required request parameter is missing
	MissingRequestParameter = 300,

	// Generic invalid request parameter message
	InvalidRequestParameter = 400,
	// A request parameter has the wrong data type
	InvalidRequestParameterDataType = 401,
	// A request parameter (float or int) is out of valid range
	RequestParameterOutOfRange = 402,
	// A request parameter (string or array) is empty and cannot be
	RequestParameterEmpty = 403,

	// An output is running and cannot be in order to perform the request (generic)
	OutputRunning = 500,
	// An output is not running and should be
	OutputNotRunning = 501,
	// Stream is running and cannot be
	StreamRunning = 502,
	// Stream is not running and should be
	StreamNotRunning = 503,
	// Record is running and cannot be
	RecordRunning = 504,
	// Record is not running and should be
	RecordNotRunning = 505,
	// Record is paused and cannot be
	RecordPaused = 506,
	// Replay buffer is running and cannot be
	ReplayBufferRunning = 507,
	// Replay buffer is not running and should be
	ReplayBufferNotRunning = 508,
	// Replay buffer is disabled and cannot be
	ReplayBufferDisabled = 509,
	// Studio mode is active and cannot be
	StudioModeActive = 510,
	// Studio mode is not active and should be
	StudioModeNotActive = 511,

	// The specified source (obs_source_t) was of the invalid type (Eg. input instead of scene)
	InvalidSourceType = 600,
	// The specified source (obs_source_t) was not found (generic for input, filter, transition, scene)
	SourceNotFound = 601,
	// The specified source (obs_source_t) already exists. Applicable to inputs, filters, transitions, scenes
	SourceAlreadyExists = 602,
	// The specified input (obs_source_t-OBS_SOURCE_TYPE_FILTER) was not found
	InputNotFound = 603,
	// The specified input (obs_source_t-OBS_SOURCE_TYPE_INPUT) had the wrong kind
	InvalidInputKind = 604,
	// The specified filter (obs_source_t-OBS_SOURCE_TYPE_FILTER) was not found
	FilterNotFound = 605,
	// The specified transition (obs_source_t-OBS_SOURCE_TYPE_TRANSITION) was not found
	TransitionNotFound = 606,
	// The specified transition (obs_source_t-OBS_SOURCE_TYPE_TRANSITION) does not support setting its position (transition is of fixed type)
	TransitionDurationFixed = 607,
	// The specified scene (obs_source_t-OBS_SOURCE_TYPE_SCENE), (obs_scene_t) was not found
	SceneNotFound = 608,
	// The specified scene item (obs_sceneitem_t) was not found
	SceneItemNotFound = 609,
	// The specified scene collection was not found
	SceneCollectionNotFound = 610,
	// The specified profile was not found
	ProfileNotFound = 611,
	// The specified output (obs_output_t) was not found
	OutputNotFound = 612,
	// The specified encoder (obs_encoder_t) was not found
	EncoderNotFound = 613,
	// The specified service (obs_service_t) was not found
	ServiceNotFound = 614,
	// The specified hotkey was not found
	HotkeyNotFound = 615,
	// The specified directory was not found
	DirectoryNotFound = 616,
	// The specified config item (obs_config_t) was not found. Could be section or parameter name.
	ConfigParameterNotFound = 617,
	// The specified property (obs_properties_t) was not found
	PropertyNotFound = 618

	// Processing the request failed unexpectedly
	RequestProcessingFailed = 700,
	// Starting the Output failed
	OutputStartFailed = 701,
	// Duplicating the scene item failed
	SceneItemDuplicationFailed = 702,
	// Rendering the screenshot failed
	ScreenshotRenderFailed = 703,
	// Encoding the screenshot failed
	ScreenshotEncodeFailed = 704,
	// Saving the screenshot failed
	ScreenshotSaveFailed = 705,
	// Creating the directory failed
	DirectoryCreationFailed = 706,
};
```

#### WebsocketCloseCode Enum
```cpp
enum WebsocketCloseCode: std::uint16_t {
	UnknownReason = 4000,

	// The server was unable to decode the incoming websocket message
	MessageDecodeError = 4001,
	// The specified `messageType` was invalid
	UnknownMessageType = 4002,
	// The client sent a websocket message without first sending `Identify` message
	NotIdentified = 4003,
	// The client sent an `Identify` message while already identified
	AlreadyIdentified = 4004,
	// The authentication attempt (via `Identify`) failed
	AuthenticationFailed = 4005,
	// There was an invalid parameter the client's `Identify` message
	InvalidIdentifyParameter = 4006,
	// A `Request` or `RequestBatch` was missing its `requestId`
	RequestMissingRequestId = 4007,
	// The websocket session has been invalidated by the obs-websocket server.
	SessionInvalidated = 4008,
	// The server detected the usage of an old version of the obs-websocket protocol.
	UnsupportedProtocolVersion = 4009,
};
```


## Message Types
The following message types are the base message types which may be sent to and from obs-websocket. 

**Every** message sent from the obs-websocket server or client must contain these fields, known as the base object:
```
{
  "messageType": string
}
```

---

### Hello
- Sent from: obs-websocket
- Sent to: Freshly connected websocket client
- Description: First message sent from the server immediately on client connection. Contains authentication information if auth is required. Also contains RPC version for version negotiation.

**Additional Base Object Fields:**
```
{
  "obsWebsocketVersion": string,
  "rpcVersion": number,
  "availableEvents": array<string>,
  "availableRequests": array<string>,
  "authentication": object(optional)
}
```
- `rpcVersion` is a version number which gets incremented on each **breaking change** to the obs-websocket protocol. Its usage in this context is to provide the current rpc version that the server would like to use.

**Example Messages:**
Authentication is required
```json
{
  "messageType": "Hello",
  "websocketVersion": "5.0.0",
  "rpcVersion": 3,
  "availableRequests": ["GetVersion"],
  "availableEvents": ["Exiting"],
  "authentication": {
    "challenge": "+IxH4CnCiqpX1rM9scsNynZzbOe4KhDeYcTNS3PDaeY=",
    "salt": "lM1GncleQOaCu9lT1yeUZhFYnqhsLLP1G5lAGo3ixaI="
  }
}
```

Authentication is not required
```json
{
  "messageType": "Hello",
  "websocketVersion": "5.0.0",
  "rpcVersion": 3,
  "availableRequests": ["GetVersion"],
  "availableEvents": ["Exiting"]
}
```

---

### Identify
- Sent from: Freshly connected websocket client
- Sent to: obs-websocket
- Description: Response to `Hello` message, should contain authentication string if authentication is required, along with PubSub subscriptions and other session parameters.

**Additional Base Object Fields:**
```
{
  "rpcVersion": number,
  "authentication": string(optional),
  "ignoreInvalidMessages": bool(optional) = false,
  "ignoreNonFatalRequestChecks": bool(optional) = false,
  "eventWhitelist": array<string>(optional),
  "eventBlacklist": array<string>(optional)
}
```
- `rpcVersion` is the version number that the client would like the obs-websocket server to use.
- When `ignoreInvalidMessages` is true, the socket will not be closed for `WebsocketCloseCode` `MessageDecodeError`, `UnknownMessageType`, or `RequestMissingRequestId`. Instead, the message will be logged and dropped.
- When `ignoreNonFatalRequestChecks` is true, requests will ignore checks which are not critical to the function of the request. Eg calling `DeleteScene` when the target scene does not exist would still return `RequestStatus::Success` if this flag is enabled.
- When `eventWhitelist` is specified, only the specified events will be sent to the client.
- When `eventBlacklist` is specified, all events except those specified will be sent to the client.
- Only `eventWhitelist` or `eventBlacklist` may be provided. If both are provided, the connection will be closed with `WebsocketCloseCode::InvalidIdentifyParameter`.

**Example Message:**
```json
{
  "messageType": "Identify",
  "rpcVersion": 3,
  "authentication": "Dj6cLS+jrNA0HpCArRg0Z/Fc+YHdt2FQfAvgD1mip6Y=",
  "eventWhitelist": ["Exiting"]
}
```

---

### Identified
- Sent from: obs-websocket
- Sent to: Freshly identified client
- Description: The identify request was received and validated, and the connection is now ready for normal operation.

**Additional Base Object Fields:**
```
{
  "negotiatedRpcVersion": number
}
```
- If rpc version negotiation succeeds, the server determines the RPC version to be used and gives it to the client as `negotiatedRpcVersion`

**Example Message:**
```json
{
  "messageType": "Identified",
  "negotiatedRpcVersion": 3
}
```

---

### Reidentify
- Sent from: Identified client
- Sent to: obs-websocket
- Description: Sent at any time after initial identification to update the provided session parameters.

**Additional Base Object Fields:**
```
{
  "ignoreInvalidMessages": bool(optional),
  "ignoreNonFatalRequestChecks": bool(optional),
  "eventWhitelist": array<string>(optional),
  "eventBlacklist": array<string>(optional)
}
```
- Only the listed parameters may be changed by `Reidentify` after initial identification. To change a parameter not listed, you must reconnect to the obs-websocket server.

---

### Event
- Sent from: obs-websocket
- Sent to: All subscribed and identified clients
- Description: An event coming from OBS has occured. Eg scene switched, source muted.

**Additional Base Object Fields:**
```
{
  "eventType": string,
  "eventData": object(optional)
}
```

---

### Request
- Sent from: Identified client
- Sent to: obs-websocket
- Description: Client is making a request to obs-websocket. Eg get current scene, create source.

**Additional Base Object Fields:**
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
  "messageType": "Request",
  "requestType": "SetCurrentScene",
  "requestId": "f819dcf0-89cc-11eb-8f0e-382c4ac93b9c",
  "requestData": {
    "sceneName": "Scene 12"
  }
}
```

---

### RequestResponse
- Sent from: obs-websocket
- Sent to: Identified client which made the request
- Description: obs-websocket is responding to a request coming from a client.

**Additional Base Object Fields:**
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
- `result` is `true` if the request resulted in `RequestStatus::Success`. False if otherwise.
- `code` is a [`RequestStatus`](#requeststatus-enum) code.
- `comment` may be provided by the server on errors to offer further details on why a request failed.

**Example Messages:**
Successful Response
```json
{
  "messageType": "RequestResponse",
  "requestType": "SetCurrentScene",
  "requestId": "f819dcf0-89cc-11eb-8f0e-382c4ac93b9c",
  "requestStatus": {
    "result": true,
    "code": 100
  }
}
```

Failure Response
```json
{
  "messageType": "RequestResponse",
  "requestType": "SetCurrentScene",
  "requestId": "f819dcf0-89cc-11eb-8f0e-382c4ac93b9c",
  "requestStatus": {
    "result": false,
    "code": 608,
    "comment": "Parameter: sceneName"
  }
}
```

---

### RequestBatch
- Sent from: Identified client
- Sent to: obs-websocket
- Description: Client is making a batch of requests for obs-websocket. Requests are processed serially (in order) by the server.

**Additional Base Object Fields:**
```
{
  "requestId": string,
  "haltOnFailure": bool(optional) = false,
  "requests": array<object>
}
```
- When `haltOnFailure` is `true`, the processing of requests will be halted on first failure. Returns only the processed requests in [`RequestBatchResponse`](#requestbatchresponse).

---

### RequestBatchResponse
- Sent from: obs-websocket
- Sent to: Identified client which made the request
- Description: obs-websocket is responding to a request batch coming from the client.

**Additional Base Object Fields:**
```
{
  "requestId": string,
  "results": array<object>
}
```




# Table of Contents

<!-- toc -->

- [Events](#events)
- [Requests](#requests)

<!-- tocstop -->

## Events





## Requests


