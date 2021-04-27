#include <QTime>
#include <obs-frontend-api.h>

#include "Config.h"

#define CONFIG_SECTION_NAME "OBSWebSocket"

#define PARAM_ENABLED "ServerEnabled"
#define PARAM_PORT "ServerPort"
#define PARAM_DEBUG "DebugEnabled"
#define PARAM_ALERTS "AlertsEnabled"
#define PARAM_AUTHREQUIRED "AuthRequred"
#define PARAM_PASSWORD "ServerPassword"

Config::Config() :
	ServerEnabled(true),
	ServerPort(4444),
	DebugEnabled(false),
	AlertsEnabled(false),
	AuthRequired(true),
	ServerPassword("")
{
	qsrand(QTime::currentTime().msec());

	SetDefaultsToGlobalStore();
}

void Config::Load()
{
	config_t* obsConfig = GetConfigStore();
	if (!obsConfig) {
		blog(LOG_ERROR, "Unable to fetch OBS config!");
		return;
	}

	ServerEnabled = config_get_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_ENABLED);
	ServerPort = config_get_uint(obsConfig, CONFIG_SECTION_NAME, PARAM_PORT);
	DebugEnabled = config_get_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_DEBUG);
	AlertsEnabled = config_get_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_ALERTS);
	AuthRequired = config_get_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_AUTHREQUIRED);
	ServerPassword = config_get_string(obsConfig, CONFIG_SECTION_NAME, PARAM_PASSWORD);
}

void Config::Save()
{
	config_t* obsConfig = GetConfigStore();
	if (!obsConfig) {
		blog(LOG_ERROR, "Unable to fetch OBS config!");
		return;
	}

	config_set_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_ENABLED, ServerEnabled);
	config_set_uint(obsConfig, CONFIG_SECTION_NAME, PARAM_PORT, ServerPort);
	config_set_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_DEBUG, DebugEnabled);
	config_set_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_ALERTS, AlertsEnabled);
	config_set_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_AUTHREQUIRED, AuthRequired);
	config_set_string(obsConfig, CONFIG_SECTION_NAME, PARAM_PASSWORD, QT_TO_UTF8(ServerPassword));

	config_save(obsConfig);
}

void Config::SetDefaultsToGlobalStore()
{
	config_t* obsConfig = GetConfigStore();
	if (!obsConfig) {
		blog(LOG_ERROR, "Unable to fetch OBS config!");
		return;
	}

	config_set_default_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_ENABLED, ServerEnabled);
	config_set_default_uint(obsConfig, CONFIG_SECTION_NAME, PARAM_PORT, ServerPort);
	config_set_default_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_DEBUG, DebugEnabled);
	config_set_default_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_ALERTS, AlertsEnabled);
	config_set_default_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_AUTHREQUIRED, AuthRequired);
	config_set_default_string(obsConfig, CONFIG_SECTION_NAME, PARAM_PASSWORD, QT_TO_UTF8(ServerPassword));
}

config_t* Config::GetConfigStore()
{
	return obs_frontend_get_global_config();
}