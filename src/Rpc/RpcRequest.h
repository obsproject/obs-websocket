#pragma once

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QVariantHash>

class RpcRequest {
  public:
    RpcRequest(const QString &id, const QString &methodName,
        const QVariantHash& params);
    QHash<QString, QVariant> immutableParams();
    const QString& getId() const;
    const QString& getMethodName() const;
  private:
    QString id;
    QString methodName;
    QVariantHash parameters;
};
