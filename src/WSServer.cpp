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

#include <chrono>
#include <thread>

#include <QtCore/QThread>
#include <QtCore/QByteArray>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#include <QtConcurrent/QtConcurrent>
#include <obs-frontend-api.h>

#include "WSServer.h"
#include "obs-websocket.h"
#include "Config.h"
#include "Utils.h"

QT_USE_NAMESPACE

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

WSServerPtr WSServer::_instance = WSServerPtr(nullptr);

WSServerPtr WSServer::Current()
{
	if (!_instance) {
		ResetCurrent();
	}
	return _instance;
}

void WSServer::ResetCurrent()
{
	_instance = WSServerPtr(new WSServer());
}

WSServer::WSServer()
	: QObject(nullptr),
	  _connections(),
	  _clMutex(QMutex::Recursive)
{
	_server.init_asio();
	_server.set_reuse_addr(true);

	_server.set_open_handler(bind(&WSServer::onOpen, this, ::_1));
	_server.set_close_handler(bind(&WSServer::onClose, this, ::_1));
	_server.set_message_handler(bind(&WSServer::onMessage, this, ::_1, ::_2));
}

WSServer::~WSServer()
{
	stop();
}

void WSServer::start(quint16 port)
{
	if (_server.is_listening() && port == _serverPort) {
		blog(LOG_INFO, "WSServer::start: server already on this port. no restart needed");
		return;
	}

	if (_server.is_listening()) {
		stop();
	}

	_server.reset();

	_serverPort = port;

	websocketpp::lib::error_code errorCode;
	_server.listen(_serverPort, errorCode);

	if (errorCode) {
		std::string errorCodeMessage = errorCode.message();
		blog(LOG_INFO, "server: listen failed: %s", errorCodeMessage.c_str());

		obs_frontend_push_ui_translation(obs_module_get_string);
		QString errorTitle = tr("OBSWebsocket.Server.StartFailed.Title");
		QString errorMessage = tr("OBSWebsocket.Server.StartFailed.Message").arg(_serverPort);
		obs_frontend_pop_ui_translation();

		QMainWindow* mainWindow = reinterpret_cast<QMainWindow*>(obs_frontend_get_main_window());
		QMessageBox::warning(mainWindow, errorTitle, errorMessage);

		return;
	}

	_server.start_accept();

	QtConcurrent::run([=]() {
		blog(LOG_INFO, "io thread started");
		_server.run();
		blog(LOG_INFO, "io thread exited");
	});

	blog(LOG_INFO, "server started successfully on port %d", _serverPort);
}

void WSServer::stop()
{
	if (!_server.is_listening()) {
		return;
	}

	_server.stop_listening();
	for (connection_hdl hdl : _connections) {
		_server.close(hdl, websocketpp::close::status::going_away, "Server stopping");
	}
	_connections.clear();
	_connectionProperties.clear();
	
	while (!_server.stopped()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	blog(LOG_INFO, "server stopped successfully");
}

void WSServer::broadcast(std::string message)
{
	QMutexLocker locker(&_clMutex);
	for (connection_hdl hdl : _connections) {
		if (Config::Current()->AuthRequired) {
			bool authenticated = _connectionProperties[hdl].value(PROP_AUTHENTICATED).toBool();
			if (!authenticated) {
				continue;
			}
		}
		_server.send(hdl, message, websocketpp::frame::opcode::text);
	}
}

void WSServer::onOpen(connection_hdl hdl)
{
	QMutexLocker locker(&_clMutex);
	_connections.insert(hdl);
	locker.unlock();

	QString clientIp = getRemoteEndpoint(hdl);
	notifyConnection(clientIp);
	blog(LOG_INFO, "new client connection from %s", clientIp.toUtf8().constData());
}

void WSServer::onMessage(connection_hdl hdl, server::message_ptr message)
{
	auto opcode = message->get_opcode();
	if (opcode != websocketpp::frame::opcode::text) {
		return;
	}

	std::string payload = message->get_payload();

	QMutexLocker locker(&_clMutex);
	QVariantHash connProperties = _connectionProperties[hdl];
	locker.unlock();

	WSRequestHandler handler(connProperties);
	std::string response = handler.processIncomingMessage(payload);

	_server.send(hdl, response, websocketpp::frame::opcode::text);

	locker.relock();
	// In multithreaded processing this would be problematic to put back
	// a copy of the connection properties, because there might conflicts
	// between several simultaneous handlers.
	// In our case, it's fine because all messages are processed in one thread.
	_connectionProperties[hdl] = connProperties;
	locker.unlock();
}

void WSServer::onClose(connection_hdl hdl)
{
	QMutexLocker locker(&_clMutex);
	_connections.erase(hdl);
	_connectionProperties.erase(hdl);
	locker.unlock();

	QString clientIp = getRemoteEndpoint(hdl);
	notifyDisconnection(clientIp);
	blog(LOG_INFO, "client %s disconnected", clientIp.toUtf8().constData());
}

QString WSServer::getRemoteEndpoint(connection_hdl hdl)
{
	auto conn = _server.get_con_from_hdl(hdl);
	return QString::fromStdString(conn->get_remote_endpoint());
}

void WSServer::notifyConnection(QString clientIp)
{
	obs_frontend_push_ui_translation(obs_module_get_string);
	QString title = tr("OBSWebsocket.NotifyConnect.Title");
	QString msg = tr("OBSWebsocket.NotifyConnect.Message").arg(clientIp);
	obs_frontend_pop_ui_translation();

	Utils::SysTrayNotify(msg, QSystemTrayIcon::Information, title);
}

void WSServer::notifyDisconnection(QString clientIp)
{
	obs_frontend_push_ui_translation(obs_module_get_string);
	QString title = tr("OBSWebsocket.NotifyDisconnect.Title");
	QString msg = tr("OBSWebsocket.NotifyDisconnect.Message").arg(clientIp);
	obs_frontend_pop_ui_translation();

	Utils::SysTrayNotify(msg, QSystemTrayIcon::Information, title);
}
