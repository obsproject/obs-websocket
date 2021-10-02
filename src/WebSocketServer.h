#pragma once

#include <mutex>
#include <QObject>
#include <QThreadPool>
#include <QString>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "utils/Json.h"
#include "WebSocketSession.h"
#include "requesthandler/rpc/Request.h"
#include "plugin-macros.generated.h"

class WebSocketServer : QObject
{
	Q_OBJECT

	public:
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

		WebSocketServer();
		~WebSocketServer();

		void Start();
		void Stop();
		void InvalidateSession(websocketpp::connection_hdl hdl);
		void BroadcastEvent(uint64_t requiredIntent, std::string eventType, json eventData = nullptr, uint8_t rpcVersion = 0);

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

	signals:
		void ClientConnected(const WebSocketSessionState state);
		void ClientDisconnected(const WebSocketSessionState state, const uint16_t closeCode);

	private:
		struct ProcessResult {
			WebSocketCloseCode closeCode = WebSocketCloseCode::DontClose;
			std::string closeReason;
			json result;
		};

		void ServerRunner();

		void onObsLoaded();
		bool onValidate(websocketpp::connection_hdl hdl);
		void onOpen(websocketpp::connection_hdl hdl);
		void onClose(websocketpp::connection_hdl hdl);
		void onMessage(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr message);

		void SetSessionParameters(SessionPtr session, WebSocketServer::ProcessResult &ret, const json &payloadData);
		void ProcessMessage(SessionPtr session, ProcessResult &ret, const uint8_t opCode, json &payloadData);

		void ProcessRequestBatch(SessionPtr session, ObsWebSocketRequestBatchExecutionType executionType, std::vector<json> &requests, std::vector<json> &results, json &variables);

		std::thread _serverThread;
		websocketpp::server<websocketpp::config::asio> _server;
		QThreadPool _threadPool;
		std::mutex _sessionMutex;
		std::map<websocketpp::connection_hdl, SessionPtr, std::owner_less<websocketpp::connection_hdl>> _sessions;
		uint16_t _serverPort;
		QString _serverPassword;
		bool _debugEnabled;
};
