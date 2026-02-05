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

#include <atomic>
#include <functional>
#include <string>

#include <QThreadPool>

#include "rpc/WebSocketSession.h"
#include "types/WebSocketCloseCode.h"
#include "types/WebSocketOpCode.h"
#include "../utils/Json.h"

class WebSocketProtocol {
public:
	using ClientSubscriptionCallback = std::function<void(bool, uint64_t)>;

	struct ProcessResult {
		WebSocketCloseCode::WebSocketCloseCode closeCode = WebSocketCloseCode::DontClose;
		std::string closeReason;
		json result;
	};

	WebSocketProtocol();

	void SetObsReady(bool ready);
	bool IsObsReady() const;
	void SetClientSubscriptionCallback(ClientSubscriptionCallback cb);
	void NotifyClientSubscriptionChange(bool type, uint64_t eventSubscriptions);
	QThreadPool *GetThreadPool();

	void ProcessMessage(SessionPtr session, ProcessResult &ret, WebSocketOpCode::WebSocketOpCode opCode, json &payloadData);

private:
	void SetSessionParameters(SessionPtr session, ProcessResult &ret, const json &payloadData);

	QThreadPool _threadPool;
	std::atomic<bool> _obsReady = false;
	ClientSubscriptionCallback _clientSubscriptionCallback;
};
