#pragma once

#include <string>
#include <QString>
#include <QSystemTrayIcon>

namespace Utils {
	namespace Platform {
		std::string GetLocalAddress();
		QString GetCommandLineArgument(QString arg);
		bool GetCommandLineFlagSet(QString arg);
		void SendTrayNotification(QSystemTrayIcon::MessageIcon icon, QString title, QString body);
	}
}