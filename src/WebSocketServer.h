#pragma once

#include <QObject>
#include <QThreadPool>
#include <QMutex>

#include <nlohmann/json.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "WebSocketSession.h"
#include "requesthandler/RequestHandler.h"

using json = nlohmann::json;

class WebSocketServer : public QObject
{
	Q_OBJECT

	public:
		WebSocketServer();
		~WebSocketServer();

		void Start();
		void Stop();
		void InvalidateSession(websocketpp::connection_hdl hdl);

		struct WebSocketState {
			websocketpp::connection_hdl hdl;
			std::string remoteAddress;
			uint64_t durationSeconds;
			uint64_t incomingMessages;
			uint64_t outgoingMessages;
		};

		std::vector<WebSocketState> GetWebSocketSessions();

		QThreadPool *GetThreadPool() {
			return &_threadPool;
		}

	public Q_SLOTS:
		void BroadcastEvent(uint64_t requiredIntent, std::string eventType, json eventData = nullptr);

	private:
		void onOpen(websocketpp::connection_hdl hdl);
		void onClose(websocketpp::connection_hdl hdl);
		void onMessage(websocketpp::connection_hdl hdl);

		WebSocketSession *GetWebSocketSession(websocketpp::connection_hdl hdl);

		websocketpp::server<websocketpp::config::asio> _server;
		QThreadPool _threadPool;

		QMutex _sessionMutex;
		std::map<websocketpp::connection_hdl, WebSocketSession, std::owner_less<websocketpp::connection_hdl>> _sessions;
};
