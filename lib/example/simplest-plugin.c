#include <obs-module.h>

#include "../obs-websocket-api.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

obs_websocket_vendor vendor;

bool obs_module_load(void)
{
    blog(LOG_INFO, "Example obs-websocket-api plugin loaded!");
    return true;
}

void example_request_cb(obs_data_t *request_data, obs_data_t *response_data, void *priv_data);
void obs_module_post_load(void)
{
	vendor = obs_websocket_register_vendor("api_example_plugin");
	if (!vendor) {
		blog(LOG_ERROR, "Vendor registration failed! (obs-websocket should have logged something if installed properly.)");
		return;
	}

	if (!obs_websocket_vendor_register_request(vendor, "example_request", example_request_cb, NULL))
		blog(LOG_ERROR, "Failed to register `example_request` request with obs-websocket.");
}

void obs_module_unload(void)
{
	blog(LOG_INFO, "Example obs-websocket-api plugin unloaded!");
}

void example_request_cb(obs_data_t *request_data, obs_data_t *response_data, void *priv_data)
{
	if (obs_data_has_user_value(request_data, "ping"))
		obs_data_set_bool(response_data, "pong", true);

	UNUSED_PARAMETER(priv_data);
}
