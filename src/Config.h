#pragma once

#include <QString>
#include <util/config-file.h>

#include "plugin-macros.generated.h"

class Config {
	public:
		Config();
		void Load();
		void Save();
		void SetDefaultsToGlobalStore();
		config_t* GetConfigStore();

		bool PortOverridden;
		bool PasswordOverridden;

		bool FirstLoad;
		bool ServerEnabled;
		uint16_t ServerPort;
		std::atomic<bool> DebugEnabled;
		std::atomic<bool> AlertsEnabled;
		bool AuthRequired;
		QString ServerPassword;

	private:

};
