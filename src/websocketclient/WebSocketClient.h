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
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <asio.hpp>
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/frame.hpp>

#include "../utils/Json.h"
#include "../websocketserver/WebSocketProtocol.h"
#include "../websocketserver/rpc/WebSocketSession.h"

class WebSocketClientTransport;

class WebSocketClient {
public:
	enum class State { Disabled, Connecting, Connected, Disconnected, Error };

	struct Status {
		State state = State::Disabled;
		std::string endpoint;
		std::string lastError;
		uint64_t lastStateChange = 0;
		uint32_t reconnectAttempt = 0;
	};

	WebSocketClient();
	~WebSocketClient();

	void Start();
	void Stop();
	void Restart();

	void SetObsReady(bool ready);
	void SetClientSubscriptionCallback(WebSocketProtocol::ClientSubscriptionCallback cb);

	void BroadcastEvent(uint64_t requiredIntent, const std::string &eventType, const json &eventData = nullptr,
			    uint8_t rpcVersion = 0);

	Status GetStatus();

private:
	void ClientRunner();
	void StopInternal(bool joinThread);

	void UpdateStatus(State state, const std::string &endpoint = {}, const std::string &error = {});
	void UpdateReconnectAttempt(uint32_t attempt);

	struct ClientConfigSnapshot {
		bool enabled = false;
		std::string host;
		uint16_t port = 4455;
		bool useTls = true;
		bool allowInsecure = false;
		bool allowInvalidCert = false;
		bool authRequired = true;
		std::string password;
		bool debug = false;
	};

	ClientConfigSnapshot GetConfigSnapshot();
	std::string BuildEndpoint(const ClientConfigSnapshot &config, std::string &error);
	std::string FormatEndpointForUi(const ClientConfigSnapshot &config);

	void HandleOpen(websocketpp::connection_hdl hdl);
	void HandleClose(websocketpp::connection_hdl hdl);
	void HandleFail(websocketpp::connection_hdl hdl);
	void HandleMessage(websocketpp::connection_hdl hdl, websocketpp::frame::opcode::value opCode, const std::string &payload);

	void SendPayload(const json &payload);
	void CloseWithCode(WebSocketCloseCode::WebSocketCloseCode code, const std::string &reason);

	void NotifySubscriptionStopIfActive(const SessionPtr &session);

	void StartHeartbeat();
	void StopHeartbeat();
	void ScheduleHeartbeat();

	std::atomic<bool> _shouldRun = false;
	std::atomic<bool> _connected = false;
	std::atomic<bool> _connecting = false;
	std::atomic<uint32_t> _reconnectAttempt = 0;

	std::mutex _statusMutex;
	Status _status;

	std::mutex _sessionMutex;
	SessionPtr _session;
	websocketpp::connection_hdl _connection;
	bool _hasConnection = false;

	std::string _authenticationSecret;
	std::string _authenticationSalt;

	std::mutex _transportMutex;
	std::shared_ptr<WebSocketClientTransport> _transport;

	std::unique_ptr<asio::steady_timer> _heartbeatTimer;

	std::thread _clientThread;
	std::mutex _reconnectMutex;
	std::condition_variable _reconnectCv;

	std::mutex _subscriptionMutex;
	WebSocketProtocol::ClientSubscriptionCallback _clientSubscriptionCallback;
	bool _subscriptionActive = false;

	WebSocketProtocol _protocol;
	ClientConfigSnapshot _activeConfig;
};
