#ifndef WSREQUESTHANDLER_H
#define WSREQUESTHANDLER_H

#include <map>
#include <set>
#include <QtWebSockets/QWebSocket>
#include <obs-frontend-api.h>

class WSRequestHandler : public QObject
{
	Q_OBJECT

	public:
		explicit WSRequestHandler(QWebSocket *client);
		~WSRequestHandler();
		void sendTextMessage(QString textMessage);

	private Q_SLOTS:
		void processTextMessage(QString textMessage);
		void socketDisconnected();

	Q_SIGNALS:
		void disconnected();

	private:
		QWebSocket *_client;
		bool _authenticated;
		unsigned long _messageId;
		const char *_requestType;
		obs_data_t *_requestData;

		std::map<std::string, void(*)(WSRequestHandler*)> messageMap;
		std::set<std::string> authNotRequired;

		void SendOKResponse(obs_data_t *additionalFields = NULL);
		void SendErrorResponse(const char *errorMessage);
		static void ErrNotImplemented(WSRequestHandler *owner);
		
		static void HandleGetVersion(WSRequestHandler *owner);
		static void HandleGetAuthRequired(WSRequestHandler *owner);
		static void HandleAuthenticate(WSRequestHandler *owner);

		static void HandleSetCurrentScene(WSRequestHandler *owner);
		static void HandleGetCurrentScene(WSRequestHandler *owner);
		static void HandleGetSceneList(WSRequestHandler *owner);
		static void HandleSetSourceRender(WSRequestHandler *owner);
		
		static void HandleGetStreamingStatus(WSRequestHandler *owner);
		static void HandleStartStopStreaming(WSRequestHandler *owner);
		static void HandleStartStopRecording(WSRequestHandler *owner);

		static void HandleGetTransitionList(WSRequestHandler *owner);
		static void HandleGetCurrentTransition(WSRequestHandler *owner);
		static void HandleSetCurrentTransition(WSRequestHandler *owner);
};

#endif // WSPROTOCOL_H