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

#include <obs-frontend-api.h>

#include <QtCore/QCryptographicHash>
#include <QtCore/QTime>
#include <QtWidgets/QSystemTrayIcon>

#define SECTION_NAME "WebsocketAPI"
#define PARAM_ENABLE "ServerEnabled"
#define PARAM_PORT "ServerPort"
#define PARAM_LOCKTOIPV4 "LockToIPv4"
#define PARAM_DEBUG "DebugEnabled"
#define PARAM_ALERT "AlertsEnabled"
#define PARAM_AUTHREQUIRED "AuthRequired"
#define PARAM_SECRET "AuthSecret"
#define PARAM_SALT "AuthSalt"

#include "Utils.h"
#include "WSServer.h"

#include "Config.h"

#define QT_TO_UTF8(str) str.toUtf8().constData()

Config::Config() :
	ServerEnabled(true),
	ServerPort(4444),
	LockToIPv4(false),
	DebugEnabled(false),
	AlertsEnabled(true),
	AuthRequired(false),
	Secret(""),
	Salt(""),
	SettingsLoaded(false)
{
	qsrand(QTime::currentTime().msec());

	SetDefaults();
	SessionChallenge = GenerateSalt();

	obs_frontend_add_event_callback(OnFrontendEvent, this);
}

Config::~Config()
{
	obs_frontend_remove_event_callback(OnFrontendEvent, this);
}

void Config::Load()
{
	config_t* obsConfig = GetConfigStore();

	ServerEnabled = config_get_bool(obsConfig, SECTION_NAME, PARAM_ENABLE);
	ServerPort = config_get_uint(obsConfig, SECTION_NAME, PARAM_PORT);
	LockToIPv4 = config_get_bool(obsConfig, SECTION_NAME, PARAM_LOCKTOIPV4);

	DebugEnabled = config_get_bool(obsConfig, SECTION_NAME, PARAM_DEBUG);
	AlertsEnabled = config_get_bool(obsConfig, SECTION_NAME, PARAM_ALERT);

	AuthRequired = config_get_bool(obsConfig, SECTION_NAME, PARAM_AUTHREQUIRED);
	Secret = config_get_string(obsConfig, SECTION_NAME, PARAM_SECRET);
	Salt = config_get_string(obsConfig, SECTION_NAME, PARAM_SALT);
}

void Config::Save()
{
	config_t* obsConfig = GetConfigStore();

	config_set_bool(obsConfig, SECTION_NAME, PARAM_ENABLE, ServerEnabled);
	config_set_uint(obsConfig, SECTION_NAME, PARAM_PORT, ServerPort);
	config_set_bool(obsConfig, SECTION_NAME, PARAM_LOCKTOIPV4, LockToIPv4);

	config_set_bool(obsConfig, SECTION_NAME, PARAM_DEBUG, DebugEnabled);
	config_set_bool(obsConfig, SECTION_NAME, PARAM_ALERT, AlertsEnabled);

	config_set_bool(obsConfig, SECTION_NAME, PARAM_AUTHREQUIRED, AuthRequired);
	config_set_string(obsConfig, SECTION_NAME, PARAM_SECRET,
		QT_TO_UTF8(Secret));
	config_set_string(obsConfig, SECTION_NAME, PARAM_SALT,
		QT_TO_UTF8(Salt));

	config_save(obsConfig);
}

void Config::SetDefaults()
{
	// OBS Config defaults
	config_t* obsConfig = GetConfigStore();
	if (obsConfig) {
		config_set_default_bool(obsConfig,
			SECTION_NAME, PARAM_ENABLE, ServerEnabled);
		config_set_default_uint(obsConfig,
			SECTION_NAME, PARAM_PORT, ServerPort);
		config_set_default_bool(obsConfig,
			SECTION_NAME, PARAM_LOCKTOIPV4, LockToIPv4);

		config_set_default_bool(obsConfig,
			SECTION_NAME, PARAM_DEBUG, DebugEnabled);
		config_set_default_bool(obsConfig,
			SECTION_NAME, PARAM_ALERT, AlertsEnabled);

		config_set_default_bool(obsConfig,
			SECTION_NAME, PARAM_AUTHREQUIRED, AuthRequired);
		config_set_default_string(obsConfig,
			SECTION_NAME, PARAM_SECRET, QT_TO_UTF8(Secret));
		config_set_default_string(obsConfig,
			SECTION_NAME, PARAM_SALT, QT_TO_UTF8(Salt));
	}
}

config_t* Config::GetConfigStore()
{
	return obs_frontend_get_profile_config();
}

QString Config::GenerateSalt()
{
	// Generate 32 random chars
	const size_t randomCount = 32;
	QByteArray randomChars;
	for (size_t i = 0; i < randomCount; i++) {
		randomChars.append((char)qrand());
	}

	// Convert the 32 random chars to a base64 string
	QString salt = randomChars.toBase64();

	return salt;
}

QString Config::GenerateSecret(QString password, QString salt)
{
	// Concatenate the password and the salt
	QString passAndSalt = "";
	passAndSalt += password;
	passAndSalt += salt;

	// Generate a SHA256 hash of the password and salt
	auto challengeHash = QCryptographicHash::hash(
		passAndSalt.toUtf8(),
		QCryptographicHash::Algorithm::Sha256
	);

	// Encode SHA256 hash to Base64
	QString challenge = challengeHash.toBase64();

	return challenge;
}

void Config::SetPassword(QString password)
{
	QString newSalt = GenerateSalt();
	QString newChallenge = GenerateSecret(password, newSalt);

	this->Salt = newSalt;
	this->Secret = newChallenge;
}

bool Config::CheckAuth(QString response)
{
	// Concatenate auth secret with the challenge sent to the user
	QString challengeAndResponse = "";
	challengeAndResponse += Secret;
	challengeAndResponse += SessionChallenge;

	// Generate a SHA256 hash of challengeAndResponse
	auto hash = QCryptographicHash::hash(
		challengeAndResponse.toUtf8(),
		QCryptographicHash::Algorithm::Sha256
	);

	// Encode the SHA256 hash to Base64
	QString expectedResponse = hash.toBase64();

	bool authSuccess = false;
	if (response == expectedResponse) {
		SessionChallenge = GenerateSalt();
		authSuccess = true;
	}

	return authSuccess;
}

void Config::OnFrontendEvent(enum obs_frontend_event event, void* param)
{
	auto config = reinterpret_cast<Config*>(param);

	if (event == OBS_FRONTEND_EVENT_PROFILE_CHANGED) {
		obs_frontend_push_ui_translation(obs_module_get_string);
		QString startMessage = QObject::tr("OBSWebsocket.ProfileChanged.Started");
		QString stopMessage = QObject::tr("OBSWebsocket.ProfileChanged.Stopped");
		QString restartMessage = QObject::tr("OBSWebsocket.ProfileChanged.Restarted");
		obs_frontend_pop_ui_translation();

		bool previousEnabled = config->ServerEnabled;
		uint64_t previousPort = config->ServerPort;
		bool previousLock = config->LockToIPv4;

		config->SetDefaults();
		config->Load();

		if (config->ServerEnabled != previousEnabled || config->ServerPort != previousPort || config->LockToIPv4 != previousLock) {
			auto server = GetServer();
			server->stop();

			if (config->ServerEnabled) {
				server->start(config->ServerPort, config->LockToIPv4);

				if (previousEnabled != config->ServerEnabled) {
					Utils::SysTrayNotify(startMessage, QSystemTrayIcon::MessageIcon::Information);
				} else {
					Utils::SysTrayNotify(restartMessage, QSystemTrayIcon::MessageIcon::Information);
				}
			} else {
				Utils::SysTrayNotify(stopMessage, QSystemTrayIcon::MessageIcon::Information);
			}
		}
	}
}

void Config::MigrateFromGlobalSettings()
{
	config_t* source = obs_frontend_get_global_config();
	config_t* destination = obs_frontend_get_profile_config();

	if(config_has_user_value(source, SECTION_NAME, PARAM_ENABLE)) {
		bool value = config_get_bool(source, SECTION_NAME, PARAM_ENABLE);
		config_set_bool(destination, SECTION_NAME, PARAM_ENABLE, value);

		config_remove_value(source, SECTION_NAME, PARAM_ENABLE);
	}

	if(config_has_user_value(source, SECTION_NAME, PARAM_PORT)) {
		uint64_t value = config_get_uint(source, SECTION_NAME, PARAM_PORT);
		config_set_uint(destination, SECTION_NAME, PARAM_PORT, value);

		config_remove_value(source, SECTION_NAME, PARAM_PORT);
	}
	
	if(config_has_user_value(source, SECTION_NAME, PARAM_LOCKTOIPV4)) {
		bool value = config_get_bool(source, SECTION_NAME, PARAM_LOCKTOIPV4);
		config_set_bool(destination, SECTION_NAME, PARAM_LOCKTOIPV4, value);

		config_remove_value(source, SECTION_NAME, PARAM_LOCKTOIPV4);
	}

	if(config_has_user_value(source, SECTION_NAME, PARAM_DEBUG)) {
		bool value = config_get_bool(source, SECTION_NAME, PARAM_DEBUG);
		config_set_bool(destination, SECTION_NAME, PARAM_DEBUG, value);

		config_remove_value(source, SECTION_NAME, PARAM_DEBUG);
	}

	if(config_has_user_value(source, SECTION_NAME, PARAM_ALERT)) {
		bool value = config_get_bool(source, SECTION_NAME, PARAM_ALERT);
		config_set_bool(destination, SECTION_NAME, PARAM_ALERT, value);

		config_remove_value(source, SECTION_NAME, PARAM_ALERT);
	}

	if(config_has_user_value(source, SECTION_NAME, PARAM_AUTHREQUIRED)) {
		bool value = config_get_bool(source, SECTION_NAME, PARAM_AUTHREQUIRED);
		config_set_bool(destination, SECTION_NAME, PARAM_AUTHREQUIRED, value);

		config_remove_value(source, SECTION_NAME, PARAM_AUTHREQUIRED);
	}

	if(config_has_user_value(source, SECTION_NAME, PARAM_SECRET)) {
		const char* value = config_get_string(source, SECTION_NAME, PARAM_SECRET);
		config_set_string(destination, SECTION_NAME, PARAM_SECRET, value);

		config_remove_value(source, SECTION_NAME, PARAM_SECRET);
	}

	if(config_has_user_value(source, SECTION_NAME, PARAM_SALT)) {
		const char* value = config_get_string(source, SECTION_NAME, PARAM_SALT);
		config_set_string(destination, SECTION_NAME, PARAM_SALT, value);

		config_remove_value(source, SECTION_NAME, PARAM_SALT);
	}

	config_save(destination);
}
