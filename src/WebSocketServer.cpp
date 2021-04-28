#include <chrono>
#include <thread>
#include <QtConcurrent>
#include <QDateTime>

#include "WebSocketServer.h"
#include "obs-websocket.h"
#include "Config.h"
#include "requesthandler/RequestHandler.h"
#include "utils/Utils.h"

#include "plugin-macros.generated.h"

WebSocketServer::WebSocketServer() :
	_sessions()
{
	_server.get_alog().clear_channels(websocketpp::log::alevel::all);
	_server.get_elog().clear_channels(websocketpp::log::elevel::all);
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
}

WebSocketServer::~WebSocketServer()
{
	if (_server.is_listening())
		Stop();
}

void WebSocketServer::ServerRunner()
{
	blog(LOG_INFO, "[ServerRunner] IO thread started.");
	try {
		_server.run();
	} catch (websocketpp::exception const & e) {
		blog(LOG_ERROR, "[ServerRunner] websocketpp instance returned an error: %s", e.what());
	} catch (const std::exception & e) {
		blog(LOG_ERROR, "[ServerRunner] websocketpp instance returned an error: %s", e.what());
	} catch (...) {
		blog(LOG_ERROR, "[ServerRunner] websocketpp instance returned an error");
	}
	blog(LOG_INFO, "[ServerRunner] IO thread exited.");
}

void WebSocketServer::Start()
{
	if (_server.is_listening()) {
		blog(LOG_WARNING, "[Start] Call to Start() but the server is already listening.");
		return;
	}

	auto conf = GetConfig();
	if (!conf) {
		blog(LOG_ERROR, "[Start] Unable to retreive config!");
		return;
	}

	_serverPort = conf->ServerPort;
	_authenticationSalt = Utils::Crypto::GenerateSalt();
	_authenticationSecret = Utils::Crypto::GenerateSecret(conf->ServerPassword.toStdString(), _authenticationSalt);

	// Set log levels if debug is enabled
	if (conf->DebugEnabled) {
		_server.get_alog().set_channels(websocketpp::log::alevel::all);
		_server.get_alog().clear_channels(websocketpp::log::alevel::frame_header | websocketpp::log::alevel::frame_payload | websocketpp::log::alevel::control);
		_server.get_elog().set_channels(websocketpp::log::elevel::all);
		_server.get_alog().clear_channels(websocketpp::log::elevel::info);
	} else {
		_server.get_alog().clear_channels(websocketpp::log::alevel::all);
		_server.get_elog().clear_channels(websocketpp::log::elevel::all);
	}

	_server.reset();

	websocketpp::lib::error_code errorCode;
	_server.listen(websocketpp::lib::asio::ip::tcp::v4(), _serverPort, errorCode);

	if (errorCode) {
		std::string errorCodeMessage = errorCode.message();
		blog(LOG_INFO, "[Start] Listen failed: %s", errorCodeMessage.c_str());
		return;
	}

	_server.start_accept();

	_serverThread = std::thread(&WebSocketServer::ServerRunner, this);

	blog(LOG_INFO, "[Start] Server started successfully on port %d. Possible connect address: %s", _serverPort, Utils::Platform::GetLocalAddress().c_str());
}

void WebSocketServer::Stop()
{
	if (!_server.is_listening()) {
		blog(LOG_WARNING, "[Stop] Call to Stop() but the server is not listening.");
		return;
	}

	_server.stop_listening();

	std::unique_lock<std::mutex> lock(_sessionMutex);
	for (auto const& [hdl, session] : _sessions) {
		websocketpp::lib::error_code errorCode;
		_server.pause_reading(hdl, errorCode);
		if (errorCode) {
			blog(LOG_INFO, "[Stop] Error: %s", errorCode.message().c_str());
			continue;
		}

		_server.close(hdl, websocketpp::close::status::going_away, "Server stopping.", errorCode);
		if (errorCode) {
			blog(LOG_INFO, "[Stop] Error: %s", errorCode.message().c_str());
			continue;
		}
	}
	lock.unlock();

	_threadPool.waitForDone();

	// This can delay the thread that it is running on. Bad but kinda required.
	while (_sessions.size() > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	_serverThread.join();

	blog(LOG_INFO, "[Stop] Server stopped successfully");
}

void WebSocketServer::InvalidateSession(websocketpp::connection_hdl hdl)
{
	blog(LOG_INFO, "[InvalidateSession] Invalidating a session.");

	websocketpp::lib::error_code errorCode;
	_server.pause_reading(hdl);
	if (errorCode) {
		blog(LOG_INFO, "[InvalidateSession] Error: %s", errorCode.message().c_str());
		return;
	}

	_server.close(hdl, WebSocketCloseCode::SessionInvalidated, "Your session has been invalidated.");
	if (errorCode) {
		blog(LOG_INFO, "[InvalidateSession] Error: %s", errorCode.message().c_str());
		return;
	}
}

std::vector<WebSocketServer::WebSocketSessionState> WebSocketServer::GetWebSocketSessions()
{
	std::vector<WebSocketServer::WebSocketSessionState> webSocketSessions;

	std::unique_lock<std::mutex> lock(_sessionMutex);
	for (auto & [hdl, session] : _sessions) {
		uint64_t connectedAt = session.ConnectedAt();
		uint64_t incomingMessages = session.IncomingMessages();
		uint64_t outgoingMessages = session.OutgoingMessages();
		std::string remoteAddress = session.RemoteAddress();
		
		webSocketSessions.emplace_back(WebSocketSessionState{hdl, remoteAddress, connectedAt, incomingMessages, outgoingMessages});
	}
	lock.unlock();

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
	auto conn = _server.get_con_from_hdl(hdl);

	std::unique_lock<std::mutex> lock(_sessionMutex);
	_sessions[hdl].SetRemoteAddress(conn->get_remote_endpoint());
	_sessions[hdl].SetConnectedAt(QDateTime::currentSecsSinceEpoch());
	lock.unlock();
}

void WebSocketServer::onClose(websocketpp::connection_hdl hdl)
{
	std::unique_lock<std::mutex> lock(_sessionMutex);
	_sessions.erase(hdl);
	lock.unlock();
}

void WebSocketServer::onMessage(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr message)
{
	;
}
