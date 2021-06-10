/*
obs-websocket
Copyright (C) 2016-2019	Stéphane Lepin <stephane.lepin@gmail.com>

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

#pragma once

#include <map>
#include <set>
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtCore/QVariantHash>
#include <QtCore/QThreadPool>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "ConnectionProperties.h"
#include "WSRequestHandler.h"
#include "rpc/RpcEvent.h"

using websocketpp::connection_hdl;

typedef websocketpp::server<websocketpp::config::asio> server;

class WSServer : public QObject
{
Q_OBJECT

public:
	explicit WSServer();
	virtual ~WSServer();
	void start(quint16 port, bool lockToIPv4);
	void stop();
	void broadcast(const RpcEvent& event);
	bool isListening();
	QThreadPool* threadPool() {
		return &_threadPool;
	}

private:
	void serverRunner();

	void onOpen(connection_hdl hdl);
	void onMessage(connection_hdl hdl, server::message_ptr message);
	void onClose(connection_hdl hdl);

	QString getRemoteEndpoint(connection_hdl hdl);
	void notifyConnection(QString clientIp);
	void notifyDisconnection(QString clientIp);

	std::thread _serverThread;
	server _server;
	quint16 _serverPort;
	bool _lockToIPv4;
	std::set<connection_hdl, std::owner_less<connection_hdl>> _connections;
	std::map<connection_hdl, ConnectionProperties, std::owner_less<connection_hdl>> _connectionProperties;
	QMutex _clMutex;
	QThreadPool _threadPool;
};
