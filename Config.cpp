#include "Config.h"

Config *Config::_instance = new Config();

Config::Config() {
	AuthRequired = false;
	Challenge = "";
	Salt = "";
	SettingsLoaded = false;
}

void Config::SetPassword(const char *password) {

}

void Config::OBSSaveCallback(obs_data_t *save_data, bool saving, void *private_data) {
	Config *conf = static_cast<Config *>(private_data);

	if (saving) {
		obs_data_t *settings = obs_data_create();
		obs_data_set_bool(settings, "auth_required", conf->AuthRequired);
		obs_data_set_string(settings, "auth_hash", conf->Challenge);
		obs_data_set_string(settings, "auth_salt", conf->Salt);

		obs_data_set_obj(save_data, "obs-websocket", settings);

		obs_data_release(settings);
	}
	else {
		obs_data_t *settings = obs_data_get_obj(save_data, "obs-websocket");
		if (!settings) {
			settings = obs_data_create();
			obs_data_set_bool(settings, "auth_required", conf->AuthRequired);
			obs_data_set_string(settings, "auth_hash", conf->Challenge);
			obs_data_set_string(settings, "auth_salt", conf->Salt);
		}

		conf->AuthRequired = obs_data_get_bool(settings, "auth_required");
		conf->Challenge = obs_data_get_string(settings, "auth_hash");
		conf->Salt = obs_data_get_string(settings, "auth_salt");
		conf->SettingsLoaded = true;

		obs_data_release(settings);
	}
}

Config* Config::Current() {
	return _instance;
}
