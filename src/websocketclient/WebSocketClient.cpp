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

#include "WebSocketClient.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <system_error>
#include <vector>

#include <obs-module.h>
#include <QDateTime>

#include <websocketpp/client.hpp>
#if OBS_WEBSOCKET_CLIENT_TLS
#include <websocketpp/config/asio_client.hpp>
#include <asio/ssl.hpp>
#include <asio/ssl/rfc2818_verification.hpp>
#else
#include <websocketpp/config/asio_no_tls_client.hpp>
#endif

#include "../Config.h"
#include "../obs-websocket.h"
#include "../utils/Compat.h"
#include "../utils/Crypto.h"

namespace {
	constexpr int kHeartbeatSeconds = 30;
	constexpr int kReconnectBaseSeconds = 1;
	constexpr int kReconnectMaxSeconds = 30;
	constexpr uint8_t kEncodingJson = 0;
	constexpr uint8_t kEncodingMsgPack = 1;

	uint64_t NowSeconds()
	{
		return static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch());
	}

	int BackoffSeconds(uint32_t attempt)
	{
		uint32_t shift = std::min<uint32_t>(attempt, 5);
		int delay = kReconnectBaseSeconds << shift;
		return std::min(delay, kReconnectMaxSeconds);
	}

	std::string NormalizeHostForUri(const std::string &host)
	{
		if (host.empty())
			return host;
		if (host.find(':') != std::string::npos && host.front() != '[' && host.find(']') == std::string::npos)
			return "[" + host + "]";
		return host;
	}

	std::string TrimWhitespace(const std::string &value)
	{
		const auto start = value.find_first_not_of(" \t\r\n");
		if (start == std::string::npos)
			return {};
		const auto end = value.find_last_not_of(" \t\r\n");
		return value.substr(start, end - start + 1);
	}

	bool ContainsWhitespace(const std::string &value)
	{
		return std::any_of(value.begin(), value.end(), [](unsigned char ch) { return std::isspace(ch) != 0; });
	}

	bool LooksLikeHostWithPort(const std::string &host)
	{
		if (host.empty())
			return false;

		const auto schemePos = host.find("://");
		if (schemePos != std::string::npos)
			return false;

		const auto bracketPos = host.find(']');
		if (host.front() == '[' && bracketPos != std::string::npos) {
			if (bracketPos + 1 < host.size() && host[bracketPos + 1] == ':') {
				const std::string portPart = host.substr(bracketPos + 2);
				return !portPart.empty() && std::all_of(portPart.begin(), portPart.end(),
									[](unsigned char ch) { return std::isdigit(ch) != 0; });
			}
			return false;
		}

		const auto colonPos = host.find(':');
		if (colonPos == std::string::npos)
			return false;
		if (host.find(':', colonPos + 1) != std::string::npos)
			return false;
		const std::string portPart = host.substr(colonPos + 1);
		return !portPart.empty() &&
		       std::all_of(portPart.begin(), portPart.end(), [](unsigned char ch) { return std::isdigit(ch) != 0; });
	}

} // namespace

class WebSocketClientTransport {
public:
	using OpenHandler = std::function<void(websocketpp::connection_hdl)>;
	using CloseHandler = std::function<void(websocketpp::connection_hdl)>;
	using FailHandler = std::function<void(websocketpp::connection_hdl)>;
	using MessageHandler =
		std::function<void(websocketpp::connection_hdl, websocketpp::frame::opcode::value, const std::string &)>;
#if OBS_WEBSOCKET_CLIENT_TLS
	using TlsInitHandler = std::function<std::shared_ptr<asio::ssl::context>(websocketpp::connection_hdl)>;
#else
	using TlsInitHandler = std::function<void(websocketpp::connection_hdl)>;
#endif

	virtual ~WebSocketClientTransport() = default;
	virtual void Init(bool debug) = 0;
	virtual void SetHandlers(OpenHandler open, CloseHandler close, FailHandler fail, MessageHandler message) = 0;
	virtual void SetTlsInitHandler(TlsInitHandler handler) = 0;
	virtual websocketpp::lib::error_code Connect(const std::string &uri, websocketpp::connection_hdl &outHdl,
						     const std::vector<std::string> &subprotocols) = 0;
	virtual void Run() = 0;
	virtual void Stop() = 0;
	virtual void Reset() = 0;
	virtual void Send(const websocketpp::connection_hdl &hdl, const std::string &payload,
			  websocketpp::frame::opcode::value opcode, websocketpp::lib::error_code &ec) = 0;
	virtual void Ping(const websocketpp::connection_hdl &hdl, const std::string &payload, websocketpp::lib::error_code &ec) = 0;
	virtual void Close(const websocketpp::connection_hdl &hdl, uint16_t code, const std::string &reason,
			   websocketpp::lib::error_code &ec) = 0;
	virtual std::string GetRemoteEndpoint(const websocketpp::connection_hdl &hdl) = 0;
	virtual std::string GetSelectedSubprotocol(const websocketpp::connection_hdl &hdl) = 0;
	virtual std::string GetLocalCloseReason(const websocketpp::connection_hdl &hdl) = 0;
	virtual uint16_t GetLocalCloseCode(const websocketpp::connection_hdl &hdl) = 0;
	virtual std::string GetFailReason(const websocketpp::connection_hdl &hdl) = 0;
	virtual asio::io_service &GetIoService() = 0;
	virtual void Post(std::function<void()> fn) = 0;
};

class WebSocketClientTransportPlain : public WebSocketClientTransport {
public:
	using Client = websocketpp::client<websocketpp::config::asio_client>;

	void Init(bool debug) override
	{
		_client.get_alog().clear_channels(websocketpp::log::alevel::all);
		_client.get_elog().clear_channels(websocketpp::log::elevel::all);
		_client.init_asio();

		if (debug) {
			_client.get_alog().set_channels(websocketpp::log::alevel::all);
			_client.get_alog().clear_channels(websocketpp::log::alevel::frame_header |
							  websocketpp::log::alevel::frame_payload |
							  websocketpp::log::alevel::control);
			_client.get_elog().set_channels(websocketpp::log::elevel::all);
			_client.get_alog().clear_channels(websocketpp::log::elevel::devel | websocketpp::log::elevel::library);
		} else {
			_client.get_alog().clear_channels(websocketpp::log::alevel::all);
			_client.get_elog().clear_channels(websocketpp::log::elevel::all);
		}
	}

	void SetHandlers(OpenHandler open, CloseHandler close, FailHandler fail, MessageHandler message) override
	{
		_client.set_open_handler(open);
		_client.set_close_handler(close);
		_client.set_fail_handler(fail);
		_client.set_message_handler([message](websocketpp::connection_hdl hdl, Client::message_ptr msg) {
			message(hdl, msg->get_opcode(), msg->get_payload());
		});
	}

	void SetTlsInitHandler(TlsInitHandler) override {}

	websocketpp::lib::error_code Connect(const std::string &uri, websocketpp::connection_hdl &outHdl,
					     const std::vector<std::string> &subprotocols) override
	{
		websocketpp::lib::error_code ec;
		Client::connection_ptr connection = _client.get_connection(uri, ec);
		if (ec)
			return ec;
		for (const auto &subprotocol : subprotocols)
			connection->add_subprotocol(subprotocol);
		outHdl = connection->get_handle();
		_client.connect(connection);
		return ec;
	}

	void Run() override { _client.run(); }
	void Stop() override { _client.stop(); }
	void Reset() override { _client.reset(); }

	void Send(const websocketpp::connection_hdl &hdl, const std::string &payload, websocketpp::frame::opcode::value opcode,
		  websocketpp::lib::error_code &ec) override
	{
		_client.send(hdl, payload, opcode, ec);
	}

	void Ping(const websocketpp::connection_hdl &hdl, const std::string &payload, websocketpp::lib::error_code &ec) override
	{
		_client.ping(hdl, payload, ec);
	}

	void Close(const websocketpp::connection_hdl &hdl, uint16_t code, const std::string &reason,
		   websocketpp::lib::error_code &ec) override
	{
		_client.close(hdl, code, reason, ec);
	}

	std::string GetRemoteEndpoint(const websocketpp::connection_hdl &hdl) override
	{
		auto connection = _client.get_con_from_hdl(hdl);
		return connection->get_remote_endpoint();
	}

	std::string GetSelectedSubprotocol(const websocketpp::connection_hdl &hdl) override
	{
		auto connection = _client.get_con_from_hdl(hdl);
		return connection->get_subprotocol();
	}

	std::string GetLocalCloseReason(const websocketpp::connection_hdl &hdl) override
	{
		auto connection = _client.get_con_from_hdl(hdl);
		return connection->get_local_close_reason();
	}

	uint16_t GetLocalCloseCode(const websocketpp::connection_hdl &hdl) override
	{
		auto connection = _client.get_con_from_hdl(hdl);
		return connection->get_local_close_code();
	}

	std::string GetFailReason(const websocketpp::connection_hdl &hdl) override
	{
		auto connection = _client.get_con_from_hdl(hdl);
		return connection->get_ec().message();
	}

	asio::io_service &GetIoService() override { return _client.get_io_service(); }

	void Post(std::function<void()> fn) override { _client.get_io_service().post(std::move(fn)); }

private:
	Client _client;
};

#if OBS_WEBSOCKET_CLIENT_TLS
class WebSocketClientTransportTls : public WebSocketClientTransport {
public:
	using Client = websocketpp::client<websocketpp::config::asio_tls_client>;

	void Init(bool debug) override
	{
		_client.get_alog().clear_channels(websocketpp::log::alevel::all);
		_client.get_elog().clear_channels(websocketpp::log::elevel::all);
		_client.init_asio();

		if (debug) {
			_client.get_alog().set_channels(websocketpp::log::alevel::all);
			_client.get_alog().clear_channels(websocketpp::log::alevel::frame_header |
							  websocketpp::log::alevel::frame_payload |
							  websocketpp::log::alevel::control);
			_client.get_elog().set_channels(websocketpp::log::elevel::all);
			_client.get_alog().clear_channels(websocketpp::log::elevel::devel | websocketpp::log::elevel::library);
		} else {
			_client.get_alog().clear_channels(websocketpp::log::alevel::all);
			_client.get_elog().clear_channels(websocketpp::log::elevel::all);
		}
	}

	void SetHandlers(OpenHandler open, CloseHandler close, FailHandler fail, MessageHandler message) override
	{
		_client.set_open_handler(open);
		_client.set_close_handler(close);
		_client.set_fail_handler(fail);
		_client.set_message_handler([message](websocketpp::connection_hdl hdl, Client::message_ptr msg) {
			message(hdl, msg->get_opcode(), msg->get_payload());
		});
	}

	void SetTlsInitHandler(TlsInitHandler handler) override { _client.set_tls_init_handler(handler); }

	websocketpp::lib::error_code Connect(const std::string &uri, websocketpp::connection_hdl &outHdl,
					     const std::vector<std::string> &subprotocols) override
	{
		websocketpp::lib::error_code ec;
		Client::connection_ptr connection = _client.get_connection(uri, ec);
		if (ec)
			return ec;
		for (const auto &subprotocol : subprotocols)
			connection->add_subprotocol(subprotocol);
		outHdl = connection->get_handle();
		_client.connect(connection);
		return ec;
	}

	void Run() override { _client.run(); }
	void Stop() override { _client.stop(); }
	void Reset() override { _client.reset(); }

	void Send(const websocketpp::connection_hdl &hdl, const std::string &payload, websocketpp::frame::opcode::value opcode,
		  websocketpp::lib::error_code &ec) override
	{
		_client.send(hdl, payload, opcode, ec);
	}

	void Ping(const websocketpp::connection_hdl &hdl, const std::string &payload, websocketpp::lib::error_code &ec) override
	{
		_client.ping(hdl, payload, ec);
	}

	void Close(const websocketpp::connection_hdl &hdl, uint16_t code, const std::string &reason,
		   websocketpp::lib::error_code &ec) override
	{
		_client.close(hdl, code, reason, ec);
	}

	std::string GetRemoteEndpoint(const websocketpp::connection_hdl &hdl) override
	{
		auto connection = _client.get_con_from_hdl(hdl);
		return connection->get_remote_endpoint();
	}

	std::string GetSelectedSubprotocol(const websocketpp::connection_hdl &hdl) override
	{
		auto connection = _client.get_con_from_hdl(hdl);
		return connection->get_subprotocol();
	}

	std::string GetLocalCloseReason(const websocketpp::connection_hdl &hdl) override
	{
		auto connection = _client.get_con_from_hdl(hdl);
		return connection->get_local_close_reason();
	}

	uint16_t GetLocalCloseCode(const websocketpp::connection_hdl &hdl) override
	{
		auto connection = _client.get_con_from_hdl(hdl);
		return connection->get_local_close_code();
	}

	std::string GetFailReason(const websocketpp::connection_hdl &hdl) override
	{
		auto connection = _client.get_con_from_hdl(hdl);
		return connection->get_ec().message();
	}

	asio::io_service &GetIoService() override { return _client.get_io_service(); }

	void Post(std::function<void()> fn) override { _client.get_io_service().post(std::move(fn)); }

private:
	Client _client;
};
#endif

WebSocketClient::WebSocketClient() : _protocol()
{
	UpdateStatus(State::Disabled);
}

WebSocketClient::~WebSocketClient()
{
	Stop();
}

void WebSocketClient::Start()
{
	if (_shouldRun.load())
		return;

	ClientConfigSnapshot config = GetConfigSnapshot();
	if (!config.enabled) {
		UpdateStatus(State::Disabled, FormatEndpointForUi(config));
		return;
	}

	_reconnectAttempt = 0;
	UpdateReconnectAttempt(0);

	_shouldRun = true;
	_clientThread = std::thread(&WebSocketClient::ClientRunner, this);
}

void WebSocketClient::Stop()
{
	StopInternal(true);
}

void WebSocketClient::Restart()
{
	StopInternal(true);
	Start();
}

void WebSocketClient::StopInternal(bool joinThread)
{
	if (!_shouldRun.load() && !joinThread)
		return;

	_shouldRun = false;
	_reconnectCv.notify_all();
	_connected = false;
	_connecting = false;

	{
		std::lock_guard<std::mutex> lock(_transportMutex);
		if (_transport)
			_transport->Stop();
	}

	if (joinThread && _clientThread.joinable())
		_clientThread.join();

	StopHeartbeat();
	_reconnectAttempt = 0;
	UpdateReconnectAttempt(0);

	ClientConfigSnapshot config = GetConfigSnapshot();
	UpdateStatus(config.enabled ? State::Disconnected : State::Disabled, FormatEndpointForUi(config));
}

void WebSocketClient::SetObsReady(bool ready)
{
	_protocol.SetObsReady(ready);
}

void WebSocketClient::SetClientSubscriptionCallback(WebSocketProtocol::ClientSubscriptionCallback cb)
{
	{
		std::lock_guard<std::mutex> lock(_subscriptionMutex);
		_clientSubscriptionCallback = cb;
	}

	_protocol.SetClientSubscriptionCallback([this](bool type, uint64_t subs) {
		WebSocketProtocol::ClientSubscriptionCallback callback;
		{
			std::lock_guard<std::mutex> lock(_subscriptionMutex);
			_subscriptionActive = type;
			callback = _clientSubscriptionCallback;
		}
		if (callback)
			callback(type, subs);
	});
}

WebSocketClient::Status WebSocketClient::GetStatus()
{
	std::lock_guard<std::mutex> lock(_statusMutex);
	return _status;
}

void WebSocketClient::BroadcastEvent(uint64_t requiredIntent, const std::string &eventType, const json &eventData,
				     uint8_t rpcVersion)
{
	if (!_connected.load() || !_protocol.IsObsReady())
		return;

	_protocol.GetThreadPool()->start(Utils::Compat::CreateFunctionRunnable([=]() {
		SessionPtr session;
		{
			std::lock_guard<std::mutex> lock(_sessionMutex);
			session = _session;
		}
		if (!session || !session->IsIdentified())
			return;
		if (rpcVersion && session->RpcVersion() != rpcVersion)
			return;
		if ((session->EventSubscriptions() & requiredIntent) == 0)
			return;

		json eventMessage;
		eventMessage["op"] = 5;
		eventMessage["d"]["eventType"] = eventType;
		eventMessage["d"]["eventIntent"] = requiredIntent;
		if (eventData.is_object())
			eventMessage["d"]["eventData"] = eventData;

		SendPayload(eventMessage);
		if (IsDebugEnabled() && (EventSubscription::All & requiredIntent) != 0)
			blog(LOG_INFO, "[WebSocketClient::BroadcastEvent] Outgoing event:\n%s", eventMessage.dump(2).c_str());
	}));
}

void WebSocketClient::UpdateStatus(State state, const std::string &endpoint, const std::string &error)
{
	std::lock_guard<std::mutex> lock(_statusMutex);
	_status.state = state;
	if (!endpoint.empty())
		_status.endpoint = endpoint;
	if (!error.empty())
		_status.lastError = error;
	else if (state != State::Error)
		_status.lastError.clear();
	_status.lastStateChange = NowSeconds();
}

void WebSocketClient::UpdateReconnectAttempt(uint32_t attempt)
{
	std::lock_guard<std::mutex> lock(_statusMutex);
	_status.reconnectAttempt = attempt;
}

WebSocketClient::ClientConfigSnapshot WebSocketClient::GetConfigSnapshot()
{
	ClientConfigSnapshot snapshot;
	auto conf = GetConfig();
	if (!conf)
		return snapshot;

	snapshot.enabled = conf->ClientEnabled.load();
	snapshot.host = conf->ClientHost;
	snapshot.port = conf->ClientPort.load();
	snapshot.useTls = conf->ClientUseTls.load();
	snapshot.allowInsecure = conf->ClientAllowInsecure.load();
	snapshot.allowInvalidCert = conf->ClientAllowInvalidCert.load();
	snapshot.authRequired = conf->ClientAuthRequired.load();
	snapshot.password = conf->ClientPassword;
	snapshot.debug = conf->DebugEnabled.load();
	return snapshot;
}

std::string WebSocketClient::BuildEndpoint(const ClientConfigSnapshot &config, std::string &error)
{
	std::string hostInput = TrimWhitespace(config.host);
	if (hostInput.empty()) {
		error = "Client host is empty.";
		return {};
	}
	if (hostInput.find("://") != std::string::npos) {
		error = "Client host should not include a scheme (ws:// or wss://).";
		return {};
	}
	if (hostInput.find('/') != std::string::npos) {
		error = "Client host should not include a path.";
		return {};
	}
	if (ContainsWhitespace(hostInput)) {
		error = "Client host must not contain whitespace.";
		return {};
	}
	if (LooksLikeHostWithPort(hostInput)) {
		error = "Client host should not include a port; use the Client Port field.";
		return {};
	}
#if !OBS_WEBSOCKET_CLIENT_TLS
	if (config.useTls) {
		error = obs_module_text("OBSWebSocket.Settings.ClientTlsDisabledMessage");
		return {};
	}
#endif
	if (!config.useTls && !config.allowInsecure) {
		error = "Unencrypted ws:// is disabled. Enable the unsafe toggle to allow it.";
		return {};
	}

	std::string host = NormalizeHostForUri(hostInput);
	std::string scheme = config.useTls ? "wss" : "ws";
	return scheme + "://" + host + ":" + std::to_string(config.port);
}

std::string WebSocketClient::FormatEndpointForUi(const ClientConfigSnapshot &config)
{
	std::string hostInput = TrimWhitespace(config.host);
	if (hostInput.empty())
		return {};
	if (hostInput.find("://") != std::string::npos || hostInput.find('/') != std::string::npos ||
	    ContainsWhitespace(hostInput) || LooksLikeHostWithPort(hostInput)) {
		return hostInput;
	}
	std::string host = NormalizeHostForUri(hostInput);
	std::string scheme = config.useTls ? "wss" : "ws";
	return scheme + "://" + host + ":" + std::to_string(config.port);
}

void WebSocketClient::ClientRunner()
{
	_activeConfig = GetConfigSnapshot();

	while (_shouldRun.load()) {
		_activeConfig = GetConfigSnapshot();
		if (!_activeConfig.enabled) {
			UpdateStatus(State::Disabled, FormatEndpointForUi(_activeConfig));
			break;
		}

		std::string error;
		std::string endpoint = BuildEndpoint(_activeConfig, error);
		if (endpoint.empty()) {
			UpdateStatus(State::Error, FormatEndpointForUi(_activeConfig), error);
			uint32_t attempt = _reconnectAttempt.fetch_add(1) + 1;
			UpdateReconnectAttempt(attempt);
			int delay = BackoffSeconds(attempt);
			std::unique_lock<std::mutex> lock(_reconnectMutex);
			_reconnectCv.wait_for(lock, std::chrono::seconds(delay), [this]() { return !_shouldRun.load(); });
			continue;
		}

		UpdateStatus(State::Connecting, endpoint);
		_connecting = true;

		{
			std::lock_guard<std::mutex> lock(_transportMutex);
#if OBS_WEBSOCKET_CLIENT_TLS
			if (_activeConfig.useTls)
				_transport = std::make_shared<WebSocketClientTransportTls>();
			else
				_transport = std::make_shared<WebSocketClientTransportPlain>();
#else
			_transport = std::make_shared<WebSocketClientTransportPlain>();
#endif

			_transport->Init(_activeConfig.debug);
			_transport->SetHandlers([this](websocketpp::connection_hdl hdl) { HandleOpen(hdl); },
						[this](websocketpp::connection_hdl hdl) { HandleClose(hdl); },
						[this](websocketpp::connection_hdl hdl) { HandleFail(hdl); },
						[this](websocketpp::connection_hdl hdl, websocketpp::frame::opcode::value opCode,
						       const std::string &payload) { HandleMessage(hdl, opCode, payload); });

#if OBS_WEBSOCKET_CLIENT_TLS
			if (_activeConfig.useTls) {
				std::string hostForVerify = TrimWhitespace(_activeConfig.host);
				bool allowInvalid = _activeConfig.allowInvalidCert;
				_transport->SetTlsInitHandler([hostForVerify, allowInvalid](websocketpp::connection_hdl) {
					auto ctx = std::make_shared<asio::ssl::context>(asio::ssl::context::tls_client);
					ctx->set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 |
							 asio::ssl::context::no_sslv3 | asio::ssl::context::single_dh_use);
					if (allowInvalid) {
						ctx->set_verify_mode(asio::ssl::verify_none);
					} else {
						ctx->set_verify_mode(asio::ssl::verify_peer);
						ctx->set_default_verify_paths();
						ctx->set_verify_callback(asio::ssl::rfc2818_verification(hostForVerify));
					}
					return ctx;
				});
			}
#endif
		}

		websocketpp::connection_hdl hdl;
		websocketpp::lib::error_code ec;
		{
			std::lock_guard<std::mutex> lock(_transportMutex);
			ec = _transport->Connect(endpoint, hdl, {"obswebsocket.json", "obswebsocket.msgpack"});
		}
		if (ec) {
			UpdateStatus(State::Error, endpoint, ec.message());
			_connecting = false;
			uint32_t attempt = _reconnectAttempt.fetch_add(1) + 1;
			UpdateReconnectAttempt(attempt);
			int delay = BackoffSeconds(attempt);
			std::unique_lock<std::mutex> lock(_reconnectMutex);
			_reconnectCv.wait_for(lock, std::chrono::seconds(delay), [this]() { return !_shouldRun.load(); });
			continue;
		}

		{
			std::lock_guard<std::mutex> lock(_sessionMutex);
			_connection = hdl;
			_hasConnection = true;
		}

		std::shared_ptr<WebSocketClientTransport> transportSnapshot;
		{
			std::lock_guard<std::mutex> lock(_transportMutex);
			transportSnapshot = _transport;
		}
		if (transportSnapshot)
			transportSnapshot->Run();

		StopHeartbeat();
		SessionPtr session;
		{
			std::lock_guard<std::mutex> lock(_sessionMutex);
			session = _session;
		}
		NotifySubscriptionStopIfActive(session);
		{
			std::lock_guard<std::mutex> lock(_sessionMutex);
			_session.reset();
			_hasConnection = false;
		}

		_connecting = false;
		_connected = false;

		{
			std::lock_guard<std::mutex> lock(_transportMutex);
			if (_transport) {
				_transport->Reset();
				_transport.reset();
			}
		}

		if (!_shouldRun.load())
			break;

		uint32_t attempt = _reconnectAttempt.fetch_add(1) + 1;
		UpdateReconnectAttempt(attempt);
		UpdateStatus(State::Disconnected, endpoint);

		int delay = BackoffSeconds(attempt);
		std::unique_lock<std::mutex> lock(_reconnectMutex);
		_reconnectCv.wait_for(lock, std::chrono::seconds(delay), [this]() { return !_shouldRun.load(); });
	}
}

void WebSocketClient::HandleOpen(websocketpp::connection_hdl hdl)
{
	auto conf = GetConfig();
	if (!conf) {
		HandleFail(hdl);
		return;
	}

	std::shared_ptr<WebSocketClientTransport> transportSnapshot;
	{
		std::lock_guard<std::mutex> lock(_transportMutex);
		transportSnapshot = _transport;
	}
	if (!transportSnapshot)
		return;

	SessionPtr session = std::make_shared<WebSocketSession>();
	session->SetRemoteAddress(transportSnapshot->GetRemoteEndpoint(hdl));
	session->SetConnectedAt(NowSeconds());
	session->SetAuthenticationRequired(_activeConfig.authRequired);

	std::string selectedSubprotocol = transportSnapshot->GetSelectedSubprotocol(hdl);
	if (selectedSubprotocol == "obswebsocket.msgpack")
		session->SetEncoding(kEncodingMsgPack);
	else
		session->SetEncoding(kEncodingJson);

	if (session->AuthenticationRequired()) {
		_authenticationSalt = Utils::Crypto::GenerateSalt();
		_authenticationSecret = Utils::Crypto::GenerateSecret(_activeConfig.password, _authenticationSalt);
		session->SetSecret(_authenticationSecret);
		session->SetChallenge(Utils::Crypto::GenerateSalt());
	}

	{
		std::lock_guard<std::mutex> lock(_sessionMutex);
		_session = session;
	}

	json helloMessageData;
	helloMessageData["obsStudioVersion"] = obs_get_version_string();
	helloMessageData["obsWebSocketVersion"] = OBS_WEBSOCKET_VERSION;
	helloMessageData["rpcVersion"] = OBS_WEBSOCKET_RPC_VERSION;
	if (session->AuthenticationRequired()) {
		helloMessageData["authentication"] = json::object();
		helloMessageData["authentication"]["challenge"] = session->Challenge();
		helloMessageData["authentication"]["salt"] = _authenticationSalt;
	}
	json helloMessage;
	helloMessage["op"] = 0;
	helloMessage["d"] = helloMessageData;

	blog(LOG_INFO, "[WebSocketClient::HandleOpen] Connected to %s", session->RemoteAddress().c_str());
	if (IsDebugEnabled())
		blog_debug("[WebSocketClient::HandleOpen] Sending Op 0 (Hello) message:\n%s", helloMessage.dump(2).c_str());

	SendPayload(helloMessage);

	UpdateStatus(State::Connected, FormatEndpointForUi(_activeConfig));
	_connected = true;
	_connecting = false;
	_reconnectAttempt = 0;
	UpdateReconnectAttempt(0);
	StartHeartbeat();
}

void WebSocketClient::HandleClose(websocketpp::connection_hdl hdl)
{
	std::shared_ptr<WebSocketClientTransport> transportSnapshot;
	{
		std::lock_guard<std::mutex> lock(_transportMutex);
		transportSnapshot = _transport;
	}
	if (!transportSnapshot)
		return;

	SessionPtr session;
	{
		std::lock_guard<std::mutex> lock(_sessionMutex);
		session = _session;
	}

	NotifySubscriptionStopIfActive(session);

	uint16_t closeCode = transportSnapshot->GetLocalCloseCode(hdl);
	std::string closeReason = transportSnapshot->GetLocalCloseReason(hdl);
	if (!closeReason.empty())
		UpdateStatus(State::Disconnected, FormatEndpointForUi(_activeConfig), closeReason);
	else
		UpdateStatus(State::Disconnected, FormatEndpointForUi(_activeConfig));

	_connected = false;
	_connecting = false;
	StopHeartbeat();

	blog(LOG_INFO, "[WebSocketClient::HandleClose] WebSocket client disconnected (code %u): %s", closeCode,
	     closeReason.c_str());
}

void WebSocketClient::HandleFail(websocketpp::connection_hdl hdl)
{
	std::shared_ptr<WebSocketClientTransport> transportSnapshot;
	{
		std::lock_guard<std::mutex> lock(_transportMutex);
		transportSnapshot = _transport;
	}
	if (!transportSnapshot)
		return;

	SessionPtr session;
	{
		std::lock_guard<std::mutex> lock(_sessionMutex);
		session = _session;
	}
	NotifySubscriptionStopIfActive(session);

	std::string error = transportSnapshot->GetFailReason(hdl);
	UpdateStatus(State::Error, FormatEndpointForUi(_activeConfig), error);
	_connected = false;
	_connecting = false;
	StopHeartbeat();

	blog(LOG_WARNING, "[WebSocketClient::HandleFail] WebSocket client connection failed: %s", error.c_str());
}

void WebSocketClient::HandleMessage(websocketpp::connection_hdl hdl, websocketpp::frame::opcode::value opCode,
				    const std::string &payload)
{
	UNUSED_PARAMETER(hdl);
	_protocol.GetThreadPool()->start(Utils::Compat::CreateFunctionRunnable([=]() {
		SessionPtr session;
		{
			std::lock_guard<std::mutex> lock(_sessionMutex);
			session = _session;
		}
		if (!session)
			return;

		session->IncrementIncomingMessages();

		json incomingMessage;
		uint8_t sessionEncoding = session->Encoding();

		if (sessionEncoding == kEncodingJson) {
			if (opCode != websocketpp::frame::opcode::text) {
				CloseWithCode(WebSocketCloseCode::MessageDecodeError,
					      "Your session encoding is set to Json, but a binary message was received.");
				return;
			}
			try {
				incomingMessage = json::parse(payload);
			} catch (json::parse_error &e) {
				CloseWithCode(WebSocketCloseCode::MessageDecodeError,
					      std::string("Unable to decode Json: ") + e.what());
				return;
			}
		} else if (sessionEncoding == kEncodingMsgPack) {
			if (opCode != websocketpp::frame::opcode::binary) {
				CloseWithCode(WebSocketCloseCode::MessageDecodeError,
					      "Your session encoding is set to MsgPack, but a text message was received.");
				return;
			}
			try {
				incomingMessage = json::from_msgpack(payload);
			} catch (json::parse_error &e) {
				CloseWithCode(WebSocketCloseCode::MessageDecodeError,
					      std::string("Unable to decode MsgPack: ") + e.what());
				return;
			}
		}

		blog_debug("[WebSocketClient::HandleMessage] Incoming message (decoded):\n%s", incomingMessage.dump(2).c_str());

		WebSocketProtocol::ProcessResult ret;

		if (!incomingMessage.is_object()) {
			ret.closeCode = WebSocketCloseCode::MessageDecodeError;
			ret.closeReason = "You sent a non-object payload.";
			goto skipProcessing;
		}

		if (!session->IsIdentified() && incomingMessage.contains("request-type")) {
			ret.closeCode = WebSocketCloseCode::UnsupportedRpcVersion;
			ret.closeReason =
				"You appear to be attempting to connect with the pre-5.0.0 plugin protocol. Check to make sure your client is updated.";
			goto skipProcessing;
		}

		if (!incomingMessage.contains("op")) {
			ret.closeCode = WebSocketCloseCode::UnknownOpCode;
			ret.closeReason = "Your request is missing an `op`.";
			goto skipProcessing;
		}

		if (!incomingMessage["op"].is_number()) {
			ret.closeCode = WebSocketCloseCode::UnknownOpCode;
			ret.closeReason = "Your `op` is not a number.";
			goto skipProcessing;
		}

		_protocol.ProcessMessage(session, ret, incomingMessage["op"], incomingMessage["d"]);

	skipProcessing:
		if (ret.closeCode != WebSocketCloseCode::DontClose) {
			CloseWithCode(ret.closeCode, ret.closeReason);
			return;
		}

		if (!ret.result.is_null()) {
			SendPayload(ret.result);
		}
	}));
}

void WebSocketClient::NotifySubscriptionStopIfActive(const SessionPtr &session)
{
	if (!session || !session->IsIdentified())
		return;

	bool active = false;
	{
		std::lock_guard<std::mutex> lock(_subscriptionMutex);
		active = _subscriptionActive;
	}
	if (!active)
		return;

	_protocol.NotifyClientSubscriptionChange(false, session->EventSubscriptions());
}

void WebSocketClient::SendPayload(const json &payload)
{
	SessionPtr session;
	websocketpp::connection_hdl hdl;
	{
		std::lock_guard<std::mutex> lock(_sessionMutex);
		if (!_hasConnection || !_session)
			return;
		session = _session;
		hdl = _connection;
	}

	std::shared_ptr<WebSocketClientTransport> transportSnapshot;
	{
		std::lock_guard<std::mutex> lock(_transportMutex);
		transportSnapshot = _transport;
	}
	if (!transportSnapshot)
		return;

	uint8_t encoding = session->Encoding();
	std::string payloadData;
	websocketpp::frame::opcode::value opcode;

	if (encoding == kEncodingJson) {
		payloadData = payload.dump();
		opcode = websocketpp::frame::opcode::text;
	} else {
		auto msgPackData = json::to_msgpack(payload);
		payloadData = std::string(msgPackData.begin(), msgPackData.end());
		opcode = websocketpp::frame::opcode::binary;
	}

	session->IncrementOutgoingMessages();
	transportSnapshot->Post([transportSnapshot, hdl, payloadData, opcode]() {
		websocketpp::lib::error_code ec;
		transportSnapshot->Send(hdl, payloadData, opcode, ec);
		if (ec)
			blog(LOG_WARNING, "[WebSocketClient::SendPayload] Sending message failed: %s", ec.message().c_str());
	});
}

void WebSocketClient::CloseWithCode(WebSocketCloseCode::WebSocketCloseCode code, const std::string &reason)
{
	std::shared_ptr<WebSocketClientTransport> transportSnapshot;
	websocketpp::connection_hdl hdl;
	{
		std::lock_guard<std::mutex> lock(_sessionMutex);
		if (!_hasConnection)
			return;
		hdl = _connection;
	}
	{
		std::lock_guard<std::mutex> lock(_transportMutex);
		transportSnapshot = _transport;
	}
	if (!transportSnapshot)
		return;

	transportSnapshot->Post([transportSnapshot, hdl, code, reason]() {
		websocketpp::lib::error_code ec;
		transportSnapshot->Close(hdl, code, reason, ec);
	});
}

void WebSocketClient::StartHeartbeat()
{
	std::shared_ptr<WebSocketClientTransport> transportSnapshot;
	{
		std::lock_guard<std::mutex> lock(_transportMutex);
		transportSnapshot = _transport;
	}
	if (!transportSnapshot)
		return;

	_heartbeatTimer = std::make_unique<asio::steady_timer>(transportSnapshot->GetIoService());
	ScheduleHeartbeat();
}

void WebSocketClient::StopHeartbeat()
{
	if (_heartbeatTimer) {
		std::error_code ec;
		_heartbeatTimer->cancel(ec);
		_heartbeatTimer.reset();
	}
}

void WebSocketClient::ScheduleHeartbeat()
{
	if (!_heartbeatTimer)
		return;

	_heartbeatTimer->expires_after(std::chrono::seconds(kHeartbeatSeconds));
	_heartbeatTimer->async_wait([this](const std::error_code &ec) {
		if (ec || !_connected.load())
			return;

		std::shared_ptr<WebSocketClientTransport> transportSnapshot;
		websocketpp::connection_hdl hdl;
		{
			std::lock_guard<std::mutex> lock(_sessionMutex);
			if (!_hasConnection)
				return;
			hdl = _connection;
		}
		{
			std::lock_guard<std::mutex> lock(_transportMutex);
			transportSnapshot = _transport;
		}
		if (!transportSnapshot)
			return;

		transportSnapshot->Post([transportSnapshot, hdl]() {
			websocketpp::lib::error_code ec;
			transportSnapshot->Ping(hdl, "ping", ec);
			if (ec)
				blog(LOG_WARNING, "[WebSocketClient::Heartbeat] Ping failed: %s", ec.message().c_str());
		});

		ScheduleHeartbeat();
	});
}
