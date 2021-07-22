#include <obs-frontend-api.h>

#include "Config.h"
#include "plugin-macros.generated.h"
#include "utils/Utils.h"

#define CONFIG_SECTION_NAME "OBSWebSocket"

#define PARAM_FIRSTLOAD "FirstLoad"
#define PARAM_ENABLED "ServerEnabled"
#define PARAM_PORT "ServerPort"
#define PARAM_ALERTS "AlertsEnabled"
#define PARAM_AUTHREQUIRED "AuthRequred"
#define PARAM_PASSWORD "ServerPassword"

#define CMDLINE_WEBSOCKET_PORT "websocket_port"
#define CMDLINE_WEBSOCKET_PASSWORD "websocket_password"
#define CMDLINE_WEBSOCKET_DEBUG "websocket_debug"

Config::Config() :
	FirstLoad(true),
	PortOverridden(false),
	PasswordOverridden(false),
	ServerEnabled(true),
	ServerPort(4444),
	DebugEnabled(false),
	AlertsEnabled(false),
	AuthRequired(true),
	ServerPassword("")
{
	SetDefaultsToGlobalStore();
}

void Config::Load()
{
	config_t* obsConfig = GetConfigStore();
	if (!obsConfig) {
		blog(LOG_ERROR, "[Config::Load] Unable to fetch OBS config!");
		return;
	}

	FirstLoad = config_get_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_FIRSTLOAD);
	ServerEnabled = config_get_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_ENABLED);
	AlertsEnabled = config_get_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_ALERTS);

	QString portArgument = Utils::Platform::GetCommandLineArgument(CMDLINE_WEBSOCKET_PORT);
	if (portArgument != "") {
		bool ok;
		uint16_t serverPort = portArgument.toUShort(&ok);
		if (ok) {
			blog(LOG_INFO, "[Config::Load] Overriding websocket port with: %d", serverPort);
			PortOverridden = true;
			ServerPort = serverPort;
		} else {
			ServerPort = config_get_uint(obsConfig, CONFIG_SECTION_NAME, PARAM_PORT);
		}
	} else {
		ServerPort = config_get_uint(obsConfig, CONFIG_SECTION_NAME, PARAM_PORT);
	}

	QString passwordArgument = Utils::Platform::GetCommandLineArgument(CMDLINE_WEBSOCKET_PASSWORD);
	if (passwordArgument != "") {
		blog(LOG_INFO, "[Config::Load] Overriding websocket password");
		PasswordOverridden = true;
		AuthRequired = true;
		ServerPassword = passwordArgument;
	} else {
		AuthRequired = config_get_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_AUTHREQUIRED);
		if (FirstLoad) {
			ServerPassword = Utils::Crypto::GeneratePassword();
		} else {
			ServerPassword = config_get_string(obsConfig, CONFIG_SECTION_NAME, PARAM_PASSWORD);
		}
	}

	if (Utils::Platform::GetCommandLineFlagSet(CMDLINE_WEBSOCKET_DEBUG)) // Debug does not persist on reload, so we let people override it with a flag.
		DebugEnabled = true;
}

void Config::Save()
{
	config_t* obsConfig = GetConfigStore();
	if (!obsConfig) {
		blog(LOG_ERROR, "[Config::Save] Unable to fetch OBS config!");
		return;
	}

	config_set_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_FIRSTLOAD, FirstLoad);
	config_set_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_ENABLED, ServerEnabled);
	if (!PortOverridden) {
		config_set_uint(obsConfig, CONFIG_SECTION_NAME, PARAM_PORT, ServerPort);
	}
	config_set_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_ALERTS, AlertsEnabled);
	if (!PasswordOverridden) {
		config_set_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_AUTHREQUIRED, AuthRequired);
		config_set_string(obsConfig, CONFIG_SECTION_NAME, PARAM_PASSWORD, QT_TO_UTF8(ServerPassword));
	}

	config_save(obsConfig);
}

void Config::SetDefaultsToGlobalStore()
{
	config_t* obsConfig = GetConfigStore();
	if (!obsConfig) {
		blog(LOG_ERROR, "[Config::SetDefaultsToGlobalStore] Unable to fetch OBS config!");
		return;
	}

	config_set_default_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_FIRSTLOAD, FirstLoad);
	config_set_default_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_ENABLED, ServerEnabled);
	config_set_default_uint(obsConfig, CONFIG_SECTION_NAME, PARAM_PORT, ServerPort);
	config_set_default_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_ALERTS, AlertsEnabled);
	config_set_default_bool(obsConfig, CONFIG_SECTION_NAME, PARAM_AUTHREQUIRED, AuthRequired);
	config_set_default_string(obsConfig, CONFIG_SECTION_NAME, PARAM_PASSWORD, QT_TO_UTF8(ServerPassword));
}

config_t* Config::GetConfigStore()
{
	return obs_frontend_get_global_config();
}
