#include <QNetworkInterface>
#include <QHostAddress>

#include "Utils.h"

#include "../plugin-macros.generated.h"

std::string Utils::Platform::GetLocalAddress()
{
	std::vector<QString> validAddresses;
	for (auto address : QNetworkInterface::allAddresses()) {
		// Exclude addresses which won't work
		if (address == QHostAddress::LocalHost)
			continue;
		else if (address == QHostAddress::LocalHostIPv6)
			continue;
		else if (address.isLoopback())
			continue;
		else if (address.isLinkLocal())
			continue;
		else if (address.isNull())
			continue;

		validAddresses.push_back(address.toString());
	}

	for (auto address : validAddresses) {
		// Hacks to try to pick the best address
		if (address.startsWith("192.168")) {
			return address.toStdString();
		} else if (address.startsWith("172.16")) {
			return address.toStdString();
		}
	}

	if (validAddresses.size() > 0)
		return validAddresses[0].toStdString();

	return "0.0.0.0";
}