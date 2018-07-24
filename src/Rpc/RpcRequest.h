#pragma once

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QVariantHash>

class RpcRequest
{
public:
	RpcRequest(const QString &id, const QString &methodName,
		const QVariantHash& params);
	const QVariantHash immutableParams() const;
	const QString& getId() const;
	const QString& getMethodName() const;
	const QVariant& param(QString name) const;

private:
	QString id;
	QString methodName;
	QVariantHash parameters;
};
