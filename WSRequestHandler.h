#ifndef WSREQUESTHANDLER_H
#define WSREQUESTHANDLER_H

#include <map>
#include <QtWebSockets/QWebSocket>
#include <obs-frontend-api.h>

class WSRequestHandler
{
	public:
		explicit WSRequestHandler(QWebSocket *client);
		~WSRequestHandler();
		void handleMessage(const char* message);

	private:
		QWebSocket *_client;
		long _messageId;
		const char *_requestType;
		obs_data_t *_requestData;

		std::map<std::string, void(*)(WSRequestHandler*)> messageMap;

		void SendOKResponse(obs_data_t *additionalFields = NULL);
		void SendErrorResponse(const char *errorMessage);
		static void ErrNotImplemented(WSRequestHandler *owner);
		
		static void HandleGetVersion(WSRequestHandler *owner);
		static void HandleGetAuthRequired(WSRequestHandler *owner);
		static void HandleAuthenticate(WSRequestHandler *owner);
		static void HandleSetCurrentScene(WSRequestHandler *owner);
		static void HandleGetCurrentScene(WSRequestHandler *owner);
		static void HandleGetSceneList(WSRequestHandler *owner);
		static void HandleGetStreamingStatus(WSRequestHandler *owner);
		static void HandleStartStopStreaming(WSRequestHandler *owner);
		static void HandleStartStopRecording(WSRequestHandler *owner);
};

#endif // WSPROTOCOL_H