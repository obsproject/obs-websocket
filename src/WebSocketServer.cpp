#include <chrono>
#include <thread>
#include <QtConcurrent>
#include <QDateTime>
#include <QTime>

#include "WebSocketServer.h"
#include "WebSocketProtocol.h"
#include "obs-websocket.h"
#include "Config.h"
#include "utils/Utils.h"

#include "plugin-macros.generated.h"

WebSocketServer::WebSocketServer() :
	QObject(nullptr),
	_sessions()
{
	// Randomize the random number generator
	qsrand(QTime::currentTime().msec());

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
	blog(LOG_INFO, "[WebSocketServer::ServerRunner] IO thread started.");
	try {
		_server.run();
	} catch (websocketpp::exception const & e) {
		blog(LOG_ERROR, "[WebSocketServer::ServerRunner] websocketpp instance returned an error: %s", e.what());
	} catch (const std::exception & e) {
		blog(LOG_ERROR, "[WebSocketServer::ServerRunner] websocketpp instance returned an error: %s", e.what());
	} catch (...) {
		blog(LOG_ERROR, "[WebSocketServer::ServerRunner] websocketpp instance returned an error");
	}
	blog(LOG_INFO, "[WebSocketServer::ServerRunner] IO thread exited.");
}

void WebSocketServer::Start()
{
	if (_server.is_listening()) {
		blog(LOG_WARNING, "[WebSocketServer::Start] Call to Start() but the server is already listening.");
		return;
	}

	auto conf = GetConfig();
	if (!conf) {
		blog(LOG_ERROR, "[WebSocketServer::Start] Unable to retreive config!");
		return;
	}

	_serverPort = conf->ServerPort;
	_serverPassword = conf->ServerPassword;
	_debugEnabled = conf->DebugEnabled;
	AuthenticationRequired = conf->AuthRequired;
	AuthenticationSalt = Utils::Crypto::GenerateSalt();
	AuthenticationSecret = Utils::Crypto::GenerateSecret(conf->ServerPassword.toStdString(), AuthenticationSalt);

	// Set log levels if debug is enabled
	if (_debugEnabled) {
		_server.get_alog().set_channels(websocketpp::log::alevel::all);
		_server.get_alog().clear_channels(websocketpp::log::alevel::frame_header | websocketpp::log::alevel::frame_payload | websocketpp::log::alevel::control);
		_server.get_elog().set_channels(websocketpp::log::elevel::all);
		_server.get_alog().clear_channels(websocketpp::log::elevel::devel | websocketpp::log::elevel::library);
	} else {
		_server.get_alog().clear_channels(websocketpp::log::alevel::all);
		_server.get_elog().clear_channels(websocketpp::log::elevel::all);
	}

	_server.reset();

	websocketpp::lib::error_code errorCode;
	_server.listen(websocketpp::lib::asio::ip::tcp::v4(), _serverPort, errorCode);

	if (errorCode) {
		std::string errorCodeMessage = errorCode.message();
		blog(LOG_INFO, "[WebSocketServer::Start] Listen failed: %s", errorCodeMessage.c_str());
		return;
	}

	_server.start_accept();

	_serverThread = std::thread(&WebSocketServer::ServerRunner, this);

	blog(LOG_INFO, "[WebSocketServer::Start] Server started successfully on port %d. Possible connect address: %s", _serverPort, Utils::Platform::GetLocalAddress().c_str());
}

void WebSocketServer::Stop()
{
	if (!_server.is_listening()) {
		blog(LOG_WARNING, "[WebSocketServer::Stop] Call to Stop() but the server is not listening.");
		return;
	}

	_server.stop_listening();

	std::unique_lock<std::mutex> lock(_sessionMutex);
	for (auto const& [hdl, session] : _sessions) {
		websocketpp::lib::error_code errorCode;
		_server.pause_reading(hdl, errorCode);
		if (errorCode) {
			blog(LOG_INFO, "[WebSocketServer::Stop] Error: %s", errorCode.message().c_str());
			continue;
		}

		_server.close(hdl, websocketpp::close::status::going_away, "Server stopping.", errorCode);
		if (errorCode) {
			blog(LOG_INFO, "[WebSocketServer::Stop] Error: %s", errorCode.message().c_str());
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

	blog(LOG_INFO, "[WebSocketServer::Stop] Server stopped successfully");
}

void WebSocketServer::InvalidateSession(websocketpp::connection_hdl hdl)
{
	blog(LOG_INFO, "[WebSocketServer::InvalidateSession] Invalidating a session.");

	websocketpp::lib::error_code errorCode;
	_server.pause_reading(hdl, errorCode);
	if (errorCode) {
		blog(LOG_INFO, "[WebSocketServer::InvalidateSession] Error: %s", errorCode.message().c_str());
		return;
	}

	_server.close(hdl, WebSocketCloseCode::SessionInvalidated, "Your session has been invalidated.", errorCode);
	if (errorCode) {
		blog(LOG_INFO, "[WebSocketServer::InvalidateSession] Error: %s", errorCode.message().c_str());
		return;
	}
}

std::vector<WebSocketServer::WebSocketSessionState> WebSocketServer::GetWebSocketSessions()
{
	std::vector<WebSocketServer::WebSocketSessionState> webSocketSessions;

	std::unique_lock<std::mutex> lock(_sessionMutex);
	for (auto & [hdl, session] : _sessions) {
		uint64_t connectedAt = session->ConnectedAt();
		uint64_t incomingMessages = session->IncomingMessages();
		uint64_t outgoingMessages = session->OutgoingMessages();
		std::string remoteAddress = session->RemoteAddress();
		
		webSocketSessions.emplace_back(WebSocketSessionState{hdl, remoteAddress, connectedAt, incomingMessages, outgoingMessages});
	}
	lock.unlock();

	return webSocketSessions;
}

QString WebSocketServer::GetConnectString()
{
	QString ret;
	if (AuthenticationRequired)
		ret = QString("obswebsocket|%1:%2|%3").arg(QString::fromStdString(Utils::Platform::GetLocalAddress())).arg(_serverPort).arg(_serverPassword);
	else
		ret = QString("obswebsocket|%1:%2").arg(QString::fromStdString(Utils::Platform::GetLocalAddress())).arg(_serverPort);
	return ret;
}

// It isn't consistent to directly call the WebSocketServer from the events system, but it would also be dumb to make it unnecessarily complicated.
void WebSocketServer::BroadcastEvent(uint64_t requiredIntent, std::string eventType, json eventData)
{
	QtConcurrent::run(&_threadPool, [=]() {
		// Populate message object
		json eventMessage;
		eventMessage["messageType"] = "Event";
		eventMessage["eventType"] = eventType;
		if (eventData.is_object())
			eventMessage["eventData"] = eventData;

		// Initialize objects. The broadcast process only dumps the data when its needed.
		std::string messageJson;
		std::string messageMsgPack;

		// Recurse connected sessions and send the event to suitable sessions.
		std::unique_lock<std::mutex> lock(_sessionMutex);
		for (auto & it : _sessions) {
			if (!it.second->IsIdentified()) {
				continue;
			}
			if ((it.second->EventSubscriptions() & requiredIntent) != 0) {
				websocketpp::lib::error_code errorCode;
				switch (it.second->Encoding()) {
					case WebSocketEncoding::Json:
						if (messageJson.empty()) {
							messageJson = eventMessage.dump();
						}
						_server.send((websocketpp::connection_hdl)it.first, messageJson, websocketpp::frame::opcode::text, errorCode);
						it.second->IncrementOutgoingMessages();
						break;
					case WebSocketEncoding::MsgPack:
						if (messageMsgPack.empty()) {
							auto msgPackData = json::to_msgpack(eventMessage);
							messageMsgPack = std::string(msgPackData.begin(), msgPackData.end());
						}
						_server.send((websocketpp::connection_hdl)it.first, messageMsgPack, websocketpp::frame::opcode::binary, errorCode);
						it.second->IncrementOutgoingMessages();
						break;
				}
			}
		}
		lock.unlock();
		if (_debugEnabled)
			blog(LOG_INFO, "[WebSocketServer::BroadcastEvent] Outgoing event:\n%s", eventMessage.dump(2).c_str());
	});
}

void WebSocketServer::onOpen(websocketpp::connection_hdl hdl)
{
	auto conn = _server.get_con_from_hdl(hdl);

	// Build new session
	std::unique_lock<std::mutex> lock(_sessionMutex);
	SessionPtr session = _sessions[hdl] = std::make_shared<WebSocketSession>();
	std::unique_lock<std::mutex> sessionLock(session->OperationMutex);
	lock.unlock();

	// Configure session details
	session->SetRemoteAddress(conn->get_remote_endpoint());
	session->SetConnectedAt(QDateTime::currentSecsSinceEpoch());
	std::string contentType = conn->get_request_header("Content-Type");
	if (contentType == "") {
		;
	} else if (contentType == "application/json") {
		session->SetEncoding(WebSocketEncoding::Json);
	} else if (contentType == "application/msgpack") {
		session->SetEncoding(WebSocketEncoding::MsgPack);
	} else {
		conn->close(WebSocketCloseCode::InvalidContentType, "Your HTTP `Content-Type` header specifies an invalid encoding type.");
		return;
	}

	// Build `Hello`
	json helloMessage;
	helloMessage["messageType"] = "Hello";
	helloMessage["obsWebSocketVersion"] = OBS_WEBSOCKET_VERSION;
	helloMessage["rpcVersion"] = OBS_WEBSOCKET_RPC_VERSION;
	// todo: Add request and event lists
	if (AuthenticationRequired) {
		std::string sessionChallenge = Utils::Crypto::GenerateSalt();
		session->SetChallenge(sessionChallenge);
		helloMessage["authentication"] = {};
		helloMessage["authentication"]["challenge"] = sessionChallenge;
		helloMessage["authentication"]["salt"] = AuthenticationSalt;
	}

	sessionLock.unlock();

	// Send object to client
	websocketpp::lib::error_code errorCode;
	auto sessionEncoding = session->Encoding();
	if (sessionEncoding == WebSocketEncoding::Json) {
		std::string helloMessageJson = helloMessage.dump();
		_server.send(hdl, helloMessageJson, websocketpp::frame::opcode::text, errorCode);
	} else if (sessionEncoding == WebSocketEncoding::MsgPack) {
		auto msgPackData = json::to_msgpack(helloMessage);
		std::string messageMsgPack(msgPackData.begin(), msgPackData.end());
		_server.send(hdl, messageMsgPack, websocketpp::frame::opcode::binary, errorCode);
	}
	session->IncrementOutgoingMessages();
}

void WebSocketServer::onClose(websocketpp::connection_hdl hdl)
{
	auto conn = _server.get_con_from_hdl(hdl);

	// Get info from the session and then delete it
	std::unique_lock<std::mutex> lock(_sessionMutex);
	SessionPtr session = _sessions[hdl];
	bool isIdentified = session->IsIdentified();
	uint64_t connectedAt = session->ConnectedAt();
	uint64_t incomingMessages = session->IncomingMessages();
	uint64_t outgoingMessages = session->OutgoingMessages();
	std::string remoteAddress = session->RemoteAddress();
	_sessions.erase(hdl);
	lock.unlock();

	// Build SessionState object for signal
	WebSocketSessionState state;
	state.remoteAddress = remoteAddress;
	state.connectedAt = connectedAt;
	state.incomingMessages = incomingMessages;
	state.outgoingMessages = outgoingMessages;

	// Emit signals
	emit ClientDisconnected(state, conn->get_local_close_code());
	if (isIdentified)
		emit IdentifiedClientDisconnected(state, conn->get_local_close_code());
}

void WebSocketServer::onMessage(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr message)
{
	auto opcode = message->get_opcode();
	std::string payload = message->get_payload();
	QtConcurrent::run(&_threadPool, [=]() {
		std::unique_lock<std::mutex> lock(_sessionMutex);
		SessionPtr session;
		try {
			session = _sessions.at(hdl);
		} catch (const std::out_of_range& oor) {
			return;
		}
		lock.unlock();

		session->IncrementIncomingMessages();

		json incomingMessage;

		// Check for invalid opcode and decode
		websocketpp::lib::error_code errorCode;
		uint8_t sessionEncoding = session->Encoding();
		if (sessionEncoding == WebSocketEncoding::Json) {
			if (opcode != websocketpp::frame::opcode::text) {
				if (!session->IgnoreInvalidMessages())
					_server.close(hdl, WebSocketCloseCode::MessageDecodeError, "Your session encoding is set to Json, but a binary message was received.", errorCode);
				return;
			}

			try {
				incomingMessage = json::parse(payload);
			} catch (json::parse_error& e) {
				if (!session->IgnoreInvalidMessages())
					_server.close(hdl, WebSocketCloseCode::MessageDecodeError, std::string("Unable to decode Json: ") + e.what(), errorCode);
				return;
			}
		} else if (sessionEncoding == WebSocketEncoding::MsgPack) {
			if (opcode != websocketpp::frame::opcode::binary) {
				if (!session->IgnoreInvalidMessages())
					_server.close(hdl, WebSocketCloseCode::MessageDecodeError, "Your session encoding is set to MsgPack, but a text message was received.", errorCode);
				return;
			}

			try {
				incomingMessage = json::from_msgpack(payload);
			} catch (json::parse_error& e) {
				if (!session->IgnoreInvalidMessages())
					_server.close(hdl, WebSocketCloseCode::MessageDecodeError, std::string("Unable to decode MsgPack: ") + e.what(), errorCode);
				return;
			}
		}

		if (_debugEnabled)
			blog(LOG_INFO, "[WebSocketServer::onMessage] Incoming message (decoded):\n%s", incomingMessage.dump(2).c_str());

		WebSocketProtocol::ProcessResult ret = WebSocketProtocol::ProcessMessage(session, incomingMessage);

		if (ret.closeCode != WebSocketCloseCode::DontClose) {
			websocketpp::lib::error_code errorCode;
			_server.close(hdl, ret.closeCode, ret.closeReason, errorCode);
			return;
		}

		if (!ret.result.is_null()) {
			websocketpp::lib::error_code errorCode;
			if (sessionEncoding == WebSocketEncoding::Json) {
				std::string helloMessageJson = ret.result.dump();
				_server.send(hdl, helloMessageJson, websocketpp::frame::opcode::text, errorCode);
			} else if (sessionEncoding == WebSocketEncoding::MsgPack) {
				auto msgPackData = json::to_msgpack(ret.result);
				std::string messageMsgPack(msgPackData.begin(), msgPackData.end());
				_server.send(hdl, messageMsgPack, websocketpp::frame::opcode::binary, errorCode);
			}
			session->IncrementOutgoingMessages();

			if (_debugEnabled)
				blog(LOG_INFO, "[WebSocketServer::onMessage] Outgoing message:\n%s", ret.result.dump(2).c_str());

			if (errorCode)
				blog(LOG_WARNING, "[WebSocketServer::onMessage] Sending message to client failed: %s", errorCode.message().c_str());
		}
	});
}
