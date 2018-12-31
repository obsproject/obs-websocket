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
#include <QMutex>

#include <set>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "WSRequestHandler.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

using websocketpp::connection_hdl;

typedef websocketpp::server<websocketpp::config::asio> server;
typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;

class WSServer : public QObject
{
Q_OBJECT

public:
	explicit WSServer(QObject* parent = Q_NULLPTR);
	virtual ~WSServer();
	void start(quint16 port);
	void stop();
	void broadcast(QString message);
	static WSServer* Instance;

private:
	bool validateConnection(connection_hdl hdl);
	void onOpen(connection_hdl hdl);
	void onMessage(connection_hdl hdl, server::message_ptr message);
	void onClose(connection_hdl hdl);

	QString getRemoteEndpoint(connection_hdl hdl);
	void notifyConnection(QString clientIp);
	void notifyDisconnection(QString clientIp);

	server _server;
	quint16 _serverPort;
	con_list _connections;
	QMutex _clMutex;
};

#endif // WSSERVER_H
