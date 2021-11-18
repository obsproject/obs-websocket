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

		bool IgnoreInvalidMessages();
		void SetIgnoreInvalidMessages(bool ignore);

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
		std::atomic<bool> _ignoreInvalidMessages;
		std::atomic<uint64_t> _eventSubscriptions;
};
