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

#include "HttpUtils.h"

#include <QtConcurrent/QtConcurrent>

#include "../obs-websocket.h"
#include "../Config.h"

void HttpUtils::wrapAsync(server::connection_ptr connection, std::function<void()> callback)
{
	connection->defer_http_response();
	QtConcurrent::run([connection, callback]() {
		callback();

		websocketpp::lib::error_code ec;
		connection->send_http_response(ec);
	});
}

std::string HttpUtils::handleIfAuthorized(server::connection_ptr con, std::function<std::string(ConnectionProperties&, std::string)> handlerCb)
{
	websocketpp::http::parser::request request = con->get_request();

	ConnectionProperties connProperties;
	if (GetConfig()->AuthRequired) {
		QString authHeaderValue = QString::fromStdString(
			request.get_header("Authorization")
		);

		if (GetConfig()->CheckHttpAuth(authHeaderValue)) {
			connProperties.setAuthenticated(true);
		} else {
			con->set_status(websocketpp::http::status_code::unauthorized);
			con->append_header("WWW-Authenticate", "Basic realm=\"obs-websocket\"");
			con->set_body("");
			return std::string();
		}
	}

	std::string requestBody = request.get_body();
	return handlerCb(connProperties, requestBody);
}
