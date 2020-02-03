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

#include "HttpRouter.h"

#include <QtConcurrent/QtConcurrent>
#include <QtCore/QHash>
#include <QtCore/QMultiHash>

#include "../Config.h"
#include "HttpUtils.h"

static const QHash<QString, http::Method> methodMap {
	{ "GET", http::Method::Get },
	{ "OPTIONS", http::Method::Options },
	{ "HEAD", http::Method::Head },
	{ "POST", http::Method::Post },
	{ "PUT", http::Method::Put },
	{ "DELETE", http::Method::Delete }
};

http::Method methodStringToEnum(const QString& method) {
	return methodMap.value(method, http::Method::UnknownMethod);
}

HttpRouter::HttpRouter(QList<RouterEntry> routes) :
	_routes(routes)
{
}

bool HttpRouter::handleConnection(server::connection_ptr connection)
{
	websocketpp::config::asio::request_type httpRequest = connection->get_request();

	QString requestUri = QString::fromStdString(
		connection->get_uri()->get_resource()
	);

	QString methodString = QString::fromStdString(
		httpRequest.get_method()
	);
	http::Method requestMethod = methodStringToEnum(methodString);

	for (RouterEntry route : _routes) {
		if (
			routeMatchesUri(route.spec, requestUri) &&
			(route.method == http::Method::AnyMethod || requestMethod == route.method)
		) {
			HttpUtils::wrapAsync(connection, [connection, route]() {
				// can get overriden by the handler callback
				connection->set_status(websocketpp::http::status_code::ok);

				std::string responseBody = route.routeCallback(connection);
				if (!responseBody.empty()) {
					connection->set_body(responseBody);
				}
			});
			return true;
		}
	}

	return false;
}

QString normalizeUri(const QString& uri)
{
	QString normalized = uri.trimmed();
	if (!normalized.startsWith("/")) {
		normalized = normalized.prepend("/");
	}
	if (normalized.endsWith("/")) {
		normalized.chop(1);
	}
	return normalized;
}

bool HttpRouter::routeMatchesUri(const QString& routeSpec, const QString& requestUri)
{
	QString spec = normalizeUri(routeSpec);
	QString uri = normalizeUri(requestUri);

	// TODO radix tree lookup
	return uri.startsWith(routeSpec);
}
