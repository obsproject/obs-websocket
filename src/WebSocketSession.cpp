#include "WebSocketSession.h"

#include "plugin-macros.generated.h"

WebSocketSession::WebSocketSession() :
	_connectedAt(0),
	_incomingMessages(0),
	_outgoingMessages(0),
	_encoding(0),
	_challenge(""),
	_rpcVersion(OBS_WEBSOCKET_RPC_VERSION),
	_isIdentified(false),
	_ignoreInvalidMessages(false),
	_ignoreNonFatalRequestChecks(false),
	_eventSubscriptions(0)
{
}

uint64_t WebSocketSession::ConnectedAt()
{
	return _connectedAt.load();
}

void WebSocketSession::SetConnectedAt(uint64_t at);
{
	_connectedAt.store(at);
}

uint64_t WebSocketSession::IncomingMessages()
{
	return _incomingMessages.load();
}

void WebSocketSession::IncrementIncomingMessages()
{
	_incomingMessages++;
}

uint64_t WebSocketSession::OutgoingMessages()
{
	return _outgoingMessages.load();
}

void WebSocketSession::IncrementOutgoingMessages()
{
	_outgoingMessages++;
}

uint8_t WebSocketSession::GetEncoding()
{
	return _encoding.load();
}

void WebSocketSession::SetEncoding(uint8_t encoding)
{
	_encoding.store(encoding);
}

std::string WebSocketSession::Challenge()
{
	std::lock_guard<std::mutex> lock(_challengeMutex);
	std::string ret(_challenge);
	return ret;
}

void WebSocketSession::SetChallenge(std::string challengeString)
{
	std::lock_guard<std::mutex> lock(_challengeMutex);
	_challenge = challengeString;
}

uint8_t WebSocketSession::RpcVersion()
{
	return _rpcVersion.load();
}

void WebSocketSession::SetRpcVersion(uint8_t version)
{
	_rpcVersion.store(version);
}

bool WebSocketSession::IsIdentified()
{
	return _isIdentified.load();
}

void WebSocketSession::SetIsIdentified(bool identified)
{
	_isIdentified.store(identified);
}

bool WebSocketSession::IgnoreInvalidMessages()
{
	return _ignoreInvalidMessages.load();
}

void WebSocketSession::SetIgnoreInvalidMessages(bool ignore)
{
	_ignoreInvalidMessages.store(ignore);
}

bool WebSocketSession::IgnoreNonFatalRequestChecks()
{
	return _ignoreNonFatalRequestChecks.load();
}

void WebSocketSession::SetIgnoreNonFatalRequestChecks(bool ignore)
{
	_ignoreNonFatalRequestChecks.store(ignore);
}

uint64_t WebSocketSession::EventSubscriptions()
{
	return _eventSubscriptions.load();
}

void WebSocketSession::SetEventSubscriptions(uint64_t subscriptions)
{
	_eventSubscriptions.store(subscriptions);
}