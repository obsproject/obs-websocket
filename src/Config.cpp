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
#include <QProcessEnvironment>
#include <qhostinfo.h>

#define SECTION_NAME "WebsocketAPI"
#define PARAM_ENABLE "ServerEnabled"
#define PARAM_PORT "ServerPort"
#define PARAM_DEBUG "DebugEnabled"
#define PARAM_ALERT "AlertsEnabled"
#define PARAM_AUTHREQUIRED "AuthRequired"
#define PARAM_SECRET "AuthSecret"
#define PARAM_SALT "AuthSalt"

#define WAMP_SECTION_NAME SECTION_NAME
#define PARAM_WAMP_ENABLED "WampEnabled"
#define PARAM_WAMP_ALERT "WampAlertsEnabled"
#define PARAM_WAMP_URL "WampUrl"
#define PARAM_WAMP_REALM "WampRealm"
#define PARAM_WAMP_ID "WampId"
#define PARAM_WAMP_ID_ENABLED "WampIdEnabled"
#define PARAM_WAMP_USER "WampUser"
#define PARAM_WAMP_PASSWORD "WampPassword"
#define PARAM_WAMP_AUTH_ENABLED "WampAuthEnabled"
#define PARAM_WAMP_ANON_FALLBACK "WampAnonFallback"
#define PARAM_WAMP_REG_PROC "WampRegProc"
#define PARAM_WAMP_BASE_URI "WampBaseUri"

#define WAMP_ENABLED_ENV_VARIABLE QStringLiteral("WAMP_ENABLED")
#define WAMP_ID_ENV_VARIABLE QStringLiteral("WAMP_ID")
#define WAMP_URL_ENV_VARIABLE QStringLiteral("WAMP_URL")
#define WAMP_REALM_ENV_VARIABLE QStringLiteral("WAMP_REALM")
#define WAMP_BASE_URI_ENV_VARIABLE QStringLiteral("WAMP_BASE_URI")
#define WAMP_REG_PROC_ENV_VARIABLE QStringLiteral("WAMP_REG_PROC")
#define WAMP_ENV_OVERRIDE_VARIABLE QStringLiteral("WAMP_ENV_OVERRIDE")

#define DEFAULT_WAMP_URL "ws://localhost:8080/ws"
#define DEFAULT_WAMP_REALM "realm1"
#define DEFAULT_WAMP_BASE_URI "com.obsstudio.ws"

#include "Config.h"
#include "Utils.h"


Config* Config::_instance = nullptr;

Config::Config(QObject * parent) : QObject(parent),
    ServerEnabled(true),
    ServerPort(4444),
    DebugEnabled(false),
    AlertsEnabled(true),
    AuthRequired(false),
    Secret(""),
    Salt(""),
    SettingsLoaded(false),
    WampEnabled(false),
    WampAlertsEnabled(false),
    WampUrl(DefaultWampUrl()),
    WampRealm(DefaultWampRealm()),
    WampId(DefaultWampId()),
    WampBaseUri(DefaultWampBaseUri()),
    WampIdEnabled(true),
    WampUser(QString()),
    WampPassword(QString()),
    WampAuthEnabled(false),
    WampAnonymousFallback(true),
    WampRegisterProcedure(QString())
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
            SECTION_NAME, PARAM_SECRET, qstring_data_copy(Secret));
        config_set_default_string(obsConfig,
            SECTION_NAME, PARAM_SALT, qstring_data_copy(Salt));
        
        config_set_default_bool(obsConfig,
            WAMP_SECTION_NAME, PARAM_WAMP_ENABLED, WampEnabled);
        config_set_default_bool(obsConfig,
            WAMP_SECTION_NAME, PARAM_WAMP_ALERT, WampAlertsEnabled);
        config_set_default_string(obsConfig,
            WAMP_SECTION_NAME, PARAM_WAMP_URL, qstring_data_copy(WampUrl.toString()));
        config_set_default_string(obsConfig,
            WAMP_SECTION_NAME, PARAM_WAMP_ID, qstring_data_copy(WampId));
        config_set_default_string(obsConfig,
            WAMP_SECTION_NAME, PARAM_WAMP_BASE_URI, qstring_data_copy(WampBaseUri));
        config_set_default_bool(obsConfig,
            WAMP_SECTION_NAME, PARAM_WAMP_ID_ENABLED, WampIdEnabled);
        config_set_default_string(obsConfig,
            WAMP_SECTION_NAME, PARAM_WAMP_USER, qstring_data_copy(WampUser));
        config_set_default_string(obsConfig,
            WAMP_SECTION_NAME, PARAM_WAMP_PASSWORD, qstring_data_copy(WampPassword));
        config_set_default_bool(obsConfig,
            WAMP_SECTION_NAME, PARAM_WAMP_AUTH_ENABLED, WampAuthEnabled);
        config_set_default_bool(obsConfig,
            WAMP_SECTION_NAME, PARAM_WAMP_ANON_FALLBACK, WampAnonymousFallback);
        config_set_default_string(obsConfig,
            WAMP_SECTION_NAME, PARAM_WAMP_REG_PROC, qstring_data_copy(WampRegisterProcedure));
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

    bool envOverride = QProcessEnvironment::systemEnvironment().value(WAMP_ENV_OVERRIDE_VARIABLE, "false") == "true";

    WampEnabled = (envOverride && QProcessEnvironment::systemEnvironment().value(WAMP_ENABLED_ENV_VARIABLE, "false") == "true")
        || config_get_bool(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_ENABLED);
    WampAlertsEnabled = config_get_bool(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_ALERT);
    WampUrl = QUrl(envOverride ?
        QProcessEnvironment::systemEnvironment().value(WAMP_URL_ENV_VARIABLE,
            QString::fromUtf8(config_get_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_URL)))
        : config_get_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_URL));
    WampRealm = envOverride ?
        QProcessEnvironment::systemEnvironment().value(WAMP_REALM_ENV_VARIABLE,
            QString::fromUtf8(config_get_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_REALM)))
        : QString::fromUtf8(config_get_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_REALM));
    WampId = envOverride ?
        QProcessEnvironment::systemEnvironment().value(WAMP_ID_ENV_VARIABLE,
            QString::fromUtf8(config_get_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_ID)))
        : QString::fromUtf8(config_get_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_ID));
    WampBaseUri = envOverride ?
        QProcessEnvironment::systemEnvironment().value(WAMP_BASE_URI_ENV_VARIABLE,
            QString::fromUtf8(config_get_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_BASE_URI)))
        : QString::fromUtf8(config_get_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_BASE_URI));
    WampIdEnabled = config_get_bool(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_ID_ENABLED);
    WampUser = QString::fromUtf8(config_get_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_USER));
    WampPassword = QString::fromUtf8(config_get_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_PASSWORD));
    WampAuthEnabled = config_get_bool(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_AUTH_ENABLED);
    WampAnonymousFallback = config_get_bool(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_ANON_FALLBACK);
    WampRegisterProcedure = envOverride ?
        QProcessEnvironment::systemEnvironment().value(WAMP_REG_PROC_ENV_VARIABLE,
            QString::fromUtf8(config_get_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_REG_PROC)))
        : QString::fromUtf8(config_get_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_REG_PROC));
}

void Config::Save() {
    config_t* obsConfig = obs_frontend_get_global_config();

    config_set_bool(obsConfig, SECTION_NAME, PARAM_ENABLE, ServerEnabled);
    config_set_uint(obsConfig, SECTION_NAME, PARAM_PORT, ServerPort);

    config_set_bool(obsConfig, SECTION_NAME, PARAM_DEBUG, DebugEnabled);
    config_set_bool(obsConfig, SECTION_NAME, PARAM_ALERT, AlertsEnabled);

    config_set_bool(obsConfig, SECTION_NAME, PARAM_AUTHREQUIRED, AuthRequired);
    config_set_string(obsConfig, SECTION_NAME, PARAM_SECRET,
        qstring_data_copy(Secret));
    config_set_string(obsConfig, SECTION_NAME, PARAM_SALT,
        qstring_data_copy(Salt));

    config_set_bool(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_ENABLED, WampEnabled);
    config_set_bool(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_ALERT, WampAlertsEnabled);
    config_set_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_URL,
        qstring_data_copy(WampUrl.toString()));
    config_set_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_REALM,
        qstring_data_copy(WampRealm));
    config_set_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_ID,
        qstring_data_copy(WampId));
    config_set_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_BASE_URI,
        qstring_data_copy(WampBaseUri));
    config_set_bool(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_ID_ENABLED, WampIdEnabled);

    config_set_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_USER,
        qstring_data_copy(WampUser));
    config_set_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_PASSWORD,
        qstring_data_copy(WampPassword));
    config_set_bool(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_AUTH_ENABLED, WampAuthEnabled);

    config_set_bool(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_ANON_FALLBACK, WampAnonymousFallback);
    config_set_string(obsConfig, WAMP_SECTION_NAME, PARAM_WAMP_REG_PROC,
        qstring_data_copy(WampRegisterProcedure));
	
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
    QString str = QString(salt);
	bfree(salt);
	return str;
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
	QString str = QString(challenge);
	bfree(challenge);
    return str;
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

QString Config::DefaultWampId()
{
    QString wampId = Utils::WampUrlFix(QProcessEnvironment::systemEnvironment().value(WAMP_ID_ENV_VARIABLE,
        QHostInfo::localHostName().indexOf('.') > 0 ? //if the hostname contains a period
            QHostInfo::localHostName().left(QHostInfo::localHostName().indexOf('.')) : //strip everything after the first .
            QHostInfo::localHostName()));
    qInfo() << "Wamp ID default resolved " << wampId;
    return wampId;
}

QString Config::DefaultWampBaseUri()
{
    QString wampBaseUri = QProcessEnvironment::systemEnvironment().value(WAMP_BASE_URI_ENV_VARIABLE, DEFAULT_WAMP_BASE_URI);
    qInfo() << "Wamp base URI default resolved " << wampBaseUri;
    return wampBaseUri;
}

QUrl Config::DefaultWampUrl()
{
    QString wampUrl = QProcessEnvironment::systemEnvironment().value(WAMP_URL_ENV_VARIABLE, DEFAULT_WAMP_URL);
    qInfo() << "Wamp Url default resolved " << wampUrl;
    return QUrl(wampUrl);
}

QString Config::DefaultWampRealm()
{
    QString wampRealm = QProcessEnvironment::systemEnvironment().value(WAMP_REALM_ENV_VARIABLE, DEFAULT_WAMP_REALM);
    qInfo() << "Wamp Realm default resolved " << wampRealm;
    return wampRealm;
}


Config* Config::Current() {
    return _instance;
}

void Config::SetCurrent(Config* current) {
    _instance = current;
}
