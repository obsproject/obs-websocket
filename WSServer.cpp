#include "WSServer.h"
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QtCore/QDebug>
#include <obs-frontend-api.h>

QT_USE_NAMESPACE

WSServer::WSServer(quint16 port, QObject *parent) :
	QObject(parent),
	_wsServer(Q_NULLPTR),
	_clients()
{
	_wsServer = new QWebSocketServer(
		QStringLiteral("OBS Websocket API"),
		QWebSocketServer::NonSecureMode,
		this);

	bool serverStarted = _wsServer->listen(QHostAddress::Any, port);
	if (serverStarted) {
		connect(_wsServer, &QWebSocketServer::newConnection, this, &WSServer::onNewConnection);
	}
}

WSServer::~WSServer()
{
	_wsServer->close();
	qDeleteAll(_clients.begin(), _clients.end());
}

void WSServer::broadcast(QString message)
{
	Q_FOREACH(QWebSocket *pClient, _clients) {
		pClient->sendTextMessage(message);
	}
}

void WSServer::onNewConnection()
{
	QWebSocket *pSocket = _wsServer->nextPendingConnection();

	blog(LOG_INFO, "[obs-websockets] new client connected from %s:%d", pSocket->peerAddress().toString().toStdString(), pSocket->peerPort());

	connect(pSocket, &QWebSocket::textMessageReceived, this, &WSServer::processTextMessage);
	connect(pSocket, &QWebSocket::disconnected, this, &WSServer::socketDisconnected);

	_clients << pSocket;
}

void WSServer::processTextMessage(QString textMessage) {
	QWebSocket *pSender = qobject_cast<QWebSocket *>(sender());
	if (pSender) {
		const char *msg = textMessage.toLocal8Bit();
		blog(LOG_INFO, "[obs-websockets] new message : %s", msg);

		obs_data_t *request = obs_data_create_from_json(msg);
		if (!request) {
			blog(LOG_ERROR, "[obs-websockets] invalid JSON payload for '%s'", msg);
		}

		const char *requestType = obs_data_get_string(request, "request");
		if (strcmp(requestType, "scene_change") == 0) {
			const char *sceneName = obs_data_get_string(request, "switch_to");

			blog(LOG_INFO, "[obs-websockets] processing scene change request to %s", sceneName);

			obs_source_t *source = obs_get_source_by_name(sceneName);

			if (source) {
				obs_frontend_set_current_scene(source);
			}
			else {
				blog(LOG_ERROR, "[obs-websockets] requested scene '%s' doesn't exist !", sceneName);
			}
			
		}

		obs_data_release(request);
	}
}

void WSServer::socketDisconnected()
{
	QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

	blog(LOG_INFO, "[obs-websockets] client %s:%d disconnected", pClient->peerAddress().toString().toStdString(), pClient->peerPort());

	if (pClient) {
		_clients.removeAll(pClient);
		pClient->deleteLater();
	}
}