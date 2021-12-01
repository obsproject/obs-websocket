<!-- This file was automatically generated. Do not edit directly! -->

# Main Table of Contents
- [obs-websocket 5.0.0 Protocol](#obs-websocket-500-protocol)
  - [Connecting to obs-websocket](#connecting-to-obs-websocket)
    - [Connection steps](#connection-steps)
    - [Creating an authentication string](#creating-an-authentication-string)
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
- [Enums](#enums)
- [Events](#events)
- [Requests](#requests)

# obs-websocket 5.0.0 Protocol

## General Intro
obs-websocket provides a feature-rich RPC communication protocol, giving access to much of OBS's feature set. This document contains everything you should know in order to make a connection and use obs-websocket's functionality to the fullest.

### Design Goals
- Abstraction of identification, events, requests, and batch requests into dedicated message types
- Conformity of request naming using similar terms like `Get`, `Set`, `Get[x]List`, `Start[x]`, `Toggle[x]`
- Conformity of OBS data field names like `sourceName`, `sourceKind`, `sourceType`, `sceneName`, `sceneItemName`
- Error code response system - integer corrosponds to type of error, with optional comment
- Possible support for multiple message encoding options: JSON and MessagePack
- PubSub system - Allow clients to specify which events they do or don't want to receive from OBS
- RPC versioning - Client and server negotiate the latest version of the obs-websocket protocol to communicate with.


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
  - If there is an `authentication` field in the `messageData` object, the server requires authentication, and the steps in [Creating an authentication string](#creating-an-authentication-string) should be followed.
  - If there is no `authentication` field, the resulting `Identify` object sent to the server does not require an `authentication` string.
  - The client determines if the server's `rpcVersion` is supported, and if not it provides its closest supported version in `Identify`.

- The server receives and processes the `Identify` sent by the client.
  - If authentication is required and the `Identify` message data does not contain an `authentication` string, or the string is not correct, the connection is closed with `WebSocketCloseCode::AuthenticationFailed`
  - If the client has requested an `rpcVersion` which the server cannot use, the connection is closed with `WebSocketCloseCode::UnsupportedRpcVersion`. This system allows both the server and client to have seamless backwards compatability.
  - If any other parameters are malformed (invalid type, etc), the connection is closed with an appropriate close code.

- Once identification is processed on the server, the server responds to the client with an [OpCode 2 `Identified`](#identified-opcode-2).

- The client will begin receiving events from obs-websocket and may now make requests to obs-websocket.

- At any time after a client has been identified, it may send an [OpCode 3 `Reidentify`](#reidentify-opcode-3) message to update certain allowed session parameters. The server will respond in the same way it does during initial identification.

#### Connection Notes
- If a binary frame is received when using the `obswebsocket.json` (default) subprotocol, or a text frame is received while using the `obswebsocket.msgpack` subprotocol, the connection is closed with `WebSocketCloseCode::MessageDecodeError`.
- The obs-websocket server listens for any messages containing a `request-type` field in the first level JSON from unidentified clients. If a message matches, the connection is closed with `WebSocketCloseCode::UnsupportedRpcVersion` and a warning is logged.
- If a message with a `messageType` is not recognized to the obs-websocket server, the connection is closed with `WebSocketCloseCode::UnknownOpCode`.
- At no point may the client send any message other than a single `Identify` before it has received an `Identified`. Doing so will result in the connection being closed with `WebSocketCloseCode::NotIdentified`.

---

### Creating an authentication string
obs-websocket uses SHA256 to transmit authentication credentials. The server starts by sending an object in the `authentication` field of its `Hello` message data. The client processes the authentication challenge and responds via the `authentication` string in the `Identify` message data.

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


## Message Types (OpCodes)
The following message types are the low-level message types which may be sent to and from obs-websocket. 

Messages sent from the obs-websocket server or client may contain these first-level fields, known as the base object:
```
{
  "op": number,
  "d": object
}
```
- `op` is a `WebSocketOpCode` OpCode.
- `d` is an object of the data fields associated with the operation.

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
  "eventSubscriptions": number(optional) = (EventSubscription::All)
}
```
- `rpcVersion` is the version number that the client would like the obs-websocket server to use.
- When `ignoreInvalidMessages` is true, the socket will not be closed for `WebSocketCloseCode`: `MessageDecodeError`, `UnknownOpCode`, or `MissingDataKey`. Instead, the message will be logged and ignored.
- `eventSubscriptions` is a bitmask of `EventSubscriptions` items to subscribe to events and event categories at will. By default, all event categories are subscribed, except for events marked as high volume. High volume events must be explicitly subscribed to.

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
- `result` is `true` if the request resulted in `RequestStatus::Success`. False if otherwise.
- `code` is a `RequestStatus` code.
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
  "executionType": number(optional) = RequestBatchExecutionType::SerialRealtime
  "requests": array<object>
}
```
- When `haltOnFailure` is `true`, the processing of requests will be halted on first failure. Returns only the processed requests in [`RequestBatchResponse`](#requestbatchresponse-opcode-9).
- Requests in the `requests` array follow the same structure as the `Request` payload data format, however `requestId` is an optional field.

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


# Enums
These are enumeration declarations, which are referenced throughout obs-websocket's protocol.

### Enumerations Table of Contents
- [WebSocketOpCode](#websocketopcode)
  - [WebSocketOpCode::Hello](#websocketopcodehello)
  - [WebSocketOpCode::Identify](#websocketopcodeidentify)
  - [WebSocketOpCode::Identified](#websocketopcodeidentified)
  - [WebSocketOpCode::Reidentify](#websocketopcodereidentify)
  - [WebSocketOpCode::Event](#websocketopcodeevent)
  - [WebSocketOpCode::Request](#websocketopcoderequest)
  - [WebSocketOpCode::RequestResponse](#websocketopcoderequestresponse)
  - [WebSocketOpCode::RequestBatch](#websocketopcoderequestbatch)
  - [WebSocketOpCode::RequestBatchResponse](#websocketopcoderequestbatchresponse)
- [WebSocketCloseCode](#websocketclosecode)
  - [WebSocketCloseCode::DontClose](#websocketclosecodedontclose)
  - [WebSocketCloseCode::UnknownReason](#websocketclosecodeunknownreason)
  - [WebSocketCloseCode::MessageDecodeError](#websocketclosecodemessagedecodeerror)
  - [WebSocketCloseCode::MissingDataField](#websocketclosecodemissingdatafield)
  - [WebSocketCloseCode::InvalidDataFieldType](#websocketclosecodeinvaliddatafieldtype)
  - [WebSocketCloseCode::InvalidDataFieldValue](#websocketclosecodeinvaliddatafieldvalue)
  - [WebSocketCloseCode::UnknownOpCode](#websocketclosecodeunknownopcode)
  - [WebSocketCloseCode::NotIdentified](#websocketclosecodenotidentified)
  - [WebSocketCloseCode::AlreadyIdentified](#websocketclosecodealreadyidentified)
  - [WebSocketCloseCode::AuthenticationFailed](#websocketclosecodeauthenticationfailed)
  - [WebSocketCloseCode::UnsupportedRpcVersion](#websocketclosecodeunsupportedrpcversion)
  - [WebSocketCloseCode::SessionInvalidated](#websocketclosecodesessioninvalidated)
  - [WebSocketCloseCode::UnsupportedFeature](#websocketclosecodeunsupportedfeature)
- [RequestBatchExecutionType](#requestbatchexecutiontype)
  - [RequestBatchExecutionType::None](#requestbatchexecutiontypenone)
  - [RequestBatchExecutionType::SerialRealtime](#requestbatchexecutiontypeserialrealtime)
  - [RequestBatchExecutionType::SerialFrame](#requestbatchexecutiontypeserialframe)
  - [RequestBatchExecutionType::Parallel](#requestbatchexecutiontypeparallel)
- [RequestStatus](#requeststatus)
  - [RequestStatus::Unknown](#requeststatusunknown)
  - [RequestStatus::NoError](#requeststatusnoerror)
  - [RequestStatus::Success](#requeststatussuccess)
  - [RequestStatus::MissingRequestType](#requeststatusmissingrequesttype)
  - [RequestStatus::UnknownRequestType](#requeststatusunknownrequesttype)
  - [RequestStatus::GenericError](#requeststatusgenericerror)
  - [RequestStatus::UnsupportedRequestBatchExecutionType](#requeststatusunsupportedrequestbatchexecutiontype)
  - [RequestStatus::MissingRequestField](#requeststatusmissingrequestfield)
  - [RequestStatus::MissingRequestData](#requeststatusmissingrequestdata)
  - [RequestStatus::InvalidRequestField](#requeststatusinvalidrequestfield)
  - [RequestStatus::InvalidRequestFieldType](#requeststatusinvalidrequestfieldtype)
  - [RequestStatus::RequestFieldOutOfRange](#requeststatusrequestfieldoutofrange)
  - [RequestStatus::RequestFieldEmpty](#requeststatusrequestfieldempty)
  - [RequestStatus::TooManyRequestFields](#requeststatustoomanyrequestfields)
  - [RequestStatus::OutputRunning](#requeststatusoutputrunning)
  - [RequestStatus::OutputNotRunning](#requeststatusoutputnotrunning)
  - [RequestStatus::OutputPaused](#requeststatusoutputpaused)
  - [RequestStatus::OutputNotPaused](#requeststatusoutputnotpaused)
  - [RequestStatus::OutputDisabled](#requeststatusoutputdisabled)
  - [RequestStatus::StudioModeActive](#requeststatusstudiomodeactive)
  - [RequestStatus::StudioModeNotActive](#requeststatusstudiomodenotactive)
  - [RequestStatus::ResourceNotFound](#requeststatusresourcenotfound)
  - [RequestStatus::ResourceAlreadyExists](#requeststatusresourcealreadyexists)
  - [RequestStatus::InvalidResourceType](#requeststatusinvalidresourcetype)
  - [RequestStatus::NotEnoughResources](#requeststatusnotenoughresources)
  - [RequestStatus::InvalidResourceState](#requeststatusinvalidresourcestate)
  - [RequestStatus::InvalidInputKind](#requeststatusinvalidinputkind)
  - [RequestStatus::ResourceCreationFailed](#requeststatusresourcecreationfailed)
  - [RequestStatus::ResourceActionFailed](#requeststatusresourceactionfailed)
  - [RequestStatus::RequestProcessingFailed](#requeststatusrequestprocessingfailed)
  - [RequestStatus::CannotAct](#requeststatuscannotact)
- [EventSubscription](#eventsubscription)
  - [EventSubscription::None](#eventsubscriptionnone)
  - [EventSubscription::General](#eventsubscriptiongeneral)
  - [EventSubscription::Config](#eventsubscriptionconfig)
  - [EventSubscription::Scenes](#eventsubscriptionscenes)
  - [EventSubscription::Inputs](#eventsubscriptioninputs)
  - [EventSubscription::Transitions](#eventsubscriptiontransitions)
  - [EventSubscription::Filters](#eventsubscriptionfilters)
  - [EventSubscription::Outputs](#eventsubscriptionoutputs)
  - [EventSubscription::SceneItems](#eventsubscriptionsceneitems)
  - [EventSubscription::MediaInputs](#eventsubscriptionmediainputs)
  - [EventSubscription::ExternalPlugins](#eventsubscriptionexternalplugins)
  - [EventSubscription::All](#eventsubscriptionall)
  - [EventSubscription::InputVolumeMeters](#eventsubscriptioninputvolumemeters)
  - [EventSubscription::InputActiveStateChanged](#eventsubscriptioninputactivestatechanged)
  - [EventSubscription::InputShowStateChanged](#eventsubscriptioninputshowstatechanged)
  - [EventSubscription::SceneItemTransformChanged](#eventsubscriptionsceneitemtransformchanged)


## WebSocketOpCode

### WebSocketOpCode::Hello

The initial message sent by obs-websocket to newly connected clients.

- Identifier Value: `0`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketOpCode::Identify

The message sent by a newly connected client to obs-websocket in response to a `Hello`.

- Identifier Value: `1`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketOpCode::Identified

The response sent by obs-websocket to a client after it has successfully identified with obs-websocket.

- Identifier Value: `2`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketOpCode::Reidentify

The message sent by an already-identified client to update identification parameters.

- Identifier Value: `3`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketOpCode::Event

The message sent by obs-websocket containing an event payload.

- Identifier Value: `5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketOpCode::Request

The message sent by a client to obs-websocket to perform a request.

- Identifier Value: `6`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketOpCode::RequestResponse

The message sent by obs-websocket in response to a particular request from a client.

- Identifier Value: `7`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketOpCode::RequestBatch

The message sent by a client to obs-websocket to perform a batch of requests.

- Identifier Value: `8`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketOpCode::RequestBatchResponse

The message sent by obs-websocket in response to a particular batch of requests from a client.

- Identifier Value: `9`
- Latest Supported RPC Version: `1`
- Added in v5.0.0
## WebSocketCloseCode

### WebSocketCloseCode::DontClose

For internal use only to tell the request handler not to perform any close action.

- Identifier Value: `0`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketCloseCode::UnknownReason

Unknown reason, should never be used.

- Identifier Value: `4000`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketCloseCode::MessageDecodeError

The server was unable to decode the incoming websocket message.

- Identifier Value: `4002`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketCloseCode::MissingDataField

A data field is required but missing from the payload.

- Identifier Value: `4003`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketCloseCode::InvalidDataFieldType

A data field's value type is invalid.

- Identifier Value: `4004`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketCloseCode::InvalidDataFieldValue

A data field's value is invalid.

- Identifier Value: `4005`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketCloseCode::UnknownOpCode

The specified `op` was invalid or missing.

- Identifier Value: `4006`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketCloseCode::NotIdentified

The client sent a websocket message without first sending `Identify` message.

- Identifier Value: `4007`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketCloseCode::AlreadyIdentified

The client sent an `Identify` message while already identified.

Note: Once a client has identified, only `Reidentify` may be used to change session parameters.

- Identifier Value: `4008`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketCloseCode::AuthenticationFailed

The authentication attempt (via `Identify`) failed.

- Identifier Value: `4009`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketCloseCode::UnsupportedRpcVersion

The server detected the usage of an old version of the obs-websocket RPC protocol.

- Identifier Value: `4010`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketCloseCode::SessionInvalidated

The websocket session has been invalidated by the obs-websocket server.

Note: This is the code used by the `Kick` button in the UI Session List. If you receive this code, you must not automatically reconnect.

- Identifier Value: `4011`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### WebSocketCloseCode::UnsupportedFeature

A requested feature is not supported due to hardware/software limitations.

- Identifier Value: `4012`
- Latest Supported RPC Version: `1`
- Added in v5.0.0
## RequestBatchExecutionType

### RequestBatchExecutionType::None

Not a request batch.

- Identifier Value: `0`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestBatchExecutionType::SerialRealtime

A request batch which processes all requests serially, as fast as possible.

Note: To introduce artificial delay, use the `Sleep` request and the `sleepMillis` request field.

- Identifier Value: `1`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestBatchExecutionType::SerialFrame

A request batch type which processes all requests serially, in sync with the graphics thread. Designed to provide high accuracy for animations.

Note: To introduce artificial delay, use the `Sleep` request and the `sleepFrames` request field.

- Identifier Value: `2`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestBatchExecutionType::Parallel

A request batch type which processes all requests using all available threads in the thread pool.

Note: This is mainly experimental, and only really shows its colors during requests which require lots of
active processing, like `GetSourceScreenshot`.

- Identifier Value: `3`
- Latest Supported RPC Version: `1`
- Added in v5.0.0
## RequestStatus

### RequestStatus::Unknown

Unknown status, should never be used.

- Identifier Value: `0`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::NoError

For internal use to signify a successful field check.

- Identifier Value: `10`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::Success

The request has succeeded.

- Identifier Value: `100`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::MissingRequestType

The `requestType` field is missing from the request data.

- Identifier Value: `203`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::UnknownRequestType

The request type is invalid or does not exist.

- Identifier Value: `204`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::GenericError

Generic error code.

Note: A comment is required to be provided by obs-websocket.

- Identifier Value: `205`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::UnsupportedRequestBatchExecutionType

The request batch execution type is not supported.

- Identifier Value: `206`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::MissingRequestField

A required request field is missing.

- Identifier Value: `300`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::MissingRequestData

The request does not have a valid requestData object.

- Identifier Value: `301`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::InvalidRequestField

Generic invalid request field message.

Note: A comment is required to be provided by obs-websocket.

- Identifier Value: `400`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::InvalidRequestFieldType

A request field has the wrong data type.

- Identifier Value: `401`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::RequestFieldOutOfRange

A request field (number) is outside of the allowed range.

- Identifier Value: `402`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::RequestFieldEmpty

A request field (string or array) is empty and cannot be.

- Identifier Value: `403`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::TooManyRequestFields

There are too many request fields (eg. a request takes two optionals, where only one is allowed at a time).

- Identifier Value: `404`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::OutputRunning

An output is running and cannot be in order to perform the request.

- Identifier Value: `500`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::OutputNotRunning

An output is not running and should be.

- Identifier Value: `501`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::OutputPaused

An output is paused and should not be.

- Identifier Value: `502`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::OutputNotPaused

An output is not paused and should be.

- Identifier Value: `503`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::OutputDisabled

An output is disabled and should not be.

- Identifier Value: `504`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::StudioModeActive

Studio mode is active and cannot be.

- Identifier Value: `505`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::StudioModeNotActive

Studio mode is not active and should be.

- Identifier Value: `506`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::ResourceNotFound

The resource was not found.

Note: Resources are any kind of object in obs-websocket, like inputs, profiles, outputs, etc.

- Identifier Value: `600`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::ResourceAlreadyExists

The resource already exists.

- Identifier Value: `601`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::InvalidResourceType

The type of resource found is invalid.

- Identifier Value: `602`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::NotEnoughResources

There are not enough instances of the resource in order to perform the request.

- Identifier Value: `603`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::InvalidResourceState

The state of the resource is invalid. For example, if the resource is blocked from being accessed.

- Identifier Value: `604`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::InvalidInputKind

The specified input (obs_source_t-OBS_SOURCE_TYPE_INPUT) had the wrong kind.

- Identifier Value: `605`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::ResourceCreationFailed

Creating the resource failed.

- Identifier Value: `700`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::ResourceActionFailed

Performing an action on the resource failed.

- Identifier Value: `701`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::RequestProcessingFailed

Processing the request failed unexpectedly.

Note: A comment is required to be provided by obs-websocket.

- Identifier Value: `702`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### RequestStatus::CannotAct

The combination of request fields cannot be used to perform an action.

- Identifier Value: `703`
- Latest Supported RPC Version: `1`
- Added in v5.0.0
## EventSubscription

### EventSubscription::None

Subcription value used to disable all events.

- Identifier Value: `0`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::General

Subscription value to receive events in the `General` category.

- Identifier Value: `(1 << 0)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::Config

Subscription value to receive events in the `Config` category.

- Identifier Value: `(1 << 1)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::Scenes

Subscription value to receive events in the `Scenes` category.

- Identifier Value: `(1 << 2)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::Inputs

Subscription value to receive events in the `Inputs` category.

- Identifier Value: `(1 << 3)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::Transitions

Subscription value to receive events in the `Transitions` category.

- Identifier Value: `(1 << 4)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::Filters

Subscription value to receive events in the `Filters` category.

- Identifier Value: `(1 << 5)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::Outputs

Subscription value to receive events in the `Outputs` category.

- Identifier Value: `(1 << 6)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::SceneItems

Subscription value to receive events in the `SceneItems` category.

- Identifier Value: `(1 << 7)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::MediaInputs

Subscription value to receive events in the `MediaInputs` category.

- Identifier Value: `(1 << 8)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::ExternalPlugins

Subscription value to receive the `ExternalPluginEvent` event.

- Identifier Value: `(1 << 9)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::All

Helper to receive all non-high-volume events.

- Identifier Value: `(General | Config | Scenes | Inputs | Transitions | Filters | Outputs | SceneItems | MediaInputs | ExternalPlugins)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::InputVolumeMeters

Subscription value to receive the `InputVolumeMeters` high-volume event.

- Identifier Value: `(1 << 16)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::InputActiveStateChanged

Subscription value to receive the `InputActiveStateChanged` high-volume event.

- Identifier Value: `(1 << 17)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::InputShowStateChanged

Subscription value to receive the `InputShowStateChanged` high-volume event.

- Identifier Value: `(1 << 18)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### EventSubscription::SceneItemTransformChanged

Subscription value to receive the `SceneItemTransformChanged` high-volume event.

- Identifier Value: `(1 << 19)`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


# Events

### Events Table of Contents
- [General](#general)
  - [ExitStarted](#exitstarted)
  - [StudioModeStateChanged](#studiomodestatechanged)
- [Config](#config)
  - [CurrentSceneCollectionChanging](#currentscenecollectionchanging)
  - [CurrentSceneCollectionChanged](#currentscenecollectionchanged)
  - [SceneCollectionListChanged](#scenecollectionlistchanged)
  - [CurrentProfileChanging](#currentprofilechanging)
  - [CurrentProfileChanged](#currentprofilechanged)
  - [ProfileListChanged](#profilelistchanged)


## General

### ExitStarted

OBS has begun the shutdown process.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0

---

### StudioModeStateChanged

Studio mode has been enabled or disabled.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Data Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| studioModeEnabled | Boolean | True == Enabled, False == Disabled |
## Config

### CurrentSceneCollectionChanging

The current scene collection has begun changing.

Note: We recommend using this event to trigger a pause of all polling requests, as performing any requests during a
scene collection change is considered undefined behavior and can cause crashes!

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Data Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| sceneCollectionName | String | Name of the current scene collection |

---

### CurrentSceneCollectionChanged

The current scene collection has changed.

Note: If polling has been paused during `CurrentSceneCollectionChanging`, this is the que to restart polling.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Data Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| sceneCollectionName | String | Name of the new scene collection |

---

### SceneCollectionListChanged

The scene collection list has changed.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Data Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| sceneCollections | Array<String> | Updated list of scene collections |

---

### CurrentProfileChanging

The current profile has begun changing.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Data Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| profileName | String | Name of the current profile |

---

### CurrentProfileChanged

The current profile has changed.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Data Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| profileName | String | Name of the new profile |

---

### ProfileListChanged

The profile list has changed.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Data Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| profiles | Array<String> | Updated list of profiles |


# Requests

### Requests Table of Contents
- [General](#general-1)
  - [GetVersion](#getversion)
  - [BroadcastCustomEvent](#broadcastcustomevent)
  - [GetStats](#getstats)
  - [GetHotkeyList](#gethotkeylist)
  - [TriggerHotkeyByName](#triggerhotkeybyname)
  - [TriggerHotkeyByKeySequence](#triggerhotkeybykeysequence)
  - [GetStudioModeEnabled](#getstudiomodeenabled)
  - [SetStudioModeEnabled](#setstudiomodeenabled)
  - [Sleep](#sleep)
- [Config](#config-1)
  - [GetPersistentData](#getpersistentdata)
  - [SetPersistentData](#setpersistentdata)
  - [GetSceneCollectionList](#getscenecollectionlist)
  - [SetCurrentSceneCollection](#setcurrentscenecollection)
  - [CreateSceneCollection](#createscenecollection)
  - [GetProfileList](#getprofilelist)
  - [SetCurrentProfile](#setcurrentprofile)
  - [CreateProfile](#createprofile)
  - [RemoveProfile](#removeprofile)
  - [GetProfileParameter](#getprofileparameter)
  - [SetProfileParameter](#setprofileparameter)
  - [GetVideoSettings](#getvideosettings)
  - [SetVideoSettings](#setvideosettings)
  - [GetStreamServiceSettings](#getstreamservicesettings)
  - [SetStreamServiceSettings](#setstreamservicesettings)




## General

### GetVersion

Gets data about the current plugin and RPC version.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Response Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| obsVersion | String | Current OBS Studio version |
| obsWebSocketVersion | String | Current obs-websocket version |
| rpcVersion | Number | Current latest obs-websocket RPC version |
| availableRequests | Array<String> | Array of available RPC requests for the currently negotiated RPC version |

---

### BroadcastCustomEvent

Broadcasts a `CustomEvent` to all WebSocket clients. Receivers are clients which are identified and subscribed.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| eventData | Object | Data payload to emit to all receivers | None | No | None |

---

### GetStats

Gets statistics about OBS, obs-websocket, and the current session.

- Complexity Rating: `2/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Response Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| cpuUsage | Number | Current CPU usage in percent |
| memoryUsage | Number | Amount of memory in MB currently being used by OBS |
| availableDiskSpace | Number | Available disk space on the device being used for recording storage |
| activeFps | Number | Current FPS being rendered |
| averageFrameRenderTime | Number | Average time in milliseconds that OBS is taking to render a frame |
| renderSkippedFrames | Number | Number of frames skipped by OBS in the render thread |
| renderTotalFrames | Number | Total number of frames outputted by the render thread |
| outputSkippedFrames | Number | Number of frames skipped by OBS in the output thread |
| outputTotalFrames | Number | Total number of frames outputted by the output thread |
| webSocketSessionIncomingMessages | Number | Total number of messages received by obs-websocket from the client |
| webSocketSessionOutgoingMessages | Number | Total number of messages sent by obs-websocket to the client |

---

### GetHotkeyList

Gets an array of all hotkey names in OBS

- Complexity Rating: `3/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Response Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| hotkeys | Array<String> | Array of hotkey names |

---

### TriggerHotkeyByName

Triggers a hotkey using its name. See `GetHotkeyList`

- Complexity Rating: `3/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| hotkeyName | String | Name of the hotkey to trigger | None | No | None |

---

### TriggerHotkeyByKeySequence

Triggers a hotkey using a sequence of keys.

- Complexity Rating: `4/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| keyId | String | The OBS key ID to use. See https://github.com/obsproject/obs-studio/blob/master/libobs/obs-hotkeys.h | None | No | None |
| keyModifiers | Object | Object containing key modifiers to apply | None | No | None |
| keyModifiers.shift | Boolean | Press Shift | None | No | None |
| keyModifiers.control | Boolean | Press CTRL | None | No | None |
| keyModifiers.alt | Boolean | Press ALT | None | No | None |
| keyModifiers.command | Boolean | Press CMD (Mac) | None | No | None |

---

### GetStudioModeEnabled

Gets whether studio is enabled.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Response Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| studioModeEnabled | Boolean | Whether studio mode is enabled |

---

### SetStudioModeEnabled

Enables or disables studio mode

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| studioModeEnabled | Boolean | True == Enabled, False == Disabled | None | No | None |

---

### Sleep

Sleeps for a time duration or number of frames. Only available in request batches with types `SERIAL_REALTIME` or `SERIAL_FRAME`.

- Complexity Rating: `2/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| sleepMillis | Number | Number of milliseconds to sleep for (if `SERIAL_REALTIME` mode) | >= 0, <= 50000 | No | None |
| sleepFrames | Number | Number of frames to sleep for (if `SERIAL_FRAME` mode) | >= 0, <= 10000 | No | None |


## Config

### GetPersistentData

Gets the value of a "slot" from the selected persistent data realm.

- Complexity Rating: `2/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| realm | String | The data realm to select. `OBS_WEBSOCKET_DATA_REALM_GLOBAL` or `OBS_WEBSOCKET_DATA_REALM_PROFILE` | None | No | None |
| slotName | String | The name of the slot to retrieve data from | None | No | None |


**Response Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| slotValue | String | Value associated with the slot. `null` if not set |

---

### SetPersistentData

Sets the value of a "slot" from the selected persistent data realm.

- Complexity Rating: `2/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| realm | String | The data realm to select. `OBS_WEBSOCKET_DATA_REALM_GLOBAL` or `OBS_WEBSOCKET_DATA_REALM_PROFILE` | None | No | None |
| slotName | String | The name of the slot to retrieve data from | None | No | None |
| slotValue | Any | The value to apply to the slot | None | No | None |

---

### GetSceneCollectionList

Gets an array of all scene collections

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Response Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| currentSceneCollectionName | String | The name of the current scene collection |
| sceneCollections | Array<String> | Array of all available scene collections |

---

### SetCurrentSceneCollection

Switches to a scene collection.

Note: This will block until the collection has finished changing.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| sceneCollectionName | String | Name of the scene collection to switch to | None | No | None |

---

### CreateSceneCollection

Creates a new scene collection, switching to it in the process.

Note: This will block until the collection has finished changing.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| sceneCollectionName | String | Name for the new scene collection | None | No | None |

---

### GetProfileList

Gets an array of all profiles

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Response Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| currentProfileName | String | The name of the current profile |
| profiles | Array<String> | Array of all available profiles |

---

### SetCurrentProfile

Switches to a profile.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| profileName | String | Name of the profile to switch to | None | No | None |

---

### CreateProfile

Creates a new profile, switching to it in the process

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| profileName | String | Name for the new profile | None | No | None |

---

### RemoveProfile

Removes a profile. If the current profile is chosen, it will change to a different profile first.

- Complexity Rating: `1/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| profileName | String | Name of the profile to remove | None | No | None |

---

### GetProfileParameter

Gets a parameter from the current profile's configuration.

- Complexity Rating: `3/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| parameterCategory | String | Category of the parameter to get | None | No | None |
| parameterName | String | Name of the parameter to get | None | No | None |


**Response Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| parameterValue | String | Value associated with the parameter. `null` if not set and no default |
| defaultParameterValue | String | Default value associated with the parameter. `null` if no default |

---

### SetProfileParameter

Sets the value of a parameter in the current profile's configuration.

- Complexity Rating: `3/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| parameterCategory | String | Category of the parameter to set | None | No | None |
| parameterName | String | Name of the parameter to set | None | No | None |
| parameterValue | String | Value of the parameter to set. Use `null` to delete | None | No | None |

---

### GetVideoSettings

Gets the current video settings.

Note: To get the true FPS value, divide the FPS numerator by the FPS denominator. Example: `60000/1001`

- Complexity Rating: `2/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Response Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| fpsNumerator | Number | Numerator of the fractional FPS value |
| fpsDenominator | Number | Denominator of the fractional FPS value |
| baseWidth | Number | Width of the base (canvas) resolution in pixels |
| baseHeight | Number | Height of the base (canvas) resolution in pixels |
| outputWidth | Number | Width of the output resolution in pixels |
| outputHeight | Number | Height of the output resolution in pixels |

---

### SetVideoSettings

Sets the current video settings.

Note: Fields must be specified in pairs. For example, you cannot set only `baseWidth` without needing to specify `baseHeight`.

- Complexity Rating: `2/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| fpsNumerator | Number | Numerator of the fractional FPS value | >= 1 | No | None |
| fpsDenominator | Number | Denominator of the fractional FPS value | >= 1 | No | None |
| baseWidth | Number | Width of the base (canvas) resolution in pixels | >= 1, <= 4096 | No | None |
| baseHeight | Number | Height of the base (canvas) resolution in pixels | >= 1, <= 4096 | No | None |
| outputWidth | Number | Width of the output resolution in pixels | >= 1, <= 4096 | No | None |
| outputHeight | Number | Height of the output resolution in pixels | >= 1, <= 4096 | No | None |

---

### GetStreamServiceSettings

Gets the current stream service settings (stream destination).

- Complexity Rating: `4/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Response Fields:**

| Name | Type  | Description |
| ---- | :---: | ----------- |
| streamServiceType | String | Stream service type, like `rtmp_custom` or `rtmp_common` |
| streamServiceSettings | Object | Stream service settings |

---

### SetStreamServiceSettings

Sets the current stream service settings (stream destination).

Note: Simple RTMP settings can be set with type `rtmp_custom` and the settings fields `server` and `key`.

- Complexity Rating: `4/5`
- Latest Supported RPC Version: `1`
- Added in v5.0.0


**Request Fields:**

| Name | Type  | Description | Restrictions | Optional? | Default Behavior (If Optional) |
| ---- | :---: | ----------- | :----------: | :-------: | ------------------------------ |
| streamServiceType | String | Type of stream service to apply. Example: `rtmp_common` or `rtmp_custom` | None | No | None |
| streamServiceSettings | Object | Settings to apply to the service | None | No | None |


