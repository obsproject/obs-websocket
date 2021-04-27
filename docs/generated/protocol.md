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

#### Notes
- The obs-websocket server listens for any messages containing a `request-type` field in the first level JSON from unidentified clients. If a message matches, the connection is dropped with `WebsocketCloseCode::UnsupportedProtocolVersion`.
- If a message with a `messageType` is not recognized to the obs-websocket server, the connection is dropped with `WebsocketCloseCode::UnknownMessageType`.
- At no point may the client send any message other than a single `Identify` before it has received an `Identified`. Breaking this rule will result in the connection being dropped by the server with `WebsocketCloseCode::NotIdentified`.
- The `Hello` object contains an `rpcVersion` field, which is the latest RPC version that the server supports.
  - If the server's version is is older than the client's, the client is allowed the capability to support older RPC versions. The client determines which RPC version it hopes to communicate on, and sends it via the `rpcVersion` field in the `Identify`.
  - If the server's version is newer than the client's, the client sends its highest supported version in its `Identify` in hopes that the server is backwards compatible to that version.
- If the `Hello` does not contain an `authentication` object, the resulting `Identify` object sent to the server does not need to have an `authentication` string.

### Creating an authentication string
obs-websocket uses SHA256 to transmit authentication credentials. The server starts by sending an object in the `authentication` field of its `Hello`. The client processes the authentication challenge and responds via the `authentication` string in `Identify`.

For this guide, we'll be using `supersecretpassword` as the password.

The `authentication` object in `Hello` looks like this (example):
```json
{
    "challenge": "ztTBnnuqrqaKDzRM3xcVdbYm",
    "salt": "PZVbYpvAnZut2SS6JNJytDm9"
}
```


## Message Types
The following message types are the base message types which may be sent to and from obs-websocket. 

### Hello
- Sent from: obs-websocket
- Sent to: Freshly connected websocket client
- Description:

### Identify
- Sent from: Freshly connected websocket client
- Sent to: obs-websocket
- Description: 

### Identified
- Sent from: obs-websocket
- Sent to: Freshly identified client
- Description: 

### Reidentify
- Sent from: Identified client
- Sent to: obs-websocket
- Description:

### Event
- Sent from: obs-websocket
- Sent to: All subscribed and identified clients
- Description: 

### Request
- Sent from: Identified client
- Sent to: obs-websocket
- Description: 

### RequestResponse
- Sent from: obs-websocket
- Sent to: Identified client which made the request
- Description: 

### RequestBatch
- Sent from: Identified client
- Sent to: obs-websocket
- Description: 

### RequestBatchResponse
- Sent from: obs-websocket
- Sent to: Identified client which made the request
- Description: 



# Table of Contents

<!-- toc -->

- [Events](#events)
- [Requests](#requests)

<!-- tocstop -->

## Events





## Requests


