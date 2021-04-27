#include <obs-module.h>
#include <obs-frontend-api.h>
#include <obs-data.h>

#include <QtCore/QTimer>
#include <QtWidgets/QAction>
#include <QtWidgets/QMainWindow>

#include "obs-websocket.h"
#include "Config.h"
#include "WebSocketServer.h"
#include "forms/SettingsDialog.h"

// Auto release definitions
void ___source_dummy_addref(obs_source_t*) {}
void ___sceneitem_dummy_addref(obs_sceneitem_t*) {}
void ___data_dummy_addref(obs_data_t*) {}
void ___data_array_dummy_addref(obs_data_array_t*) {}
void ___output_dummy_addref(obs_output_t*) {}

void ___data_item_dummy_addref(obs_data_item_t*) {}
void ___data_item_release(obs_data_item_t* dataItem)
{
	obs_data_item_release(&dataItem);
}


OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-websocket", "en-US")

ConfigPtr _config;
WebSocketServerPtr _webSocketServer;
SettingsDialog *_settingsDialog = nullptr;

bool obs_module_load(void)
{
	blog(LOG_INFO, "you can haz websockets (version %s)", OBS_WEBSOCKET_VERSION);
	blog(LOG_INFO, "Qt version (compile-time): %s | Qt version (run-time): %s",
		QT_VERSION_STR, qVersion());

	_config = ConfigPtr(new Config());
	_config->Load();

	_webSocketServer = WebSocketServerPtr(new WebSocketServer());

	obs_frontend_push_ui_translation(obs_module_get_string);
	QMainWindow* mainWindow = (QMainWindow*)obs_frontend_get_main_window();
	_settingsDialog = new SettingsDialog(mainWindow);
	obs_frontend_pop_ui_translation();

	const char* menuActionText = obs_module_text("OBSWebSocket.Settings.DialogTitle");
	QAction* menuAction = (QAction*)obs_frontend_add_tools_menu_qaction(menuActionText);
	QObject::connect(menuAction, &QAction::triggered, [] { _settingsDialog->ToggleShowHide(); });

	// Loading finished
	blog(LOG_INFO, "Module loaded.");

	return true;
}

void obs_module_unload()
{
	_config.reset();
	_webSocketServer.reset();
	blog(LOG_INFO, "Finished shutting down.");
}

ConfigPtr GetConfig()
{
	return _config;
}

WebSocketServerPtr GetWebSocketServer()
{
	return _webSocketServer;
}
