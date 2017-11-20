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

#ifndef WSSERVER_H
#define WSSERVER_H

#include <QObject>
#include <QList>
#include <QMutex>

#include <wampconnection.h>

#include "WSRequestHandler.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class WSServer : public QObject {
  Q_OBJECT
  public:
    enum WampConnectionStatus {
        Disconnected = 0,
        Connecting = 1,
        Connected = 2,
        Error = 3
    };

    explicit WSServer(QObject* parent = Q_NULLPTR);
    virtual ~WSServer();
    void Start(quint16 port);
    void Stop();
    void Broadcast(const char* type, obs_data_t* message);

    void StopWamp();

    WampConnectionStatus GetWampStatus();
    QString GetWampErrorMessage();
    QString GetWampErrorUri();
    OBSDataAutoRelease GetWampErrorData();

    static WSServer* Instance;
	
  public slots:
    void StartWamp(QUrl url, QString realm, QString wampBaseUri, QString wampId, QString user = QString(), QString password = QString());

  private slots:
    void onNewConnection();
    void onTextMessageReceived(QString message);
    void onSocketDisconnected();

    void RegisterWampProcedures();
    void RegisterWamp();
    void CompleteWampRegistration(QVariant v);

    void onWampConnected();
    void onWampDisconnected();
    void onWampError(const WampError& error);

  private:
	

    QString WampTopic(QString value);
    QString WampProcedure(QString value);

    QWebSocketServer* _wsServer;
    QList<QWebSocket*> _clients;
    QMutex _clMutex;
    WampConnection* _wampConnection;
    WampConnectionStatus _wampStatus;
    QUrl _wampUrl;
    QString _wampId;
    QString _wampBaseUri;
    QString _wampRealm;
    QString _wampUser;
    QString _wampErrorUri;
    QString _wampErrorMessage;
    bool _canPublish;
    OBSDataAutoRelease _wampErrorData;
};

#endif // WSSERVER_H
