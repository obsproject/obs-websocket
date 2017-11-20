#include <QString>
#include "Utils.h"
#include "Config.h"
#include "WSRequestHandler.h"
#include "WSServer.h"

const char* wampStatusToString(WSServer::WampConnectionStatus status) {
    switch (status)
    {
        case WSServer::Connected:
            return "CONNECTED";
        case WSServer::Connecting:
            return "CONNECTING";
        case WSServer::Disconnected:
            return "DISCONNECTED";
        case WSServer::Error:
            return "ERROR";
    }
}

/**
 * Sets the current WAMP settings. Requires authentication.
 *
 * @return {String} `url` The WAMP web socket connection url.  Cannot be removed by setting to empty string.
 * @return {String} `realm` The WAMP realm.  Cannot be removed by setting to empty string.
 * @return {String} `id` The suffix added to all wamp procedure names and publish topics. Used to uniquely identify a given OBS Studio instance. Can be unspecified by setting to empty string.
 * @return {String} `baseuri` The prefix added to all wamp procedure names and publish topics. Generalled used for WAMP path based authentication. Cannot be removed by setting to empty string.
 * @return {bool} `alerts-enabled` Are desktop tray alerts shown on wamp connection events?
 * @return {bool} `auth-enabled` Should authentication be used when connecting to the WAMP server.
 * @return {bool} `anonymous-fallback` If authentication fails, should an unauthenticated connection be attempted.
 * @return {String} `user` The username for authentication. Can be unspecified by setting to empty string.
 * @return {String} `password` The password (i.e.secret) used for authentication. Can be unspecified by setting to empty string.
 * @return {String} `register-procedure` If specified, this procedure is called after the WAMP connection that allows the WAMP server to update the same configuration parameters as available here before the OBS proedures are registered. Can be unspecified by setting to empty string.
 *
 * @api requests
 * @name SetWampSettings
 * @category wamp
 * @since 4.3
 */
void WSRequestHandler::HandleSetWampSettings(WSRequestHandler* req) {
    Config* config = Config::Current();
    
    OBSDataAutoRelease response = obs_data_create();
    
    QString url = obs_data_get_string(req->data, "url");
    QString realm = obs_data_get_string(req->data, "realm");
    QString wampId = obs_data_get_string(req->data, "id");
    QString baseuri = obs_data_get_string(req->data, "baseuri");
    bool alertsEnabled = obs_data_get_bool(req->data, "alerts-enabled");
    bool authEnabled = obs_data_get_bool(req->data, "auth-enabled");
    bool anonymousFallback = obs_data_get_bool(req->data, "anonymous-fallback");
    QString user = obs_data_get_string(req->data, "user");
    QString password = obs_data_get_string(req->data, "password");
    QString registerProcedure = obs_data_get_string(req->data, "register-procedure");
    
    
    if (!url.isEmpty())  //required
        config->WampUrl = QUrl(url);
    if (!realm.isEmpty()) //required
        config->WampRealm = realm;
    
    if (req->hasField("id")) //optional so can be set to empty string
    {
        config->WampIdEnabled = !wampId.isEmpty();
        config->WampId = wampId;
    }
    
    if (!baseuri.isEmpty()) //required
        config->WampBaseUri = baseuri;
    
    if (alertsEnabled != config->WampAlertsEnabled && req->hasField("alerts-enabled"))
    {
        config->WampAlertsEnabled = alertsEnabled;
    }
    
    if (req->hasField("user")) //optional so can be set to empty string
    {
        authEnabled = !user.isEmpty();
        config->WampAuthEnabled = authEnabled;
        config->WampUser = user;
    }
    else if (authEnabled != config->WampAuthEnabled && req->hasField("auth-enabled"))
    {
        config->WampAuthEnabled = authEnabled;
    }
    if (req->hasField("password")) //optional so can be set to empty string
    {
        authEnabled = true;
        config->WampAuthEnabled = true;
        config->WampPassword = password;
    }
    
    if (anonymousFallback != config->WampAnonymousFallback && req->hasField("anonymous-fallback"))
    {
        config->WampAnonymousFallback = anonymousFallback;
    }
    
    if (req->hasField("register-procedure")) //optional so can be set to empty string
    {
        config->WampRegisterProcedure = registerProcedure;
    }
    
    bool save = obs_data_get_bool(req->data, "save");
    bool reconnect = obs_data_get_bool(req->data, "reconnect");
    
    if (save)
        config->Save();
    
    if (reconnect || !config->WampEnabled)
    {
        QMetaObject::invokeMethod(WSServer::Instance, "StopWamp");
    }
    
    if (config->WampEnabled)
        QMetaObject::invokeMethod(WSServer::Instance, "StartWamp",
                                  Q_ARG(QUrl, config->WampUrl),
                                  Q_ARG(QString, config->WampRealm),
                                  Q_ARG(QString, config->WampBaseUri),
                                  Q_ARG(QString, config->WampIdEnabled?config->WampId:""),
                                  Q_ARG(QString, config->WampAuthEnabled?config->WampUser:""),
                                  Q_ARG(QString, config->WampAuthEnabled?config->WampPassword:""));

    req->SendOKResponse(response);
}

/**
 * Gets the current WAMP settings.  Can be called unauthenticated but then only returns 'url', 'realm', and 'id'
 *
 * @return {String} `url` The WAMP web socket connection url
 * @return {String} `realm` The WAMP realm
 * @return {String} `id` The suffix added to all wamp procedure names and publish topics. Used to uniquely identify a given OBS Studio instance. Can be unspecified if no suffix is added
 * @return {String} `baseuri` The prefix added to all wamp procedure names and publish topics. Generalled used for WAMP path based authentication.
 * @return {bool} `alerts-enabled` Are desktop tray alerts shown on wamp connection events?
 * @return {bool} `auth-enabled` Should authentication be used when connecting to the WAMP server.
 * @return {bool} `anonymous-fallback` If authentication fails, should an unauthenticated connection be attempted.
 * @return {String} `user` The username for authentication
 * @return {String} `password` The password (i.e.secret) used for authentication
 * @return {String} `register-procedure` If specified, this procedure is called after the WAMP connection that allows the WAMP server to update the same configuration parameters as available here before the OBS proedures are registered
 *
 * @api requests
 * @name GetWampSettings
 * @category wamp
 * @since 4.3
 */
void WSRequestHandler::HandleGetWampSettings(WSRequestHandler* req) {
    Config* config = Config::Current();
    
    OBSDataAutoRelease response = obs_data_create();
    
    obs_data_set_string(response, "url", config->WampUrl.toString().toUtf8());
    obs_data_set_string(response, "realm", config->WampRealm.toUtf8());
    if (config->WampIdEnabled)
    {
        obs_data_set_string(response, "id", config->WampId.toUtf8());
    }
    
    if (req->isAuthenticated())
    {
        obs_data_set_string(response, "baseuri", config->WampBaseUri.toUtf8());
        obs_data_set_bool(response, "alerts-enabled", config->WampAlertsEnabled);
        obs_data_set_bool(response, "auth-enabled", config->WampIdEnabled);
        obs_data_set_bool(response, "anonymous-fallback", config->WampAnonymousFallback);
        obs_data_set_string(response, "user", config->WampUser.toUtf8());
        obs_data_set_string(response, "password", config->WampPassword.toUtf8());
        obs_data_set_string(response, "register-procedure", config->WampRegisterProcedure.toUtf8());
    }
    
    req->SendOKResponse(response);
}

/**
 * Gets the current WAMP status.  Can be called unauthenticated
 *
 * @return {String} `status` Either DISCONNECTED, CONNECTING, CONNECTED or ERROR
 * @return {String} `error-uri` The WAMP uri of the error that occured
 * @return {String} `error-message` If the status is ERROR, a human readable message describing the error or the 'error-uri' if not present.
 * @return {Object} `error-data` If the status is ERROR and additional information about the error is present, this can contain additional data fields.
 *
 * @api requests
 * @name GetWampSettings
 * @category wamp
 * @since 4.3
 */
void WSRequestHandler::HandleGetWampStatus(WSRequestHandler* req)
{
    OBSDataAutoRelease response = obs_data_create();
    
    WSServer::WampConnectionStatus status = WSServer::Instance->GetWampStatus();
    obs_data_set_string(response, "status", wampStatusToString(status));
    if (status == WSServer::Error)
    {
        obs_data_set_string(response, "error-uri", WSServer::Instance->GetWampErrorUri().toUtf8());
        obs_data_set_string(response, "error-message", WSServer::Instance->GetWampErrorMessage().toUtf8());
        
        OBSDataAutoRelease errorData = WSServer::Instance->GetWampErrorData();
        OBSDataItemAutoRelease firstItem = obs_data_first(errorData);
        if (firstItem) //there is at least one item in the data
            obs_data_set_obj(response, "error-data", errorData);
    }
    
    req->SendOKResponse(response);
}
