#include "WSServer.h"
#include "WSRequestHandler.h"
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

		WSRequestHandler *handler = new WSRequestHandler(pSender);
		handler->handleMessage(msg);
		delete handler;
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