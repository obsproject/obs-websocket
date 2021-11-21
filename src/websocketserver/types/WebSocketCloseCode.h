/*
obs-websocket
Copyright (C) 2016-2021 Stephane Lepin <stephane.lepin@gmail.com>
Copyright (C) 2020-2021 Kyle Manning <tt2468@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#pragma once

namespace WebSocketCloseCode {
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
		// The server detected the usage of an old version of the obs-websocket RPC protocol.
		UnsupportedRpcVersion = 4009,
		// The websocket session has been invalidated by the obs-websocket server.
		SessionInvalidated = 4010,
		// A data key's value is invalid, in the case of things like enums.
		InvalidDataKeyValue = 4011,
		// A feature is not supported because of hardware/software limitations.
		UnsupportedFeature = 4012,
	};
}
