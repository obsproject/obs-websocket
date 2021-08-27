#include <QtCore/QTimer>
#include <QtWidgets/QAction>
#include <QtWidgets/QMainWindow>
#include <QTime>
#include <obs-module.h>
#include <obs-data.h>
#include <obs-frontend-api.h>

#include "plugin-macros.generated.h"
#include "obs-websocket.h"
#include "Config.h"
#include "WebSocketServer.h"
#include "eventhandler/EventHandler.h"
#include "forms/SettingsDialog.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-websocket", "en-US")

ConfigPtr _config;
WebSocketServerPtr _webSocketServer;
EventHandlerPtr _eventHandler;
SettingsDialog *_settingsDialog = nullptr;

void ___source_dummy_addref(obs_source_t*) {}
void ___sceneitem_dummy_addref(obs_sceneitem_t*) {};
void ___data_dummy_addref(obs_data_t*) {};
void ___data_array_dummy_addref(obs_data_array_t*) {};
void ___output_dummy_addref(obs_output_t*) {};
void ___data_item_dummy_addref(obs_data_item_t*) {};
void ___data_item_release(obs_data_item_t* dataItem){ obs_data_item_release(&dataItem); };

bool obs_module_load(void)
{
	blog(LOG_INFO, "[obs_module_load] you can haz websockets (Version: %s | RPC Version: %d)", OBS_WEBSOCKET_VERSION, OBS_WEBSOCKET_RPC_VERSION);
	blog(LOG_INFO, "[obs_module_load] Qt version (compile-time): %s | Qt version (run-time): %s",
		QT_VERSION_STR, qVersion());

	// Randomize the random number generator
	qsrand(QTime::currentTime().msec());

	// Create the config object then load the parameters from storage
	_config = ConfigPtr(new Config());
	_config->Load();

	_webSocketServer = WebSocketServerPtr(new WebSocketServer());

	_eventHandler = EventHandlerPtr(new EventHandler(_webSocketServer));

	obs_frontend_push_ui_translation(obs_module_get_string);
	QMainWindow* mainWindow = reinterpret_cast<QMainWindow*>(obs_frontend_get_main_window());
	_settingsDialog = new SettingsDialog(mainWindow);
	obs_frontend_pop_ui_translation();

	const char* menuActionText = obs_module_text("OBSWebSocket.Settings.DialogTitle");
	QAction* menuAction = (QAction*)obs_frontend_add_tools_menu_qaction(menuActionText);
	QObject::connect(menuAction, &QAction::triggered, [] { _settingsDialog->ToggleShowHide(); });

	// Loading finished
	blog(LOG_INFO, "[obs_module_load] Module loaded.");

	if (_config->ServerEnabled)
		_webSocketServer->Start();

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

	_config->FirstLoad = false;
	_config->Save();
	_config.reset();

	blog(LOG_INFO, "[obs_module_unload] Finished shutting down.");
}

ConfigPtr GetConfig()
{
	return _config;
}

WebSocketServerPtr GetWebSocketServer()
{
	return _webSocketServer;
}

EventHandlerPtr GetEventHandler()
{
	return _eventHandler;
}
