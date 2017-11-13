<!-- This file was generated based on handlebars templates. Do not edit directly! -->

# obs-websocket 4.2.1 protocol reference

**This is the reference for the unreleased obs-websocket 4.2.1. See the list below for older versions.**
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
- [Requests](#requests)

<!-- tocstop -->

# Events
Events are broadcast by the server to each connected client when a recognized action occurs within OBS.

An event message will contain at least the following base fields:
- `update-type` _String_: the type of event.
- `stream-timecode` _String (optional)_: time elapsed between now and stream start (only present if OBS Studio is streaming).
- `rec-timecode` _String (optional)_: time elapsed between now and recording start (only present if OBS Studio is recording).

Timecodes are sent using the format: `HH:MM:SS.mmm`

Additional fields may be present in the event message depending on the event type.





# Requests
Requests are sent by the client and require at least the following two fields:
- `request-type` _String_: String name of the request type.
- `message-id` _String_: Client defined identifier for the message, will be echoed in the response.

Once a request is sent, the server will return a JSON response with at least the following fields:
- `message-id` _String_: The client defined identifier specified in the request.
- `status` _String_: Response status, will be one of the following: `ok`, `error`
- `error` _String_: An error message accompanying an `error` status.

Additional information may be required/returned depending on the request type. See below for more information.


