/*
obs-websocket
Copyright (C) 2020-2023 Kyle Manning <tt2468@gmail.com>

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
#include <string>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <obs.h>

#include "../lib/obs-websocket-api.h"

class WebSocketApi {
public:
	enum RequestReturnCode {
		Normal,
		NoVendor,
		NoVendorRequest,
	};

	typedef std::function<void(std::string, std::string, obs_data_t *)> EventCallback;

	struct Vendor {
		std::shared_mutex _mutex;
		std::string _name;
		std::map<std::string, obs_websocket_request_callback> _requests;
	};

	WebSocketApi();
	~WebSocketApi();

	void SetEventCallback(EventCallback cb);

	enum RequestReturnCode PerformVendorRequest(std::string vendorName, std::string requestName, obs_data_t *requestData,
						    obs_data_t *responseData);

	static void get_ph_cb(void *priv_data, calldata_t *cd);
	static void get_api_version(void *, calldata_t *cd);
	static void call_request(void *, calldata_t *cd);
	static void vendor_register_cb(void *priv_data, calldata_t *cd);
	static void vendor_request_register_cb(void *priv_data, calldata_t *cd);
	static void vendor_request_unregister_cb(void *priv_data, calldata_t *cd);
	static void vendor_event_emit_cb(void *priv_data, calldata_t *cd);

private:
	std::shared_mutex _mutex;
	EventCallback _eventCallback;
	proc_handler_t *_procHandler;
	std::map<std::string, Vendor *> _vendors;
};
