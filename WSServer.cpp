/*
obs-websocket
Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>

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

#include <QtWebSockets/QWebSocket>
#include <QtCore/QThread>
#include <QtCore/QByteArray>
#include <obs-frontend-api.h>

#include "WSServer.h"
#include "obs-websocket.h"
#include "Config.h"
#include "Utils.h"

QT_USE_NAMESPACE

WSServer* WSServer::Instance = nullptr;

WSServer::WSServer(QObject *parent) :
	QObject(parent),
	_wsServer(Q_NULLPTR),
	_clients(),
	_clMutex(QMutex::Recursive)
{
	_serverThread = new QThread();

	_wsServer = new QWebSocketServer(
		QStringLiteral("obs-websocket"),
		QWebSocketServer::NonSecureMode,
		_serverThread);

	_serverThread->start();
}

WSServer::~WSServer()
{
	Stop();

	delete _serverThread;
}

void WSServer::Start(quint16 port)
{
	if (port == _wsServer->serverPort())
		return;

	if(_wsServer->isListening())
		Stop();

	bool serverStarted = _wsServer->listen(QHostAddress::Any, port);
	if (serverStarted)
	{
		connect(_wsServer, &QWebSocketServer::newConnection, this, &WSServer::onNewConnection);
	}
}

void WSServer::Stop()
{
	_clMutex.lock();
	Q_FOREACH(QWebSocket *pClient, _clients)
	{
		pClient->close();
	}
	_clMutex.unlock();

	_wsServer->close();
}

void WSServer::broadcast(QString message)
{
	_clMutex.lock();

	Q_FOREACH(QWebSocket *pClient, _clients)
	{
		if (Config::Current()->AuthRequired
			&& (pClient->property(PROP_AUTHENTICATED).toBool() == false))
		{
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

	if (pSocket)
	{
		connect(pSocket, &QWebSocket::textMessageReceived, this, &WSServer::textMessageReceived);
		connect(pSocket, &QWebSocket::disconnected, this, &WSServer::socketDisconnected);
		pSocket->setProperty(PROP_AUTHENTICATED, false);

		_clMutex.lock();
		_clients << pSocket;
		_clMutex.unlock();

		QByteArray client_ip = pSocket->peerAddress().toString().toUtf8();
		blog(LOG_INFO, "new client connection from %s:%d", client_ip.constData(), pSocket->peerPort());

		QString msg = QString("Client IP: ") + pSocket->peerAddress().toString();
		Utils::SysTrayNotify(msg, QSystemTrayIcon::Information, QString("New WebSocket connection"));
	}
}

void WSServer::textMessageReceived(QString message) 
{
	QWebSocket *pSocket = qobject_cast<QWebSocket *>(sender());

	if (pSocket)
	{
		WSRequestHandler handler(pSocket);
		handler.processIncomingMessage(message);
	}
}

void WSServer::socketDisconnected()
{
	QWebSocket *pSocket = qobject_cast<QWebSocket *>(sender());

	if (pSocket)
	{
		pSocket->setProperty(PROP_AUTHENTICATED, false);

		_clMutex.lock();
		_clients.removeAll(pSocket);
		_clMutex.unlock();
		
		pSocket->deleteLater();

		QByteArray client_ip = pSocket->peerAddress().toString().toUtf8();
		blog(LOG_INFO, "client %s:%d disconnected", client_ip.constData(), pSocket->peerPort());
	}
}
