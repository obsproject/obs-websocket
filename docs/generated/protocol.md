<!-- This file was generated based on handlebars templates. Do not edit directly! -->

# obs-websocket 5.0.0 protocol reference


## General Introduction
obs-websocket provides a feature-rich RPC communication protocol, giving access to much of OBS's feature set. This document contains everything you should know in order to make a connection and use obs-websocket's functionality to the fullest.


## Table of Contents
- [Connecting to obs-websocket](#connecting_to_obs-websocket)
  - [Connection steps](#connecting_steps)
  - [Creating an authentication string](#connecting_authentication_string)
- [Base message types](#message_types)
  - [Hello](#basemessage_hello)
  - [Identify](#basemessage_identify)
  - [Identified](#basemessage_identified)
  - [Reidentify](#basemessage_reidentify)
  - [Event](#basemessage_event)
  - [Request](#basemessage_request)
  - [RequestResponse](#basemessage_requestresponse)
  - [RequestBatch](#basemessage_requestbatch)
  - [RequestBatchResponse](#basemessage_requestbatchresponse)
- [Requests](#requests)
- [Events](#events)


## Connecting to obs-websocket {#connecting_to_obs-websocket}
Here's info on how to connect to obs-websocket

### Connection steps {#connecting_steps}
- Step 1

### Creating an authentication string {#connecting_authentication_string}
- Start by


## Message Types {#message_types}
The following message types (`messageType`) are the base message types which may be sent to and from obs-websocket. Sending a message with a `messageType` that is not recognized to the obs-websocket server will result in your connection being closed with `WebsocketCloseCode::UnknownMessageType`.

### Hello {#basemessage_hello}
- Sent from: obs-websocket
- Sent to: Freshly connected websocket client
- Description:

### Identify {#basemessage_identify}
- Sent from: Freshly connected websocket client
- Sent to: obs-websocket
- Description: 

### Identified {#basemessage_event}
- Sent from: obs-websocket
- Sent to: Freshly identified client
- Description: 

### Reidentify {#basemessage_reidentify}
- Sent from: Identified client
- Sent to: obs-websocket
- Description:

### Event {#basemessage_event}
- Sent from: obs-websocket
- Sent to: All subscribed and identified clients
- Description: 

### Request {#basemessage_request}
- Sent from: Identified client
- Sent to: obs-websocket
- Description: 

### RequestResponse {#basemessage_requestresponse}
- Sent from: obs-websocket
- Sent to: Identified client which made the request
- Description: 

### RequestBatch {#basemessage_requestbatch}
- Sent from: Identified client
- Sent to: obs-websocket
- Description: 

### RequestBatchResponse {#basemessage_requestbatchresponse}
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


