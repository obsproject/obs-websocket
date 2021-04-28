#pragma once

#include <mutex>
#include <string>
#include <atomic>

class WebSocketSession
{
	public:
		WebSocketSession();

		std::string Challenge();
		void SetChallenge(std::string challenge);

		uint64_t IncomingMessages();
		void IncrementIncomingMessages();

		uint64_t OutgoingMessages();
		void IncrementOutgoingMessages();

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
		std::mutex challengeMutex;
		std::string challenge;
		std::atomic<uint64_t> incomingMessages;
		std::atomic<uint64_t> outgoingMessages;
		std::atomic<uint8_t> rpcVersion;
		std::atomic<bool> isIdentified;
		std::atomic<bool> ignoreInvalidMessages;
		std::atomic<bool> ignoreNonFatalRequestChecks;
		std::atomic<uint64_t> eventSubscriptions;
};