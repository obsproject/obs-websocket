<!-- This file was generated based on handlebars templates. Do not edit directly! -->

# obs-websocket 5.0.0 protocol reference


## General Introduction
obs-websocket provides a feature-rich RPC communication protocol, giving access to much of OBS's feature set. This document contains everything you should know in order to make a connection and use obs-websocket's functionality to the fullest.


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
- Step 1

### Creating an authentication string
- Start by


## Message Types
The following message types (`messageType`) are the base message types which may be sent to and from obs-websocket. Sending a message with a `messageType` that is not recognized to the obs-websocket server will result in your connection being closed with `WebsocketCloseCode::UnknownMessageType`.

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


