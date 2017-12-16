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

#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QString>
#include <QUrl>

#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

class Config : public QObject {
  public:
	
    explicit Config(QObject* parent);
    ~Config();
    void Load();
    void Save();

    void SetPassword(QString password);
    bool CheckAuth(QString userChallenge);
    QString GenerateSalt();
    static QString GenerateSecret(
        QString password, QString salt);

    bool ServerEnabled;
    uint64_t ServerPort;

    bool DebugEnabled;
    bool AlertsEnabled;

    bool AuthRequired;
    QString Secret;
    QString Salt;
    QString SessionChallenge;
    bool SettingsLoaded;
	
	bool WampEnabled;
    bool WampAlertsEnabled;
    /* ws:// or wss:// url syntax */
    QUrl WampUrl;
    /* realm1 on the example from crossfire io */
    QString WampRealm;

    /*  defaults to the local hostname of the box or WS_HOSTNAME environment variable.  Used to create the topic and procedure strings for WAMP so that they are specific to this device.  They will all end with this suffix. */
    QString WampId;
    QString WampBaseUri;
    bool WampIdEnabled;

    QString WampUser;
    QString WampPassword;
    bool WampAuthEnabled;
    //if authentication fails, should the connection attempt be retried with anonymous auth only
    bool WampAnonymousFallback;
    //after the connection completes, if specified, this procedure will be called and the response will be used to configure the WAMP connection
    QString WampRegisterProcedure;

    static Config* Current();
    static void SetCurrent(Config* config);
  private:
    static Config* _instance;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context rng;
    static QString DefaultWampId();
    static QString DefaultWampBaseUri();
    static QString DefaultWampRealm();
    static QUrl DefaultWampUrl();
};

#endif // CONFIG_H
