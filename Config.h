#ifndef CONFIG_H
#define CONFIG_H

#include <obs-module.h>

class Config {
	public:
		Config();
		void SetPassword(const char *password);
		bool AuthRequired;
		const char *Challenge;
		const char *Salt;
		bool SettingsLoaded;
		static void OBSSaveCallback(obs_data_t *save_data, bool saving, void *);

		static Config* Current();

	private:
		static Config *_instance;
};

#endif // CONFIG_H