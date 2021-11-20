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

#include <QtCore/QTimer>
#include <QtWidgets/QAction>
#include <QtWidgets/QMainWindow>
#include <obs-module.h>
#include <obs-data.h>
#include <obs-frontend-api.h>

#include "obs-websocket.h"
#include "Config.h"
#include "WebSocketApi.h"
#include "websocketserver/WebSocketServer.h"
#include "eventhandler/EventHandler.h"
#include "forms/SettingsDialog.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-websocket", "en-US")

ConfigPtr _config;
WebSocketApiPtr _webSocketApi;
WebSocketServerPtr _webSocketServer;
EventHandlerPtr _eventHandler;
SettingsDialog *_settingsDialog = nullptr;
os_cpu_usage_info_t* _cpuUsageInfo;

void ___source_dummy_addref(obs_source_t*) {}
void ___scene_dummy_addref(obs_scene_t*) {};
void ___sceneitem_dummy_addref(obs_sceneitem_t*) {};
void ___data_dummy_addref(obs_data_t*) {};
void ___data_array_dummy_addref(obs_data_array_t*) {};
void ___output_dummy_addref(obs_output_t*) {};
void ___data_item_dummy_addref(obs_data_item_t*) {};
void ___data_item_release(obs_data_item_t* dataItem){ obs_data_item_release(&dataItem); };
void ___properties_dummy_addref(obs_properties_t*) {};

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
		blog(LOG_INFO, "[obs_module_unload] WebSocket server is running. Stopping...");
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

void WebSocketApiEventCallback(std::string vendorName, std::string eventType, obs_data_t *obsEventData)
{
	json eventData = Utils::Json::ObsDataToJson(obsEventData);

	json broadcastEventData;
	broadcastEventData["vendorName"] = vendorName;
	broadcastEventData["eventType"] = eventType;
	broadcastEventData["eventData"] = eventData;

	_webSocketServer->BroadcastEvent(EventSubscription::ExternalPlugins, "ExternalPluginEvent", broadcastEventData);
}
