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
#include <util/config-file.h>

#include <QCryptographicHash>
#include <QRandomGenerator>

#define SECTION_NAME "WebsocketAPI"
#define PARAM_ENABLE "ServerEnabled"
#define PARAM_PORT "ServerPort"
#define PARAM_DEBUG "DebugEnabled"
#define PARAM_ALERT "AlertsEnabled"
#define PARAM_AUTHREQUIRED "AuthRequired"
#define PARAM_SECRET "AuthSecret"
#define PARAM_SALT "AuthSalt"

#include "Config.h"
#include "Utils.h"

#define QT_TO_UTF8(str) str.toUtf8().constData()

Config* Config::_instance = new Config();

Config::Config() :
	ServerEnabled(true),
	ServerPort(4444),
	DebugEnabled(false),
	AlertsEnabled(true),
	AuthRequired(false),
	Secret(""),
	Salt(""),
	SettingsLoaded(false)
{
	// OBS Config defaults
	config_t* obsConfig = obs_frontend_get_global_config();
	if (obsConfig) {
		config_set_default_bool(obsConfig,
			SECTION_NAME, PARAM_ENABLE, ServerEnabled);
		config_set_default_uint(obsConfig,
			SECTION_NAME, PARAM_PORT, ServerPort);

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

	SessionChallenge = GenerateSalt();
}

Config::~Config()
{
}

void Config::Load()
{
	config_t* obsConfig = obs_frontend_get_global_config();

	ServerEnabled = config_get_bool(obsConfig, SECTION_NAME, PARAM_ENABLE);
	ServerPort = config_get_uint(obsConfig, SECTION_NAME, PARAM_PORT);

	DebugEnabled = config_get_bool(obsConfig, SECTION_NAME, PARAM_DEBUG);
	AlertsEnabled = config_get_bool(obsConfig, SECTION_NAME, PARAM_ALERT);

	AuthRequired = config_get_bool(obsConfig, SECTION_NAME, PARAM_AUTHREQUIRED);
	Secret = config_get_string(obsConfig, SECTION_NAME, PARAM_SECRET);
	Salt = config_get_string(obsConfig, SECTION_NAME, PARAM_SALT);
}

void Config::Save() 
{
	config_t* obsConfig = obs_frontend_get_global_config();

	config_set_bool(obsConfig, SECTION_NAME, PARAM_ENABLE, ServerEnabled);
	config_set_uint(obsConfig, SECTION_NAME, PARAM_PORT, ServerPort);

	config_set_bool(obsConfig, SECTION_NAME, PARAM_DEBUG, DebugEnabled);
	config_set_bool(obsConfig, SECTION_NAME, PARAM_ALERT, AlertsEnabled);

	config_set_bool(obsConfig, SECTION_NAME, PARAM_AUTHREQUIRED, AuthRequired);
	config_set_string(obsConfig, SECTION_NAME, PARAM_SECRET,
		QT_TO_UTF8(Secret));
	config_set_string(obsConfig, SECTION_NAME, PARAM_SALT,
		QT_TO_UTF8(Salt));

	config_save(obsConfig);
}

QString Config::GenerateSalt()
{
	auto random = QRandomGenerator::global();

	// Generate 32 random chars
	QByteArray randomChars(32, '\0');
	random->fillRange((quint32*)randomChars.data(), randomChars.size() / 4);

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

Config* Config::Current()
{
	return _instance;
}
