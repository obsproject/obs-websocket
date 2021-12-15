/*
obs-websocket
Copyright (C) 2016-2021 Stephane Lepin <stephane.lepin@gmail.com>
Copyright (C) 2020-2021 Kyle Manning <tt2468@gmail.com>

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

#include <QAction>
#include <QMainWindow>
#include <obs-module.h>
#include <obs-frontend-api.h>

#include "obs-websocket.h"
#include "Config.h"
#include "WebSocketApi.h"
#include "websocketserver/WebSocketServer.h"
#include "eventhandler/EventHandler.h"
#include "forms/SettingsDialog.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-websocket", "en-US")
OBS_MODULE_AUTHOR("OBSProject")
const char *obs_module_name(void) { return "obs-websocket"; }
const char *obs_module_description(void) { return obs_module_text("OBSWebSocket.Plugin.Description"); }

ConfigPtr _config;
WebSocketApiPtr _webSocketApi;
WebSocketServerPtr _webSocketServer;
EventHandlerPtr _eventHandler;
SettingsDialog *_settingsDialog = nullptr;
os_cpu_usage_info_t* _cpuUsageInfo;

void WebSocketApiEventCallback(std::string vendorName, std::string eventType, obs_data_t *obsEventData);

bool obs_module_load(void)
{
	blog(LOG_INFO, "[obs_module_load] you can haz websockets (Version: %s | RPC Version: %d)", OBS_WEBSOCKET_VERSION, OBS_WEBSOCKET_RPC_VERSION);
	blog(LOG_INFO, "[obs_module_load] Qt version (compile-time): %s | Qt version (run-time): %s", QT_VERSION_STR, qVersion());

	// Create the config object then load the parameters from storage
	_config = ConfigPtr(new Config());
	_config->Load();

	// Initialize event handler before server, as the server configures the event handler.
	_eventHandler = EventHandlerPtr(new EventHandler());

	_webSocketApi = WebSocketApiPtr(new WebSocketApi(WebSocketApiEventCallback));

	_webSocketServer = WebSocketServerPtr(new WebSocketServer());

	obs_frontend_push_ui_translation(obs_module_get_string);
	QMainWindow* mainWindow = reinterpret_cast<QMainWindow*>(obs_frontend_get_main_window());
	_settingsDialog = new SettingsDialog(mainWindow);
	obs_frontend_pop_ui_translation();

	const char* menuActionText = obs_module_text("OBSWebSocket.Settings.DialogTitle");
	QAction* menuAction = (QAction*)obs_frontend_add_tools_menu_qaction(menuActionText);
	QObject::connect(menuAction, &QAction::triggered, [] { _settingsDialog->ToggleShowHide(); });

	_cpuUsageInfo = os_cpu_usage_info_start();

	// Loading finished
	blog(LOG_INFO, "[obs_module_load] Module loaded.");

	return true;
}

void obs_module_unload()
{
	blog(LOG_INFO, "[obs_module_unload] Shutting down...");

	if (_webSocketServer->IsListening()) {
		blog_debug("[obs_module_unload] WebSocket server is running. Stopping...");
		_webSocketServer->Stop();
	}
	_webSocketServer.reset();

	_eventHandler.reset();

	_webSocketApi.reset();

	_config->Save();
	_config.reset();

	os_cpu_usage_info_destroy(_cpuUsageInfo);

	blog(LOG_INFO, "[obs_module_unload] Finished shutting down.");
}

ConfigPtr GetConfig()
{
	return _config;
}

WebSocketApiPtr GetWebSocketApi()
{
	return _webSocketApi;
}

WebSocketServerPtr GetWebSocketServer()
{
	return _webSocketServer;
}

EventHandlerPtr GetEventHandler()
{
	return _eventHandler;
}

os_cpu_usage_info_t* GetCpuUsageInfo()
{
	return _cpuUsageInfo;
}

bool IsDebugEnabled()
{
	return !_config || _config->DebugEnabled;
}

void ___source_dummy_addref(obs_source_t*) {}
void ___weak_source_dummy_addref(obs_weak_source_t*) {}
void ___scene_dummy_addref(obs_scene_t*) {}
void ___sceneitem_dummy_addref(obs_sceneitem_t*) {}
void ___data_dummy_addref(obs_data_t*) {}
void ___data_array_dummy_addref(obs_data_array_t*) {}
void ___output_dummy_addref(obs_output_t*) {}
void ___data_item_dummy_addref(obs_data_item_t*) {}
void ___data_item_release(obs_data_item_t* dataItem){ obs_data_item_release(&dataItem); }
void ___properties_dummy_addref(obs_properties_t*) {}

/**
 * An event has been emitted from a vendor. 
 *
 * A vendor is a unique name registered by a third-party plugin or script, which allows for custom requests and events to be added to obs-websocket.
 * If a plugin or script implements vendor requests or events, documentation is expected to be provided with them.
 *
 * @dataField vendorName | String | Name of the vendor emitting the event
 * @dataField eventType  | String | Vendor-provided event typedef
 * @dataField eventData  | Object | Vendor-provided event data. {} if event does not provide any data
 *
 * @eventSubscription ExternalPlugins
 * @eventType VendorEvent
 * @complexity 3
 * @rpcVersion -1
 * @initialVersion 5.0.0
 * @api events
 * @category general
 */
void WebSocketApiEventCallback(std::string vendorName, std::string eventType, obs_data_t *obsEventData)
{
	json eventData = Utils::Json::ObsDataToJson(obsEventData);

	json broadcastEventData;
	broadcastEventData["vendorName"] = vendorName;
	broadcastEventData["eventType"] = eventType;
	broadcastEventData["eventData"] = eventData;

	_webSocketServer->BroadcastEvent(EventSubscription::ExternalPlugins, "VendorEvent", broadcastEventData);
}


#define PLUGIN_API_TEST
#ifdef PLUGIN_API_TEST

static void test_vendor_request_cb(obs_data_t *requestData, obs_data_t *responseData, void *priv_data)
{
	blog(LOG_INFO, "[test_vendor_request_cb] Request called!");

	blog(LOG_INFO, "[test_vendor_request_cb] Request data: %s", obs_data_get_json(requestData));

	// Set an item to the response data
	obs_data_set_string(responseData, "test", "pp");

	// Emit an event with the request data as the event data
	obs_websocket_vendor_emit_event(priv_data, "TestEvent", requestData);
}

void obs_module_post_load()
{
	blog(LOG_INFO, "[obs_module_post_load] Post load started.");

	auto vendor = obs_websocket_register_vendor("obs-websocket-test");
	if (!vendor) {
		blog(LOG_WARNING, "[obs_module_post_load] Failed to create vendor!");
		return;
	}

	if (!obs_websocket_vendor_register_request(vendor, "TestRequest", test_vendor_request_cb, vendor)) {
		blog(LOG_WARNING, "[obs_module_post_load] Failed to register vendor request!");
		return;
	}

	blog(LOG_INFO, "[obs_module_post_load] Post load completed.");
}

#endif
