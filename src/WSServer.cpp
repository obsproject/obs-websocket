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
#include <QMainWindow>
#include <QMessageBox>
#include <obs-frontend-api.h>

#include "WSServer.h"
#include "obs-websocket.h"
#include "Config.h"
#include "Utils.h"

#define WAMP_FUNCTION_PREFIX ".f."
#define WAMP_TOPIC_PREFIX ".t."

#include <wampcrauser.h>

QT_USE_NAMESPACE

WSServer* WSServer::Instance = nullptr;

WSServer::WSServer(QObject* parent)
    : QObject(parent),
      _wsServer(Q_NULLPTR),
      _clients(),
      _clMutex(QMutex::Recursive),
      _wampConnection(nullptr),
      _wampStatus(WampConnectionStatus::Disconnected),
      _wampUrl(QUrl()),
      _wampId(QString()),
      _wampBaseUri(QString()),
      _wampRealm(QString()),
      _wampUser(QString()),
      _wampErrorUri(QString()),
      _wampErrorMessage(QString()),
      _canPublish(true)
{
    _wsServer = new QWebSocketServer(
        QStringLiteral("obs-websocket"),
        QWebSocketServer::NonSecureMode, this);
    
}

WSServer::~WSServer() {
    Stop();
    StopWamp();
}

void WSServer::Start(quint16 port) {
    if (port == _wsServer->serverPort())
        return;

    if(_wsServer->isListening())
        Stop();

    bool serverStarted = _wsServer->listen(QHostAddress::Any, port);
    if (serverStarted) {
        blog(LOG_INFO, "server started successfully on TCP port %d", port);

        connect(_wsServer, SIGNAL(newConnection()),
            this, SLOT(onNewConnection()));
    }
    else {
        QString errorString = _wsServer->errorString();
        blog(LOG_ERROR,
            "error: failed to start server on TCP port %d: %s",
            port, errorString.toUtf8().constData());

        QMainWindow* mainWindow = (QMainWindow*)obs_frontend_get_main_window();

        obs_frontend_push_ui_translation(obs_module_get_string);
        QString title = tr("OBSWebsocket.Server.StartFailed.Title");
        QString msg = tr("OBSWebsocket.Server.StartFailed.Message").arg(port);
        obs_frontend_pop_ui_translation();

        QMessageBox::warning(mainWindow, title, msg);
    }
}

void WSServer::Stop() {
    _clMutex.lock();
    for(QWebSocket* pClient : _clients) {
        pClient->close();
    }
    _clMutex.unlock();

    _wsServer->close();

    blog(LOG_INFO, "server stopped successfully");
}

void WSServer::StartWamp(QUrl url, QString realm, QString baseUri, QString id, QString user, QString password)
{
    if (url == _wampUrl && realm == _wampRealm) {
        _wampId = id;
        _wampBaseUri = baseUri;
        if (!user.isEmpty()) {
            _wampConnection->setUser(new WampCraUser(user, password));
        }
        return;
    }
    if (_wampConnection) {
        disconnect(_wampConnection, &WampConnection::disconnected,
            this, &WSServer::onWampDisconnected); //ignore the disconnected event for the other socket
        delete _wampConnection;
    }
    _wampConnection = new WampConnection(this);
    _wampConnection->setUrl(url);
    _wampConnection->setRealm(realm);

    if (!user.isEmpty()) {
        _wampConnection->setUser(new WampCraUser(user, password));
    }

    _wampId = id;
    _wampBaseUri = baseUri;
    _wampRealm = realm;
    _wampUrl = url;
    _wampUser = user;

    qInfo() << "wamp " << _wampUrl << " connecting";

    connect(_wampConnection, &WampConnection::connected,
            this, &WSServer::onWampConnected);
    connect(_wampConnection, &WampConnection::disconnected,
            this, &WSServer::onWampDisconnected);
    connect(_wampConnection, &WampConnection::error,
            this, &WSServer::onWampError);

    _wampConnection->connect();
}

void WSServer::StopWamp()
{
    delete _wampConnection;
    _wampConnection = nullptr;
    _wampId = QString();
    _wampRealm = QString();
    _wampUrl = QString();
    _wampUser = QString();
}


void WSServer::onWampConnected()
{
    _wampStatus = WampConnectionStatus::Connected;
    
    qInfo() << "wamp " << _wampUrl << " connected";
    
    obs_frontend_push_ui_translation(obs_module_get_string);
    QString title = tr("OBSWebsocket.WampNotifyConnected.Title");
    QString msg = tr("OBSWebsocket.WampNotifyConnected.Message")
    .arg(_wampUrl.toString());
    
    obs_frontend_pop_ui_translation();
    
    Utils::WampSysTrayNotify(msg, QSystemTrayIcon::Information, title);

    QVariantList args = QVariantList();
    if (!_wampId.isEmpty())
        args.append(_wampId);
    
    RegisterWamp();
    
}

void WSServer::onWampDisconnected()
{
    
    
    _wampStatus = WampConnectionStatus::Disconnected;
    
    qInfo() << "wamp " << _wampUrl << " disconnected";
    
    obs_frontend_push_ui_translation(obs_module_get_string);
    QString title = tr("OBSWebsocket.WampNotifyDisconnected.Title");
    QString msg = tr("OBSWebsocket.WampNotifyConnected.Message")
        .arg(_wampUrl.toString().toUtf8().constData());
    
    obs_frontend_pop_ui_translation();
    
    Utils::WampSysTrayNotify(msg, QSystemTrayIcon::Information, title);
}

void WSServer::onWampError(const WampError& error)
{
    //fall back to anonymous if the connection attempt fails and a user was specified and we are configured to fallback
    if ((_wampStatus == WampConnectionStatus::Connecting || _wampStatus == WampConnectionStatus::Disconnected) && !_wampUrl.isEmpty() && Config::Current()->WampAnonymousFallback && !_wampConnection->user()->name().isEmpty()) {
        QUrl url = _wampUrl; // to force the reconnect
        _wampUrl = QString();
        QMetaObject::invokeMethod(this, "StartWamp", Qt::QueuedConnection,
              Q_ARG(QUrl, url),
              Q_ARG(QString, _wampRealm),
              Q_ARG(QString, _wampBaseUri),
              Q_ARG(QString, _wampId));
        return;
    }
    
    _wampStatus = WampConnectionStatus::Error;
    _wampErrorUri = error.uri().toString();
    
    QVariantList args = error.args();
    if (!args.isEmpty())
        _wampErrorMessage = args.first().toString();
    else
        _wampErrorMessage = _wampErrorUri;
    
    QVariantMap details = error.details();
    if (!details.isEmpty())
    {
        _wampErrorData = Utils::DataFromMap(details);
    }
    else
    {
        obs_data_clear(_wampErrorData);
    }
    
    blog(LOG_INFO, "wamp %s error %s (%s)",
         _wampUrl.toString().toUtf8().constData(),
         _wampErrorMessage.toUtf8().constData(),
         _wampErrorUri.toUtf8().constData());
    
    obs_frontend_push_ui_translation(obs_module_get_string);
    QString title = tr("OBSWebsocket.WampNotifyError.Title");
    QString msg = tr("OBSWebsocket.WampNotifyError.Message")
         .arg(_wampUrl.toString())
         .arg(_wampErrorMessage)
         .arg(_wampErrorUri);
    
    obs_frontend_pop_ui_translation();
    
    Utils::WampSysTrayNotify(msg, QSystemTrayIcon::Information, title);
}

QString WSServer::WampTopic(QString value)
{
    QString topic = Utils::WampUrlFix(value).prepend(WAMP_TOPIC_PREFIX).prepend(_wampBaseUri);
    if (!_wampId.isEmpty())
        topic = topic.append('.').append(_wampId);
    return topic;
}

QString WSServer::WampProcedure(QString value)
{
    QString topic = Utils::WampUrlFix(value)
        .prepend(WAMP_FUNCTION_PREFIX)
        .prepend(_wampBaseUri);
    
    if (!_wampId.isEmpty())
        topic = topic.append('.').append(_wampId);
    
    return topic;
}

void WSServer::Broadcast(const char* type, obs_data_t* message) {
    
    if (_wampStatus == WampConnectionStatus::Connected && _canPublish)
    {
        QString topic = WampTopic(QString(type));
        QVariantList list = Utils::ListFromData(message);
        _wampConnection->publish(topic, list);
    }
    
    obs_data_set_string(message, "update-type", type);
    QString json = obs_data_get_json(message);
    
    _clMutex.lock();
    for(QWebSocket* pClient : _clients) {
        if (Config::Current()->AuthRequired
            && (pClient->property(PROP_AUTHENTICATED).toBool() == false)) {
            // Skip this client if unauthenticated
            continue;
        }
        
        pClient->sendTextMessage(json);
    }
    _clMutex.unlock();
    
    if (Config::Current()->DebugEnabled)
        blog(LOG_DEBUG, "Update << '%s'", json.toUtf8().constData());
    
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

        obs_frontend_push_ui_translation(obs_module_get_string);
        QString title = tr("OBSWebsocket.NotifyConnect.Title");
        QString msg = tr("OBSWebsocket.NotifyConnect.Message")
            .arg(Utils::FormatIPAddress(clientAddr));
        obs_frontend_pop_ui_translation();

        Utils::SysTrayNotify(msg, QSystemTrayIcon::Information, title);
    }
}

void WSServer::onTextMessageReceived(QString message) {
    QWebSocket* pSocket = qobject_cast<QWebSocket*>(sender());
    if (pSocket) {
        WSWebSocketRequestHandler handler(pSocket);
        handler.processIncomingMessage(message);
    }
}

void WSServer::onSocketDisconnected() {
    QWebSocket* pSocket = qobject_cast<QWebSocket*>(sender());
    if (pSocket) {
        pSocket->setProperty(PROP_AUTHENTICATED, false);

        _clMutex.lock();
        _clients.removeAll(pSocket);
        _clMutex.unlock();;

        pSocket->deleteLater();

        QHostAddress clientAddr = pSocket->peerAddress();
        QString clientIp = Utils::FormatIPAddress(clientAddr);

        blog(LOG_INFO, "client %s:%d disconnected",
            clientIp.toUtf8().constData(), pSocket->peerPort());

        obs_frontend_push_ui_translation(obs_module_get_string);
        QString title = tr("OBSWebsocket.NotifyDisconnect.Title");
        QString msg = tr("OBSWebsocket.NotifyDisconnect.Message")
            .arg(Utils::FormatIPAddress(clientAddr));
        obs_frontend_pop_ui_translation();

        Utils::SysTrayNotify(msg, QSystemTrayIcon::Information, title);
    }
}

void WSServer::RegisterWamp()
{
    QString regProc = Config::Current()->WampRegisterProcedure;
    if (!regProc.isEmpty())
    {
        qInfo() << "Calling WAMP registration procedure: " << regProc;
        
        QVariantMap r {
            {"id" , _wampId},
            {"user", _wampUser},
            {"url", _wampUrl},
            {"realm", _wampRealm},
            {"baseuri", _wampBaseUri}
        };
        QVariantList args { r };
        QVariantMap options {
            {"disclose_me", true }
        };
        _wampConnection->call2(regProc, args, [this](QVariant v) {
            this->CompleteWampRegistration(v);
        }, options);
    }
    else
    {
        QMetaObject::invokeMethod(this, "RegisterWampProcedures", Qt::QueuedConnection);
    }
}

void WSServer::CompleteWampRegistration(QVariant v)
{
    Config* config = Config::Current();
    qInfo() << "Received registration response from WAMP server";
    
    QVariantMap r;
    if (v.type() == QVariant::Map) {
        r = v.toMap();
    } else if (v.type() == QVariant::List) {
        r = v.toList()[0].toMap();
    }
    QString id = r["id"].toString();
    QString user = r["user"].toString();
    QString secret = r["secret"].toString();
    if (secret.isEmpty())
        secret = r["password"].toString();
    QUrl url = r["url"].toUrl();
    QString realm = r["realm"].toString();
    QString baseuri = r["baseuri"].toString();
    
    bool save = r["save"].toBool();
    bool reconnect = r["reconnect"].toBool();
    bool canRegister = r.contains("register") ? r["register"].toBool() : true;
    _canPublish = r.contains("publish") ? r["publish"].toBool() : true;
    
    if (!id.isEmpty() && id != config->WampId)
        config->WampId = id;
    if (!baseuri.isEmpty() && baseuri != config->WampBaseUri)
        config->WampBaseUri = baseuri;
    if (!user.isEmpty() && user != config->WampUser)
        config->WampUser = user;
    if (!secret.isEmpty() && secret != config->WampPassword) {
        config->WampPassword = secret;
        _wampConnection->setUser(new WampCraUser(config->WampUser, config->WampPassword));  //update on the current conncetion in the case of a reconnection
    }
    if (!url.isEmpty() && url != config->WampUrl) {
        config->WampUrl = url;
        _wampConnection->setUrl(url);
    }
    if (!realm.isEmpty() && realm != config->WampRealm) {
        config->WampRealm = realm;
        _wampConnection->setRealm(realm);
    }
    
    if (save)
    {
        config->Save();
    }
    
    if (reconnect)
    {
        qInfo() << "WAMP reg response indicated need to reconnect with alternative information. Reconnecting...";

        _wampUrl = QString(); //to force reconnection
        QMetaObject::invokeMethod(WSServer::Instance, "StartWamp",
          Q_ARG(QUrl, config->WampUrl),
          Q_ARG(QString, config->WampRealm),
          Q_ARG(QString, config->WampBaseUri),
          Q_ARG(QString, config->WampIdEnabled?config->WampId:""),
          Q_ARG(QString, config->WampAuthEnabled?config->WampUser:""),
          Q_ARG(QString, config->WampAuthEnabled?config->WampPassword:""));
    }
    else if (canRegister)
    {
        QMetaObject::invokeMethod(this, "RegisterWampProcedures", Qt::QueuedConnection);
    }
}

void WSServer::RegisterWampProcedures()
{
    qDebug() << "Registering wamp procedures";
    
    for (QHash<QString,void(*)(WSRequestHandler*)>::iterator i = WSRequestHandler::messageMap.begin();i != WSRequestHandler::messageMap.end();i++) {
        QString procedure = WampProcedure(i.key());
        void(*handlerMethod)(WSRequestHandler*) = i.value();
        _wampConnection->registerProcedure(procedure, [handlerMethod, this](QVariantList args) {
            WSWampRequestHandler handler(handlerMethod);
            return handler.processIncomingMessage(args);
        });
    }
}

WSServer::WampConnectionStatus WSServer::GetWampStatus()
{
    return _wampStatus;
}

QString WSServer::GetWampErrorMessage()
{
    return _wampErrorMessage;
}

QString WSServer::GetWampErrorUri()
{
    return _wampErrorUri;
}

OBSDataAutoRelease WSServer::GetWampErrorData()
{
    return _wampErrorData;
}
