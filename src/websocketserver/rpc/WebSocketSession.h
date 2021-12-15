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

#pragma once

#include <mutex>
#include <string>
#include <atomic>
#include <memory>

#include "../../plugin-macros.generated.h"

class WebSocketSession;
typedef std::shared_ptr<WebSocketSession> SessionPtr;

class WebSocketSession
{
	public:
		WebSocketSession();

		std::string RemoteAddress();
		void SetRemoteAddress(std::string address);

		uint64_t ConnectedAt();
		void SetConnectedAt(uint64_t at);

		uint64_t IncomingMessages();
		void IncrementIncomingMessages();

		uint64_t OutgoingMessages();
		void IncrementOutgoingMessages();

		uint8_t Encoding();
		void SetEncoding(uint8_t encoding);

		bool AuthenticationRequired();
		void SetAuthenticationRequired(bool required);

		std::string Secret();
		void SetSecret(std::string secret);

		std::string Challenge();
		void SetChallenge(std::string challenge);

		uint8_t RpcVersion();
		void SetRpcVersion(uint8_t version);

		bool IsIdentified();
		void SetIsIdentified(bool identified);

		uint64_t EventSubscriptions();
		void SetEventSubscriptions(uint64_t subscriptions);

		std::mutex OperationMutex;

	private:
		std::mutex _remoteAddressMutex;
		std::string _remoteAddress;
		std::atomic<uint64_t> _connectedAt;
		std::atomic<uint64_t> _incomingMessages;
		std::atomic<uint64_t> _outgoingMessages;
		std::atomic<uint8_t> _encoding;
		std::atomic<bool> _authenticationRequired;
		std::mutex _secretMutex;
		std::string _secret;
		std::mutex _challengeMutex;
		std::string _challenge;
		std::atomic<uint8_t> _rpcVersion;
		std::atomic<bool> _isIdentified;
		std::atomic<uint64_t> _eventSubscriptions;
};
