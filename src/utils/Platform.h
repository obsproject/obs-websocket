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
		bool GetTextFileContent(std::string fileName, std::string &content);
		bool SetTextFileContent(std::string filePath, std::string content, bool createNew = true);
	}
}
