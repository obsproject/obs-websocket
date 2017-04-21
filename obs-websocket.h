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

#ifndef OBSWEBSOCKET_H
#define OBSWEBSOCKET_H

#define PROP_AUTHENTICATED "wsclient_authenticated"
#define OBS_WEBSOCKET_VERSION "4.1.0"
#define API_VERSION 1.3

#define blog(level, msg, ...) blog(level, "[obs-websocket] " msg, ##__VA_ARGS__)

#endif // OBSWEBSOCKET_H