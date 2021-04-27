#include <obs-module.h>
#include <obs-frontend-api.h>
#include <obs-data.h>

#include <QtCore/QTimer>
#include <QtWidgets/QAction>
#include <QtWidgets/QMainWindow>

#include "obs-websocket.h"
#include "Config.h"
#include "forms/SettingsDialog.h"

// Auto release definitions
void ___source_dummy_addref(obs_source_t*) {}
void ___sceneitem_dummy_addref(obs_sceneitem_t*) {}
void ___data_dummy_addref(obs_data_t*) {}
void ___data_array_dummy_addref(obs_data_array_t*) {}
void ___output_dummy_addref(obs_output_t*) {}

void ___data_item_dummy_addref(obs_data_item_t*) {}
void ___data_item_release(obs_data_item_t* dataItem) {
	obs_data_item_release(&dataItem);
}


OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-websocket", "en-US")

ConfigPtr _config;

bool obs_module_load(void) {
	blog(LOG_INFO, "you can haz websockets (version %s)", OBS_WEBSOCKET_VERSION);
	blog(LOG_INFO, "Qt version (compile-time): %s | Qt version (run-time): %s",
		QT_VERSION_STR, qVersion());

	// Loading finished
	blog(LOG_INFO, "Module loaded.");

	return true;
}

void obs_module_unload() {
	blog(LOG_INFO, "Finished shutting down.");
}

ConfigPtr GetConfig() {
	return _config;
}