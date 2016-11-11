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
	Q_FOREACH(WSRequestHandler *pClient, _clients) {
		pClient->sendTextMessage(message);
	}
}

void WSServer::onNewConnection()
{
	QWebSocket *pSocket = _wsServer->nextPendingConnection();

	if (pSocket) {
		WSRequestHandler *pHandler = new WSRequestHandler(pSocket);

		connect(pHandler, &WSRequestHandler::disconnected, this, &WSServer::socketDisconnected);
		_clients << pHandler;
	}
}

void WSServer::socketDisconnected()
{
	WSRequestHandler *pClient = qobject_cast<WSRequestHandler *>(sender());

	if (pClient) {
		_clients.removeAll(pClient);
		pClient->deleteLater();
	}
}