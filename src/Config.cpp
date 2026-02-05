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

#include <filesystem>

#include <QUrl>
#include <obs-frontend-api.h>

#include "Config.h"
#include "utils/Crypto.h"
#include "utils/Platform.h"
#include "utils/Obs.h"

#define CONFIG_SECTION_NAME "OBSWebSocket"
#define CONFIG_PARAM_FIRSTLOAD "FirstLoad"
#define CONFIG_PARAM_ENABLED "ServerEnabled"
#define CONFIG_PARAM_PORT "ServerPort"
#define CONFIG_PARAM_ALERTS "AlertsEnabled"
#define CONFIG_PARAM_AUTHREQUIRED "AuthRequired"
#define CONFIG_PARAM_PASSWORD "ServerPassword"

#define CONFIG_FILE_NAME "config.json"
#define PARAM_FIRSTLOAD "first_load"
#define PARAM_ENABLED "server_enabled"
#define PARAM_PORT "server_port"
#define PARAM_ALERTS "alerts_enabled"
#define PARAM_AUTHREQUIRED "auth_required"
#define PARAM_PASSWORD "server_password"
#define PARAM_CLIENT_ENABLED "client_enabled"
#define PARAM_CLIENT_HOST "client_host"
#define PARAM_CLIENT_PORT "client_port"
#define PARAM_CLIENT_USE_TLS "client_use_tls"
#define PARAM_CLIENT_ALLOW_INSECURE "client_allow_insecure"
#define PARAM_CLIENT_ALLOW_INVALID_CERT "client_allow_invalid_cert"
#define PARAM_CLIENT_AUTH_REQUIRED "client_auth_required"
#define PARAM_CLIENT_PASSWORD "client_password"

#define CMDLINE_WEBSOCKET_PORT "websocket_port"
#define CMDLINE_WEBSOCKET_IPV4_ONLY "websocket_ipv4_only"
#define CMDLINE_WEBSOCKET_PASSWORD "websocket_password"
#define CMDLINE_WEBSOCKET_DEBUG "websocket_debug"
#define CMDLINE_WEBSOCKET_CLIENT_ENABLED "websocket_client_enabled"
#define CMDLINE_WEBSOCKET_CLIENT_URL "websocket_client_url"
#define CMDLINE_WEBSOCKET_CLIENT_PASSWORD "websocket_client_password"
#define CMDLINE_WEBSOCKET_CLIENT_ALLOW_INSECURE "websocket_client_allow_insecure"
#define CMDLINE_WEBSOCKET_CLIENT_ALLOW_INVALID_CERT "websocket_client_allow_invalid_cert"

namespace {
	struct ParsedClientEndpoint {
		bool valid = false;
		bool useTls = false;
		std::string host;
		bool hasPort = false;
		uint16_t port = 0;
		std::string error;
	};

	ParsedClientEndpoint ParseClientEndpoint(QString endpointInput)
	{
		ParsedClientEndpoint parsed;
		QString trimmed = endpointInput.trimmed();
		if (trimmed.isEmpty()) {
			parsed.error = "value is empty";
			return parsed;
		}

		QUrl endpoint(trimmed, QUrl::StrictMode);
		if (!endpoint.isValid()) {
			parsed.error = "value is not a valid URL";
			return parsed;
		}

		QString scheme = endpoint.scheme().toLower();
		if (scheme != "ws" && scheme != "wss") {
			parsed.error = "URL scheme must be ws:// or wss://";
			return parsed;
		}

		if (endpoint.host().isEmpty()) {
			parsed.error = "URL host is empty";
			return parsed;
		}

		if (!endpoint.userName().isEmpty() || !endpoint.password().isEmpty()) {
			parsed.error = "URL must not include credentials";
			return parsed;
		}

		QString path = endpoint.path();
		if ((path.size() > 1) || endpoint.hasQuery() || endpoint.hasFragment()) {
			parsed.error = "URL must not include a path, query, or fragment";
			return parsed;
		}

		int parsedPort = endpoint.port(-1);
		if (parsedPort != -1) {
			if (parsedPort < 1 || parsedPort > 65534) {
				parsed.error = "URL port must be between 1 and 65534";
				return parsed;
			}

			parsed.hasPort = true;
			parsed.port = static_cast<uint16_t>(parsedPort);
		}

		parsed.valid = true;
		parsed.useTls = (scheme == "wss");
		parsed.host = endpoint.host().toStdString();
		return parsed;
	}
}

void Config::Load(json config)
{
	// Only load from plugin config directory if there hasn't been a migration
	if (config.is_null()) {
		std::string configFilePath = Utils::Obs::StringHelper::GetModuleConfigPath(CONFIG_FILE_NAME);
		Utils::Json::GetJsonFileContent(configFilePath, config); // Fetch the existing config, which may not exist
	}

	if (!config.is_object()) {
		blog(LOG_INFO, "[Config::Load] Existing configuration not found, using defaults.");
		config = json::object();
	}

	if (config.contains(PARAM_FIRSTLOAD) && config[PARAM_FIRSTLOAD].is_boolean())
		FirstLoad = config[PARAM_FIRSTLOAD];
	if (config.contains(PARAM_ENABLED) && config[PARAM_ENABLED].is_boolean())
		ServerEnabled = config[PARAM_ENABLED];
	if (config.contains(PARAM_ALERTS) && config[PARAM_ALERTS].is_boolean())
		AlertsEnabled = config[PARAM_ALERTS];
	if (config.contains(PARAM_PORT) && config[PARAM_PORT].is_number_unsigned())
		ServerPort = config[PARAM_PORT];
	if (config.contains(PARAM_AUTHREQUIRED) && config[PARAM_AUTHREQUIRED].is_boolean())
		AuthRequired = config[PARAM_AUTHREQUIRED];
	if (config.contains(PARAM_PASSWORD) && config[PARAM_PASSWORD].is_string())
		ServerPassword = config[PARAM_PASSWORD];
	if (config.contains(PARAM_CLIENT_ENABLED) && config[PARAM_CLIENT_ENABLED].is_boolean())
		ClientEnabled = config[PARAM_CLIENT_ENABLED];
	if (config.contains(PARAM_CLIENT_HOST) && config[PARAM_CLIENT_HOST].is_string())
		ClientHost = config[PARAM_CLIENT_HOST];
	if (config.contains(PARAM_CLIENT_PORT) && config[PARAM_CLIENT_PORT].is_number_unsigned())
		ClientPort = config[PARAM_CLIENT_PORT];
	if (config.contains(PARAM_CLIENT_USE_TLS) && config[PARAM_CLIENT_USE_TLS].is_boolean())
		ClientUseTls = config[PARAM_CLIENT_USE_TLS];
	if (config.contains(PARAM_CLIENT_ALLOW_INSECURE) && config[PARAM_CLIENT_ALLOW_INSECURE].is_boolean())
		ClientAllowInsecure = config[PARAM_CLIENT_ALLOW_INSECURE];
	if (config.contains(PARAM_CLIENT_ALLOW_INVALID_CERT) && config[PARAM_CLIENT_ALLOW_INVALID_CERT].is_boolean())
		ClientAllowInvalidCert = config[PARAM_CLIENT_ALLOW_INVALID_CERT];
	if (config.contains(PARAM_CLIENT_AUTH_REQUIRED) && config[PARAM_CLIENT_AUTH_REQUIRED].is_boolean())
		ClientAuthRequired = config[PARAM_CLIENT_AUTH_REQUIRED];
	if (config.contains(PARAM_CLIENT_PASSWORD) && config[PARAM_CLIENT_PASSWORD].is_string())
		ClientPassword = config[PARAM_CLIENT_PASSWORD];

	// Set server password and save it to the config before processing overrides,
	// so that there is always a true configured password regardless of if
	// future loads use the override flag.
	if (FirstLoad) {
		FirstLoad = false;
		if (ServerPassword.empty()) {
			blog(LOG_INFO, "[Config::Load] (FirstLoad) Generating new server password.");
			ServerPassword = Utils::Crypto::GeneratePassword();
		} else {
			blog(LOG_INFO, "[Config::Load] (FirstLoad) Not generating new password since one is already configured.");
		}
		Save();
	}

	if (ClientHost.empty())
		ClientHost = "127.0.0.1";
	if (ClientPassword.empty())
		ClientPassword = ServerPassword;

	// If there are migrated settings, write them to disk before processing arguments.
	if (!config.empty())
		Save();

	// Process `--websocket_port` override
	QString portArgument = Utils::Platform::GetCommandLineArgument(CMDLINE_WEBSOCKET_PORT);
	if (portArgument != "") {
		bool ok;
		uint16_t serverPort = portArgument.toUShort(&ok);
		if (ok) {
			blog(LOG_INFO, "[Config::Load] --websocket_port passed. Overriding WebSocket port with: %d", serverPort);
			PortOverridden = true;
			ServerPort = serverPort;
		} else {
			blog(LOG_WARNING, "[Config::Load] Not overriding WebSocket port since integer conversion failed.");
		}
	}

	// Process `--websocket_ipv4_only` override
	if (Utils::Platform::GetCommandLineFlagSet(CMDLINE_WEBSOCKET_IPV4_ONLY)) {
		blog(LOG_INFO, "[Config::Load] --websocket_ipv4_only passed. Binding only to IPv4 interfaces.");
		Ipv4Only = true;
	}

	// Process `--websocket_password` override
	QString passwordArgument = Utils::Platform::GetCommandLineArgument(CMDLINE_WEBSOCKET_PASSWORD);
	if (passwordArgument != "") {
		blog(LOG_INFO, "[Config::Load] --websocket_password passed. Overriding WebSocket password.");
		PasswordOverridden = true;
		AuthRequired = true;
		ServerPassword = passwordArgument.toStdString();
	}

	// Process `--websocket_debug` override
	if (Utils::Platform::GetCommandLineFlagSet(CMDLINE_WEBSOCKET_DEBUG)) {
		// Debug does not persist on reload, so we let people override it with a flag.
		blog(LOG_INFO, "[Config::Load] --websocket_debug passed. Enabling debug logging.");
		DebugEnabled = true;
	}

	// Process `--websocket_client_enabled` override
	if (Utils::Platform::GetCommandLineFlagSet(CMDLINE_WEBSOCKET_CLIENT_ENABLED)) {
		blog(LOG_INFO, "[Config::Load] --websocket_client_enabled passed. Enabling WebSocket client mode.");
		ClientEnabled = true;
	}

	// Process `--websocket_client_url` override
	QString clientUrlArgument = Utils::Platform::GetCommandLineArgument(CMDLINE_WEBSOCKET_CLIENT_URL);
	if (clientUrlArgument != "") {
		auto parsedEndpoint = ParseClientEndpoint(clientUrlArgument);
		if (parsedEndpoint.valid) {
			blog(LOG_INFO, "[Config::Load] --websocket_client_url passed. Overriding client endpoint.");
			ClientHost = parsedEndpoint.host;
			ClientUseTls = parsedEndpoint.useTls;
			if (parsedEndpoint.hasPort)
				ClientPort = parsedEndpoint.port;
		} else {
			blog(LOG_WARNING, "[Config::Load] Ignoring --websocket_client_url override: %s",
			     parsedEndpoint.error.c_str());
		}
	}

	// Process `--websocket_client_password` override
	QString clientPasswordArgument = Utils::Platform::GetCommandLineArgument(CMDLINE_WEBSOCKET_CLIENT_PASSWORD);
	if (clientPasswordArgument != "") {
		blog(LOG_INFO, "[Config::Load] --websocket_client_password passed. Overriding WebSocket client password.");
		ClientAuthRequired = true;
		ClientPassword = clientPasswordArgument.toStdString();
	}

	// Process `--websocket_client_allow_insecure` override
	if (Utils::Platform::GetCommandLineFlagSet(CMDLINE_WEBSOCKET_CLIENT_ALLOW_INSECURE)) {
		blog(LOG_INFO, "[Config::Load] --websocket_client_allow_insecure passed. Enabling insecure ws:// client mode.");
		ClientAllowInsecure = true;
	}

	// Process `--websocket_client_allow_invalid_cert` override
	if (Utils::Platform::GetCommandLineFlagSet(CMDLINE_WEBSOCKET_CLIENT_ALLOW_INVALID_CERT)) {
		blog(LOG_INFO,
		     "[Config::Load] --websocket_client_allow_invalid_cert passed. Allowing invalid TLS certs for client mode.");
		ClientAllowInvalidCert = true;
	}
}

void Config::Save()
{
	json config;

	std::string configFilePath = Utils::Obs::StringHelper::GetModuleConfigPath(CONFIG_FILE_NAME);
	Utils::Json::GetJsonFileContent(configFilePath, config); // Fetch the existing config, which may not exist

	config[PARAM_FIRSTLOAD] = FirstLoad.load();
	config[PARAM_ENABLED] = ServerEnabled.load();
	if (!PortOverridden)
		config[PARAM_PORT] = ServerPort.load();
	config[PARAM_ALERTS] = AlertsEnabled.load();
	if (!PasswordOverridden) {
		config[PARAM_AUTHREQUIRED] = AuthRequired.load();
		config[PARAM_PASSWORD] = ServerPassword;
	}
	config[PARAM_CLIENT_ENABLED] = ClientEnabled.load();
	config[PARAM_CLIENT_HOST] = ClientHost;
	config[PARAM_CLIENT_PORT] = ClientPort.load();
	config[PARAM_CLIENT_USE_TLS] = ClientUseTls.load();
	config[PARAM_CLIENT_ALLOW_INSECURE] = ClientAllowInsecure.load();
	config[PARAM_CLIENT_ALLOW_INVALID_CERT] = ClientAllowInvalidCert.load();
	config[PARAM_CLIENT_AUTH_REQUIRED] = ClientAuthRequired.load();
	config[PARAM_CLIENT_PASSWORD] = ClientPassword;

	if (Utils::Json::SetJsonFileContent(configFilePath, config))
		blog(LOG_DEBUG, "[Config::Save] Saved config.");
	else
		blog(LOG_ERROR, "[Config::Save] Failed to write config file!");
}

// Finds any old values in global.ini and removes them, then returns the values as JSON
json MigrateGlobalConfigData()
{
	// Get existing global config
	config_t *config = obs_frontend_get_app_config();
	json ret;

	// Move values to temporary JSON blob
	if (config_has_user_value(config, CONFIG_SECTION_NAME, CONFIG_PARAM_FIRSTLOAD)) {
		ret[PARAM_FIRSTLOAD] = config_get_bool(config, CONFIG_SECTION_NAME, CONFIG_PARAM_FIRSTLOAD);
		config_remove_value(config, CONFIG_SECTION_NAME, CONFIG_PARAM_FIRSTLOAD);
	}
	if (config_has_user_value(config, CONFIG_SECTION_NAME, CONFIG_PARAM_ENABLED)) {
		ret[PARAM_ENABLED] = config_get_bool(config, CONFIG_SECTION_NAME, CONFIG_PARAM_ENABLED);
		config_remove_value(config, CONFIG_SECTION_NAME, CONFIG_PARAM_ENABLED);
	}
	if (config_has_user_value(config, CONFIG_SECTION_NAME, CONFIG_PARAM_PORT)) {
		ret[PARAM_PORT] = config_get_uint(config, CONFIG_SECTION_NAME, CONFIG_PARAM_PORT);
		config_remove_value(config, CONFIG_SECTION_NAME, CONFIG_PARAM_PORT);
	}
	if (config_has_user_value(config, CONFIG_SECTION_NAME, CONFIG_PARAM_ALERTS)) {
		ret[PARAM_ALERTS] = config_get_bool(config, CONFIG_SECTION_NAME, CONFIG_PARAM_ALERTS);
		config_remove_value(config, CONFIG_SECTION_NAME, CONFIG_PARAM_ALERTS);
	}
	if (config_has_user_value(config, CONFIG_SECTION_NAME, CONFIG_PARAM_AUTHREQUIRED)) {
		ret[PARAM_AUTHREQUIRED] = config_get_bool(config, CONFIG_SECTION_NAME, CONFIG_PARAM_AUTHREQUIRED);
		config_remove_value(config, CONFIG_SECTION_NAME, CONFIG_PARAM_AUTHREQUIRED);
	}
	if (config_has_user_value(config, CONFIG_SECTION_NAME, CONFIG_PARAM_PASSWORD)) {
		ret[PARAM_PASSWORD] = config_get_string(config, CONFIG_SECTION_NAME, CONFIG_PARAM_PASSWORD);
		config_remove_value(config, CONFIG_SECTION_NAME, CONFIG_PARAM_PASSWORD);
	}

	if (!ret.is_null()) {
		blog(LOG_INFO, "[MigrateGlobalConfigData] Some configurations have been migrated from old config");
		config_save(config);
	}

	return ret;
}

// Migration from storing persistent data in obsWebSocketPersistentData.json to the module config directory
// This will overwrite any persistent data in the destination. People doing manual OBS config modification be warned!
bool MigratePersistentData()
{
	std::error_code ec;

	// Ensure module config directory exists
	auto moduleConfigDirectory = std::filesystem::u8path(Utils::Obs::StringHelper::GetModuleConfigPath(""));
	if (!std::filesystem::exists(moduleConfigDirectory, ec))
		std::filesystem::create_directories(moduleConfigDirectory, ec);
	if (ec) {
		blog(LOG_ERROR, "[MigratePersistentData] Failed to create directory `%s`: %s", moduleConfigDirectory.c_str(),
		     ec.message().c_str());
		return false;
	}

	// Move any existing persistent data to module config directory, then delete old file
	auto oldPersistentDataPath = std::filesystem::u8path(Utils::Obs::StringHelper::GetCurrentProfilePath() +
							     "/../../../obsWebSocketPersistentData.json");
	if (std::filesystem::exists(oldPersistentDataPath, ec)) {
		auto persistentDataPath =
			std::filesystem::u8path(Utils::Obs::StringHelper::GetModuleConfigPath("persistent_data.json"));
		std::filesystem::copy_file(oldPersistentDataPath, persistentDataPath, ec);
		std::filesystem::remove(oldPersistentDataPath, ec);
		blog(LOG_INFO, "[MigratePersistentData] Persistent data migrated to new path");
	}
	if (ec) {
		blog(LOG_ERROR, "[MigratePersistentData] Failed to move persistent data: %s", ec.message().c_str());
		return false;
	}

	return true;
}
