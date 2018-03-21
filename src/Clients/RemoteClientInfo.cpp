#include "RemoteClientInfo.h"

RemoteClientInfo::RemoteClientInfo(
        const QHostAddress& remoteAddr,
        const QHash<QString, QString>& headers
) : remoteAddr(remoteAddr), headers(headers)
{
}

const QHostAddress& RemoteClientInfo::getRemoteAddr() const
{
    return this->remoteAddr;
}

const QHash<QString, QString> RemoteClientInfo::getHeaders() const
{
    return QHash<QString, QString>(this->headers);
}
