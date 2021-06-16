#pragma once

#include <QObject>
#include <QThreadPool>
#include <QString>
#include <mutex>

#include "utils/Utils.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "WebSocketSession.h"

class WebSocketServer : QObject
{
	Q_OBJECT

	public:
		enum WebSocketCloseCode {
			/**
			* @api
			* Internal only 
			*/
			DontClose = 0,
			/**
			* @api
			* Reserved 
			*/
			UnknownReason = 4000,
			/**
			* @api
			* The server was unable to decode the incoming websocket message 
			*/
			MessageDecodeError = 4001,
			/**
			* @api
			* The specified `messageType` was invalid or missing 
			*/
			UnknownMessageType = 4002,
			/**
			* @api
			* The client sent a websocket message without first sending `Identify` message 
			*/
			NotIdentified = 4003,
			/**
			* @api
			* The client sent an `Identify` message while already identified 
			*/
			AlreadyIdentified = 4004,
			/**
			* @api
			* The authentication attempt (via `Identify`) failed 
			*/
			AuthenticationFailed = 4005,
			/**
			* @api
			* There was an invalid parameter the client's `Identify` message 
			*/
			InvalidIdentifyParameter = 4006,
			/**
			* @api
			* A `Request` or `RequestBatch` was missing its `requestId` or `requestType` 
			*/
			RequestMissingRequiredField = 4007,
			/**
			* @api
			* The websocket session has been invalidated by the obs-websocket server. 
			*/
			SessionInvalidated = 4008,
			/**
			* @api
			* The server detected the usage of an old version of the obs-websocket protocol. 
			*/
			UnsupportedProtocolVersion = 4009,
			/**
			* @api
			* The requested `Content-Type` specified in the request HTTP header is invalid. 
			*/
			InvalidContentType = 4010,
		};

		enum WebSocketEncoding {
			Json,
			MsgPack
		};

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

		bool IsListening() {
			return _server.is_listening();
		}

		std::vector<WebSocketSessionState> GetWebSocketSessions();

		QThreadPool *GetThreadPool() {
			return &_threadPool;
		}

		bool AuthenticationRequired;
		std::string AuthenticationSecret;
		std::string AuthenticationSalt;

	public Q_SLOTS:
		void BroadcastEvent(uint64_t requiredIntent, std::string eventType, json eventData = nullptr, uint8_t rpcVersion = 0);

	signals:
		void ClientConnected(const WebSocketSessionState state);
		void ClientDisconnected(const WebSocketSessionState state, const uint16_t closeCode);

	private:
		void ServerRunner();

		void onOpen(websocketpp::connection_hdl hdl);
		void onClose(websocketpp::connection_hdl hdl);
		void onMessage(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr message);

		std::thread _serverThread;
		websocketpp::server<websocketpp::config::asio> _server;
		QThreadPool _threadPool;
		std::mutex _sessionMutex;
		std::map<websocketpp::connection_hdl, SessionPtr, std::owner_less<websocketpp::connection_hdl>> _sessions;
		uint16_t _serverPort;
		QString _serverPassword;
		bool _debugEnabled;
};
