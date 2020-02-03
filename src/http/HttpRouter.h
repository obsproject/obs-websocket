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

#pragma once

#include <functional>
#include <QtCore/QList>

#include "../server-defs.h"

#define ROUTER_ENTRY(method, route, handler) { method, route, handler }

namespace http 
{
    enum Method {
        UnknownMethod = 0,
        AnyMethod,
        Get,
        Options,
        Head,
        Post,
        Put,
        Delete
    };
}

class HttpRouter {
public:
    typedef std::function<std::string(server::connection_ptr)> RouteHandler;

    typedef struct {
        http::Method method;
        QString spec;
        RouteHandler routeCallback;
    } RouterEntry;

    explicit HttpRouter(QList<RouterEntry> routes);
    bool handleConnection(server::connection_ptr connection);

private:
    bool routeMatchesUri(const QString& routeSpec, const QString& requestUri);

    QList<RouterEntry> _routes;
};
