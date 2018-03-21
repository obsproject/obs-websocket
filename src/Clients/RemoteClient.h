#pragma once

#include <QtCore/QString>
#include "RemoteClientInfo.h"

class RemoteClient
{
    public:
        virtual void sendTextMessage(QString& message) = 0;
        virtual const RemoteClientInfo& getConnectionInfo() = 0;
};
