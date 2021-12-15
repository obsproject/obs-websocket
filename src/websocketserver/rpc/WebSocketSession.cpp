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

#include "WebSocketSession.h"
#include "../../eventhandler/types/EventSubscription.h"

WebSocketSession::WebSocketSession() :
	_remoteAddress(""),
	_connectedAt(0),
	_incomingMessages(0),
	_outgoingMessages(0),
	_encoding(0),
	_challenge(""),
	_rpcVersion(OBS_WEBSOCKET_RPC_VERSION),
	_isIdentified(false),
	_eventSubscriptions(EventSubscription::All)
{
}

std::string WebSocketSession::RemoteAddress()
{
	std::lock_guard<std::mutex> lock(_remoteAddressMutex);
	std::string ret(_remoteAddress);
	return ret;
}

void WebSocketSession::SetRemoteAddress(std::string address)
{
	std::lock_guard<std::mutex> lock(_remoteAddressMutex);
	_remoteAddress = address;
}

uint64_t WebSocketSession::ConnectedAt()
{
	return _connectedAt.load();
}

void WebSocketSession::SetConnectedAt(uint64_t at)
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

uint8_t WebSocketSession::Encoding()
{
	return _encoding.load();
}

void WebSocketSession::SetEncoding(uint8_t encoding)
{
	_encoding.store(encoding);
}

bool WebSocketSession::AuthenticationRequired()
{
	return _authenticationRequired.load();
}

void WebSocketSession::SetAuthenticationRequired(bool required)
{
	_authenticationRequired.store(required);
}

std::string WebSocketSession::Secret()
{
	std::lock_guard<std::mutex> lock(_secretMutex);
	std::string ret(_secret);
	return ret;
}

void WebSocketSession::SetSecret(std::string secret)
{
	std::lock_guard<std::mutex> lock(_secretMutex);
	_secret = secret;
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

uint64_t WebSocketSession::EventSubscriptions()
{
	return _eventSubscriptions.load();
}

void WebSocketSession::SetEventSubscriptions(uint64_t subscriptions)
{
	_eventSubscriptions.store(subscriptions);
}
