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

#include "WSRequestHandler.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class WSServer : public QObject {
  Q_OBJECT
  public:
    explicit WSServer(QObject* parent = Q_NULLPTR);
    virtual ~WSServer();
    void Start(quint16 port);
    void Stop();
    void broadcast(QString message);
    static WSServer* Instance;

  private slots:
    void onNewConnection();
    void onTextMessageReceived(QString message);
    void onSocketDisconnected();

  private:
    QWebSocketServer* _wsServer;
    QList<QWebSocket*> _clients;
    QMutex _clMutex;
};

#endif // WSSERVER_H