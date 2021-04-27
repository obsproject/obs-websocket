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

		bool ServerEnabled;
		uint16_t ServerPort;
		bool DebugEnabled;
		bool AlertsEnabled;
		bool AuthRequired;
		QString ServerPassword;

	private:
		;
};
