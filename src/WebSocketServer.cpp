#include <QtConcurrent>

#include "plugin-macros.generated.h"

#include "WebSocketServer.h"
#include "obs-websocket.h"
#include "Config.h"
#include "requesthandler/RequestHandler.h"

WebSocketServer::WebSocketServer() :
	QObject(nullptr),
	_sessionMutex(QMutex::Recursive),
	_sessions()
{
	_server.get_alog().clear_channels(websocketpp::log::alevel::frame_header | websocketpp::log::alevel::frame_payload | websocketpp::log::alevel::control);
	_server.init_asio();

#ifndef _WIN32
	_server.set_reuse_addr(true);
#endif

	_server.set_open_handler(
		websocketpp::lib::bind(
			&WebSocketServer::onOpen, this, websocketpp::lib::placeholders::_1
		)
	);
	_server.set_close_handler(
		websocketpp::lib::bind(
			&WebSocketServer::onClose, this, websocketpp::lib::placeholders::_1
		)
	);
	_server.set_message_handler(
		websocketpp::lib::bind(
			&WebSocketServer::onMessage, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2
		)
	);

	blog(LOG_INFO, "test");
}

WebSocketServer::~WebSocketServer()
{
	Stop();
}

void WebSocketServer::Start()
{
	;
}

void WebSocketServer::Stop()
{
	;
}

void WebSocketServer::InvalidateSession(websocketpp::connection_hdl hdl)
{
	;
}

std::vector<WebSocketServer::WebSocketState> WebSocketServer::GetWebSocketSessions()
{
	std::vector<WebSocketServer::WebSocketState> webSocketSessions;
	return webSocketSessions;
}

std::string WebSocketServer::GetConnectUrl()
{
	return "";
}

void WebSocketServer::BroadcastEvent(uint64_t requiredIntent, std::string eventType, json eventData)
{
	;
}

WebSocketSession *WebSocketServer::GetWebSocketSession(websocketpp::connection_hdl hdl)
{
	return new WebSocketSession();
}

void WebSocketServer::onOpen(websocketpp::connection_hdl hdl)
{
	;
}

void WebSocketServer::onClose(websocketpp::connection_hdl hdl)
{
	;
}

void WebSocketServer::onMessage(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr message)
{
	;
}
