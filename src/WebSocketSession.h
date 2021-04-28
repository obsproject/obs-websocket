#pragma once

#include <mutex>
#include <string>
#include <atomic>

class WebSocketSession
{
	public:
		WebSocketSession();

		uint64_t ConnectedAt();
		void SetConnectedAt(uint64_t at);

		uint64_t IncomingMessages();
		void IncrementIncomingMessages();

		uint64_t OutgoingMessages();
		void IncrementOutgoingMessages();

		uint8_t GetEncoding();
		void SetEncoding(uint8_t encoding);

		std::string Challenge();
		void SetChallenge(std::string challenge);

		uint8_t RpcVersion();
		void SetRpcVersion(uint8_t version);

		bool IsIdentified();
		void SetIsIdentified(bool identified);

		bool IgnoreInvalidMessages();
		void SetIgnoreInvalidMessages(bool ignore);

		bool IgnoreNonFatalRequestChecks();
		void SetIgnoreNonFatalRequestChecks(bool ignore);

		uint64_t EventSubscriptions();
		void SetEventSubscriptions(uint64_t subscriptions);

	private:
		std::atomic<uint64_t> _connectedAt;
		std::atomic<uint64_t> _incomingMessages;
		std::atomic<uint64_t> _outgoingMessages;
		std::atomic<uint8_t> _encoding;
		std::mutex _challengeMutex;
		std::string _challenge;
		std::atomic<uint8_t> _rpcVersion;
		std::atomic<bool> _isIdentified;
		std::atomic<bool> _ignoreInvalidMessages;
		std::atomic<bool> _ignoreNonFatalRequestChecks;
		std::atomic<uint64_t> _eventSubscriptions;
};