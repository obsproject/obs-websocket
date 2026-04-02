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

#include <mutex>
#include <QObject>
#include <QString>
#include <asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "WebSocketProtocol.h"
#include "rpc/WebSocketSession.h"
#include "types/WebSocketCloseCode.h"
#include "types/WebSocketOpCode.h"
#include "../requesthandler/rpc/Request.h"
#include "../utils/Json.h"
#include "plugin-macros.generated.h"

class WebSocketServer : QObject {
	Q_OBJECT

public:
	enum WebSocketEncoding { Json, MsgPack };

	struct WebSocketSessionState {
		websocketpp::connection_hdl hdl;
		std::string remoteAddress;
		uint64_t connectedAt;
		uint64_t incomingMessages;
		uint64_t outgoingMessages;
		bool isIdentified;
	};

	WebSocketServer();
	~WebSocketServer();

	void Start();
	void Stop();
	void InvalidateSession(websocketpp::connection_hdl hdl);
	void BroadcastEvent(uint64_t requiredIntent, const std::string &eventType, const json &eventData = nullptr,
			    uint8_t rpcVersion = 0);
	inline void SetObsReady(bool ready) { _protocol.SetObsReady(ready); }
	inline bool IsListening() { return _server.is_listening(); }
	std::vector<WebSocketSessionState> GetWebSocketSessions();
	inline QThreadPool *GetThreadPool() { return _protocol.GetThreadPool(); }

	// Callback for when a client subscribes or unsubscribes. `true` for sub, `false` for unsub
	using ClientSubscriptionCallback = WebSocketProtocol::ClientSubscriptionCallback; // bool type, uint64_t eventSubscriptions
	inline void SetClientSubscriptionCallback(ClientSubscriptionCallback cb) { _protocol.SetClientSubscriptionCallback(cb); }

signals:
	void ClientConnected(WebSocketSessionState state);
	void ClientDisconnected(WebSocketSessionState state, uint16_t closeCode);

private:
	void ServerRunner();

	bool onValidate(websocketpp::connection_hdl hdl);
	void onOpen(websocketpp::connection_hdl hdl);
	void onClose(websocketpp::connection_hdl hdl);
	void onMessage(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr message);

	std::thread _serverThread;
	websocketpp::server<websocketpp::config::asio> _server;
	WebSocketProtocol _protocol;

	std::string _authenticationSecret;
	std::string _authenticationSalt;

	std::mutex _sessionMutex;
	std::map<websocketpp::connection_hdl, SessionPtr, std::owner_less<websocketpp::connection_hdl>> _sessions;
};
