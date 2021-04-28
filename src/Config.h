#pragma once

#include <QString>
#include <util/config-file.h>

class Config {
	public:
		Config();
		void Load();
		void Save();
		void SetDefaultsToGlobalStore();
		config_t* GetConfigStore();

		bool PortOverridden;
		bool PasswordOverridden;

		bool ServerEnabled;
		uint16_t ServerPort;
		bool DebugEnabled;
		bool AlertsEnabled;
		bool AuthRequired;
		QString ServerPassword;

	private:

};
