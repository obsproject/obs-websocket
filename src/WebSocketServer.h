#pragma once

#include <QObject>
#include <QThreadPool>
#include <mutex>

#include <nlohmann/json.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "WebSocketSession.h"

using json = nlohmann::json;

class WebSocketServer
{
	public:
		enum WebSocketCloseCode: uint16_t {
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

		enum WebSocketEncoding: uint8_t {
			Json,
			MsgPack
		};

		struct WebSocketState {
			websocketpp::connection_hdl hdl;
			std::string remoteAddress;
			uint64_t durationSeconds;
			uint64_t incomingMessages;
			uint64_t outgoingMessages;
		};

		WebSocketServer();
		~WebSocketServer();

		void Start();
		void Stop();
		void InvalidateSession(websocketpp::connection_hdl hdl);

		bool IsListening() {
			return _server.is_listening();
		}

		std::vector<WebSocketState> GetWebSocketSessions();

		QThreadPool *GetThreadPool() {
			return &_threadPool;
		}

		std::string GetConnectUrl();

		void BroadcastEvent(uint64_t requiredIntent, std::string eventType, json eventData = nullptr);

	private:
		void ServerRunner();
		WebSocketSession *GetWebSocketSession(websocketpp::connection_hdl hdl);

		void onOpen(websocketpp::connection_hdl hdl);
		void onClose(websocketpp::connection_hdl hdl);
		void onMessage(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr message);

		std::thread _serverThread;
		websocketpp::server<websocketpp::config::asio> _server;
		QThreadPool _threadPool;
		std::mutex _sessionMutex;
		std::map<websocketpp::connection_hdl, WebSocketSession, std::owner_less<websocketpp::connection_hdl>> _sessions;
		uint16_t _serverPort;
		std::string _authenticationSecret;
		std::string _authenticationSalt;
};
