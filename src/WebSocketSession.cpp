#include "WebSocketSession.h"

WebSocketSession::WebSocketSession() :
	incomingMessages(0),
	outgoingMessages(0),
	rpcVersion(OBS_WEBSOCKET_RPC_VERSION),
	isIdentified(false),
	ignoreInvalidMessages(false),
	ignoreNonFatalRequestChecks(false),
	eventSubscriptions(0)
{
}

uint64_t WebSocketSession::IncomingMessages()
{
	return incomingMessages.load();
}

void WebSocketSession::IncrementIncomingMessages()
{
	incomingMessages++;
}

uint64_t WebSocketSession::OutgoingMessages()
{
	return outgoingMessages.load();
}

void WebSocketSession::IncrementOutgoingMessages()
{
	outgoingMessages++;
}

uint8_t WebSocketSession::RpcVersion()
{
	return rpcVersion.load();
}

void WebSocketSession::SetRpcVersion(uint8_t version)
{
	rpcVersion.store(version);
}

bool WebSocketSession::IsIdentified()
{
	return isIdentified.load();
}

void WebSocketSession::SetIsIdentified(bool identified)
{
	isIdentified.store(identified);
}

bool WebSocketSession::IgnoreInvalidMessages()
{
	return ignoreInvalidMessages.load();
}

void WebSocketSession::SetIgnoreInvalidMessages(bool ignore)
{
	ignoreInvalidMessages.store(ignore);
}

bool WebSocketSession::IgnoreNonFatalRequestChecks()
{
	return ignoreNonFatalRequestChecks.load();
}

void WebSocketSession::SetIgnoreNonFatalRequestChecks(bool ignore)
{
	ignoreNonFatalRequestChecks.store(ignore);
}

uint64_t WebSocketSession::EventSubscriptions()
{
	return eventSubscriptions.load();
}

void WebSocketSession::SetEventSubscriptions(uint64_t subscriptions)
{
	eventSubscriptions.store(subscriptions);
}