/*
obs-websocket
Copyright (C) 2016	Stéphane Lepin <stephane.lepin@gmail.com>

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

#include "WSServer.h"
#include "WSRequestHandler.h"
#include "Config.h"
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QtCore/QDebug>
#include <QtCore/QThread>
#include <obs-frontend-api.h>

QT_USE_NAMESPACE

WSServer::WSServer(quint16 port, QObject *parent) :
	QObject(parent),
	_wsServer(Q_NULLPTR),
	_clients(),
	_clMutex(QMutex::NonRecursive)
{
	_serverThread = new QThread();

	_wsServer = new QWebSocketServer(
		QStringLiteral("obs-websocket"),
		QWebSocketServer::NonSecureMode,
		this);

	_wsServer->moveToThread(_serverThread);
	_serverThread->start();

	bool serverStarted = _wsServer->listen(QHostAddress::Any, port);
	if (serverStarted) {
		connect(_wsServer, &QWebSocketServer::newConnection, this, &WSServer::onNewConnection);
	}
}

WSServer::~WSServer()
{
	_wsServer->close();

	_clMutex.lock();
	qDeleteAll(_clients.begin(), _clients.end());
	_clMutex.unlock();
}

void WSServer::broadcast(QString message)
{
	_clMutex.lock();

	Q_FOREACH(WSRequestHandler *pClient, _clients) {
		if (Config::Current()->AuthRequired == true 
			&& pClient->isAuthenticated() == false) {
			// Skip this client if unauthenticated
			continue;
		}

		pClient->sendTextMessage(message);
	}

	_clMutex.unlock();
}

void WSServer::onNewConnection()
{
	QWebSocket *pSocket = _wsServer->nextPendingConnection();

	if (pSocket) {
		WSRequestHandler *pHandler = new WSRequestHandler(pSocket);
		connect(pHandler, &WSRequestHandler::disconnected, this, &WSServer::socketDisconnected);
		
		_clMutex.lock();
		_clients << pHandler;
		_clMutex.unlock();
	}
}

void WSServer::socketDisconnected()
{
	WSRequestHandler *pHandler = qobject_cast<WSRequestHandler *>(sender());

	if (pHandler) {
		_clMutex.lock();
		_clients.removeAll(pHandler);
		_clMutex.unlock();

		pHandler->deleteLater();
	}
}