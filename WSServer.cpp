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

WSServer::WSServer(QObject* parent) :
    QObject(parent),
    _wsServer(Q_NULLPTR),
    _clients(),
    _clMutex(QMutex::Recursive) {
    _serverThread = new QThread();
    _wsServer = new QWebSocketServer(
        QStringLiteral("obs-websocket"),
        QWebSocketServer::NonSecureMode,
        _serverThread);
    _serverThread->start();
}

WSServer::~WSServer() {
    Stop();
    delete _serverThread;
}

void WSServer::Start(quint16 port) {
    if (port == _wsServer->serverPort())
        return;

    if(_wsServer->isListening())
        Stop();

    bool serverStarted = _wsServer->listen(QHostAddress::Any, port);
    if (serverStarted) {
        connect(_wsServer, SIGNAL(newConnection()),
            this, SLOT(onNewConnection()));
    }
}

void WSServer::Stop() {
    _clMutex.lock();
    for(QWebSocket* pClient : _clients) {
        pClient->close();
    }
    _clMutex.unlock();

    _wsServer->close();
}

void WSServer::broadcast(QString message) {
    _clMutex.lock();
    for(QWebSocket* pClient : _clients) {
        if (Config::Current()->AuthRequired
            && (pClient->property(PROP_AUTHENTICATED).toBool() == false)) {
            // Skip this client if unauthenticated
            continue;
        }
        pClient->sendTextMessage(message);
    }
    _clMutex.unlock();
}

void WSServer::onNewConnection() {
    QWebSocket* pSocket = _wsServer->nextPendingConnection();
    if (pSocket) {
        connect(pSocket, SIGNAL(textMessageReceived(const QString&)),
            this, SLOT(onTextMessageReceived(QString)));
        connect(pSocket, SIGNAL(disconnected()),
            this, SLOT(onSocketDisconnected()));

        pSocket->setProperty(PROP_AUTHENTICATED, false);

        _clMutex.lock();
        _clients << pSocket;
        _clMutex.unlock();

        QHostAddress clientAddr = pSocket->peerAddress();
        QString clientIp = Utils::FormatIPAddress(clientAddr);

        blog(LOG_INFO, "new client connection from %s:%d",
            clientIp.toUtf8().constData(), pSocket->peerPort());

        QString msg = QString(obs_module_text("OBSWebsocket.ConnectNotify.ClientIP"))
            + QString(" ")
            + clientAddr.toString();

        Utils::SysTrayNotify(msg,
            QSystemTrayIcon::Information,
            QString(obs_module_text("OBSWebsocket.ConnectNotify.Connected")));
    }
}

void WSServer::onTextMessageReceived(QString message) {
    QWebSocket* pSocket = qobject_cast<QWebSocket*>(sender());
    if (pSocket) {
        WSRequestHandler handler(pSocket);
        handler.processIncomingMessage(message);
    }
}

void WSServer::onSocketDisconnected() {
    QWebSocket* pSocket = qobject_cast<QWebSocket*>(sender());
    if (pSocket) {
        pSocket->setProperty(PROP_AUTHENTICATED, false);

        _clMutex.lock();
        _clients.removeAll(pSocket);
        _clMutex.unlock();

        pSocket->deleteLater();

        QHostAddress clientAddr = pSocket->peerAddress();
        QString clientIp = Utils::FormatIPAddress(clientAddr);

        blog(LOG_INFO, "client %s:%d disconnected",
            clientIp.toUtf8().constData(), pSocket->peerPort());

        QString msg = QString(obs_module_text("OBSWebsocket.ConnectNotify.ClientIP"))
            + QString(" ")
            + clientAddr.toString();

        Utils::SysTrayNotify(msg,
            QSystemTrayIcon::Information,
            QString(obs_module_text("OBSWebsocket.ConnectNotify.Disconnected")));
    }
}
