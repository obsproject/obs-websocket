/*
obs-websocket
Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>
Copyright (C) 2017	Mikhail Swift <https://github.com/mikhailswift>

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

#ifndef WSREQUESTHANDLER_H
#define WSREQUESTHANDLER_H

#include <QWebSocket>
#include <QWebSocketServer>

#include <obs-frontend-api.h>

class WSRequestHandler : public QObject {
  Q_OBJECT

  public:
    explicit WSRequestHandler(QWebSocket* client);
    ~WSRequestHandler();
    void processIncomingMessage(QString textMessage);
    bool hasField(const char* name);

  private:
    static obs_service_t* _service;
    QWebSocket* _client;
    const char* _messageId;
    const char* _requestType;
    obs_data_t* data;

    QMap<QString, void(*)(WSRequestHandler*)> messageMap;
    QSet<QString> authNotRequired;

    void SendOKResponse(obs_data_t* additionalFields = NULL);
    void SendErrorResponse(const char* errorMessage);
    void SendResponse(obs_data_t* response);

    static void HandleGetVersion(WSRequestHandler* req);
    static void HandleGetAuthRequired(WSRequestHandler* req);
    static void HandleAuthenticate(WSRequestHandler* req);

    static void HandleSetCurrentScene(WSRequestHandler* req);
    static void HandleGetCurrentScene(WSRequestHandler* req);
    static void HandleGetSceneList(WSRequestHandler* req);

    static void HandleSetSceneItemRender(WSRequestHandler* req);
    static void HandleSetSceneItemPosition(WSRequestHandler* req);
    static void HandleSetSceneItemTransform(WSRequestHandler* req);
    static void HandleSetSceneItemCrop(WSRequestHandler* req);
    static void HandleResetSceneItem(WSRequestHandler* req);

    static void HandleGetStreamingStatus(WSRequestHandler* req);
    static void HandleStartStopStreaming(WSRequestHandler* req);
    static void HandleStartStopRecording(WSRequestHandler* req);
    static void HandleStartStreaming(WSRequestHandler* req);
    static void HandleStopStreaming(WSRequestHandler* req);
    static void HandleStartRecording(WSRequestHandler* req);
    static void HandleStopRecording(WSRequestHandler* req);

    static void HandleSetRecordingFolder(WSRequestHandler* req);
    static void HandleGetRecordingFolder(WSRequestHandler* req);

    static void HandleGetTransitionList(WSRequestHandler* req);
    static void HandleGetCurrentTransition(WSRequestHandler* req);
    static void HandleSetCurrentTransition(WSRequestHandler* req);

    static void HandleSetVolume(WSRequestHandler* req);
    static void HandleGetVolume(WSRequestHandler* req);
    static void HandleToggleMute(WSRequestHandler* req);
    static void HandleSetMute(WSRequestHandler* req);
    static void HandleGetMute(WSRequestHandler* req);
    static void HandleGetSpecialSources(WSRequestHandler* req);

    static void HandleSetCurrentSceneCollection(WSRequestHandler* req);
    static void HandleGetCurrentSceneCollection(WSRequestHandler* req);
    static void HandleListSceneCollections(WSRequestHandler* req);

    static void HandleSetCurrentProfile(WSRequestHandler* req);
    static void HandleGetCurrentProfile(WSRequestHandler* req);
    static void HandleListProfiles(WSRequestHandler* req);

    static void HandleSetStreamSettings(WSRequestHandler* req);
    static void HandleGetStreamSettings(WSRequestHandler* req);
    static void HandleSaveStreamSettings(WSRequestHandler* req);

    static void HandleSetTransitionDuration(WSRequestHandler* req);
    static void HandleGetTransitionDuration(WSRequestHandler* req);

    static void HandleGetStudioModeStatus(WSRequestHandler* req);
    static void HandleGetPreviewScene(WSRequestHandler* req);
    static void HandleSetPreviewScene(WSRequestHandler* req);
    static void HandleTransitionToProgram(WSRequestHandler* req);
    static void HandleEnableStudioMode(WSRequestHandler* req);
    static void HandleDisableStudioMode(WSRequestHandler* req);
    static void HandleToggleStudioMode(WSRequestHandler* req);
        
    static void HandleSetTextGDIPlusProperties(WSRequestHandler* req);
    static void HandleGetTextGDIPlusProperties(WSRequestHandler* req);
    static void HandleSetBrowserSourceProperties(WSRequestHandler* req);
    static void HandleGetBrowserSourceProperties(WSRequestHandler* req);
};

#endif // WSPROTOCOL_H
