#pragma once

#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtNetwork/QHostAddress>

class RemoteClientInfo
{
    public:
        RemoteClientInfo(const QHostAddress& remoteAddr,
                             const QHash<QString, QString>& headers);
        const QHostAddress& getRemoteAddr() const;
        const QHash<QString, QString> getHeaders() const;
    private:
        QHostAddress remoteAddr;
        QHash<QString, QString> headers;
};
