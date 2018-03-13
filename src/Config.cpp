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

#include <mbedtls/base64.h>
#include <mbedtls/sha256.h>
#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <string>

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

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&rng);
    mbedtls_ctr_drbg_seed(&rng, mbedtls_entropy_func, &entropy, nullptr, 0);

    SessionChallenge = GenerateSalt();
}

Config::~Config() {
    mbedtls_ctr_drbg_free(&rng);
    mbedtls_entropy_free(&entropy);
}

void Config::Load() {
    config_t* obsConfig = obs_frontend_get_global_config();

    ServerEnabled = config_get_bool(obsConfig, SECTION_NAME, PARAM_ENABLE);
    ServerPort = config_get_uint(obsConfig, SECTION_NAME, PARAM_PORT);

    DebugEnabled = config_get_bool(obsConfig, SECTION_NAME, PARAM_DEBUG);
    AlertsEnabled = config_get_bool(obsConfig, SECTION_NAME, PARAM_ALERT);

    AuthRequired = config_get_bool(obsConfig, SECTION_NAME, PARAM_AUTHREQUIRED);
    Secret = config_get_string(obsConfig, SECTION_NAME, PARAM_SECRET);
    Salt = config_get_string(obsConfig, SECTION_NAME, PARAM_SALT);
}

void Config::Save() {
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

QString Config::GenerateSalt() {
    // Generate 32 random chars
    unsigned char* randomChars = (unsigned char*)bzalloc(32);
    mbedtls_ctr_drbg_random(&rng, randomChars, 32);

    // Convert the 32 random chars to a base64 string
    char* salt = (char*)bzalloc(64);
    size_t saltBytes;
    mbedtls_base64_encode(
        (unsigned char*)salt, 64, &saltBytes,
        randomChars, 32);

    bfree(randomChars);
    return salt;
}

QString Config::GenerateSecret(QString password, QString salt) {
    // Concatenate the password and the salt
    QString passAndSalt = "";
    passAndSalt += password;
    passAndSalt += salt;

    // Generate a SHA256 hash of the password
    unsigned char* challengeHash = (unsigned char*)bzalloc(32);
    mbedtls_sha256(
        (unsigned char*)passAndSalt.toUtf8().constData(), passAndSalt.length(),
        challengeHash, 0);

    // Encode SHA256 hash to Base64
    char* challenge = (char*)bzalloc(64);
    size_t challengeBytes = 0;
    mbedtls_base64_encode(
        (unsigned char*)challenge, 64, &challengeBytes,
        challengeHash, 32);

    bfree(challengeHash);
    return challenge;
}

void Config::SetPassword(QString password) {
    QString newSalt = GenerateSalt();
    QString newChallenge = GenerateSecret(password, newSalt);

    this->Salt = newSalt;
    this->Secret = newChallenge;
}

bool Config::CheckAuth(QString response) {
    // Concatenate auth secret with the challenge sent to the user
    QString challengeAndResponse = "";
    challengeAndResponse += Secret;
    challengeAndResponse += SessionChallenge;

    // Generate a SHA256 hash of challengeAndResponse
    unsigned char* hash = (unsigned char*)bzalloc(32);
    mbedtls_sha256(
        (unsigned char*)challengeAndResponse.toUtf8().constData(),
        challengeAndResponse.length(),
        hash, 0);

    // Encode the SHA256 hash to Base64
    char* expectedResponse = (char*)bzalloc(64);
    size_t base64_size = 0;
    mbedtls_base64_encode(
        (unsigned char*)expectedResponse, 64, &base64_size,
        hash, 32);

    bool authSuccess = false;
    if (response == QString(expectedResponse)) {
        SessionChallenge = GenerateSalt();
        authSuccess = true;
    }

    bfree(hash);
    bfree(expectedResponse);
    return authSuccess;
}

Config* Config::Current() {
    return _instance;
}
