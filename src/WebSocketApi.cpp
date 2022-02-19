#include "WebSocketApi.h"
#include "obs-websocket.h"

#define RETURN_STATUS(status) { calldata_set_bool(cd, "success", status); return; }
#define RETURN_SUCCESS() RETURN_STATUS(true);
#define RETURN_FAILURE() RETURN_STATUS(false);

WebSocketApi::Vendor *get_vendor(calldata_t *cd)
{
	void *voidVendor;
	if (!calldata_get_ptr(cd, "vendor", &voidVendor)) {
		blog(LOG_WARNING, "[WebSocketApi: get_vendor] Failed due to missing `vendor` pointer.");
		return nullptr;
	}

	return static_cast<WebSocketApi::Vendor*>(voidVendor);
}

WebSocketApi::WebSocketApi()
{
	blog_debug("[WebSocketApi::WebSocketApi] Setting up...");

	_procHandler = proc_handler_create();

	proc_handler_add(_procHandler, "bool vendor_register(in string name, out ptr vendor)", &vendor_register_cb, this);
	proc_handler_add(_procHandler, "bool vendor_request_register(in ptr vendor, in string type, in ptr callback)", &vendor_request_register_cb, this);
	proc_handler_add(_procHandler, "bool vendor_request_unregister(in ptr vendor, in string type)", &vendor_request_unregister_cb, this);
	proc_handler_add(_procHandler, "bool vendor_event_emit(in ptr vendor, in string type, in ptr data)", &vendor_event_emit_cb, this);

	proc_handler_t *ph = obs_get_proc_handler();
	assert(ph != NULL);

	proc_handler_add(ph, "bool obs_websocket_api_get_ph(out ptr ph)", &get_ph_cb, this);

	blog_debug("[WebSocketApi::WebSocketApi] Finished.");
}

WebSocketApi::~WebSocketApi()
{
	blog_debug("[WebSocketApi::~WebSocketApi] Shutting down...");

	proc_handler_destroy(_procHandler);

	for (auto vendor : _vendors) {
		blog_debug("[WebSocketApi::~WebSocketApi] Deleting vendor: %s", vendor.first.c_str());
		delete vendor.second;
	}

	blog_debug("[WebSocketApi::~WebSocketApi] Finished.");
}

void WebSocketApi::SetEventCallback(EventCallback cb)
{
	_eventCallback = cb;
}

enum WebSocketApi::RequestReturnCode WebSocketApi::PerformVendorRequest(std::string vendorName, std::string requestType, obs_data_t *requestData, obs_data_t *responseData)
{
	std::shared_lock l(_mutex);

	if (_vendors.count(vendorName) == 0)
		return RequestReturnCode::NoVendor;

	auto v = _vendors[vendorName];

	l.unlock();

	std::shared_lock v_l(v->_mutex);

	if (v->_requests.count(requestType) == 0)
		return RequestReturnCode::NoVendorRequest;

	auto cb = v->_requests[requestType];

	v_l.unlock();

	cb.callback(requestData, responseData, cb.priv_data);

	return RequestReturnCode::Normal;
}

void WebSocketApi::get_ph_cb(void *priv_data, calldata_t *cd)
{
	auto c = static_cast<WebSocketApi*>(priv_data);

	calldata_set_ptr(cd, "ph", (void*)c->_procHandler);

	RETURN_SUCCESS();
}

void WebSocketApi::vendor_register_cb(void *priv_data, calldata_t *cd)
{
	auto c = static_cast<WebSocketApi*>(priv_data);

	const char *vendorName;
	if (!calldata_get_string(cd, "name", &vendorName) || strlen(vendorName) == 0) {
		blog(LOG_WARNING, "[WebSocketApi::vendor_register_cb] Failed due to missing `name` string.");
		RETURN_FAILURE();
	}

	// Theoretically doesn't need a mutex, but it's good to be safe.
	std::unique_lock l(c->_mutex);

	if (c->_vendors.count(vendorName)) {
		blog(LOG_WARNING, "[WebSocketApi::vendor_register_cb] Failed because `%s` is already a registered vendor.", vendorName);
		RETURN_FAILURE();
	}

	Vendor* v = new Vendor();
	v->_name = vendorName;

	c->_vendors[vendorName] = v;

	blog_debug("[WebSocketApi::vendor_register_cb] [vendorName: %s] Registered new vendor.", v->_name.c_str());

	calldata_set_ptr(cd, "vendor", static_cast<void*>(v));

	RETURN_SUCCESS();
}

void WebSocketApi::vendor_request_register_cb(void *, calldata_t *cd)
{
	Vendor *v = get_vendor(cd);
	if (!v)
		RETURN_FAILURE();

	const char *requestType;
	if (!calldata_get_string(cd, "type", &requestType) || strlen(requestType) == 0) {
		blog(LOG_WARNING, "[WebSocketApi::vendor_request_register_cb] [vendorName: %s] Failed due to missing or empty `type` string.", v->_name.c_str());
		RETURN_FAILURE();
	}

	void *voidCallback;
	if (!calldata_get_ptr(cd, "callback", &voidCallback) || !voidCallback) {
		blog(LOG_WARNING, "[WebSocketApi::vendor_request_register_cb] [vendorName: %s] Failed due to missing `callback` pointer.", v->_name.c_str());
		RETURN_FAILURE();
	}

	auto cb = static_cast<obs_websocket_request_callback*>(voidCallback);

	std::unique_lock l(v->_mutex);

	if (v->_requests.count(requestType)) {
		blog(LOG_WARNING, "[WebSocketApi::vendor_request_register_cb] [vendorName: %s] Failed because `%s` is already a registered request.", v->_name.c_str(), requestType);
		RETURN_FAILURE();
	}

	v->_requests[requestType] = *cb;

	blog_debug("[WebSocketApi::vendor_request_register_cb] [vendorName: %s] Registered new vendor request: %s", v->_name.c_str(), requestType);

	RETURN_SUCCESS();
}

void WebSocketApi::vendor_request_unregister_cb(void *, calldata_t *cd)
{
	Vendor *v = get_vendor(cd);
	if (!v)
		RETURN_FAILURE();

	const char *requestType;
	if (!calldata_get_string(cd, "type", &requestType) || strlen(requestType) == 0) {
		blog(LOG_WARNING, "[WebSocketApi::vendor_request_unregister_cb] [vendorName: %s] Failed due to missing `type` string.", v->_name.c_str());
		RETURN_FAILURE();
	}

	std::unique_lock l(v->_mutex);

	if (!v->_requests.count(requestType)) {
		blog(LOG_WARNING, "[WebSocketApi::vendor_request_register_cb] [vendorName: %s] Failed because `%s` is not a registered request.", v->_name.c_str(), requestType);
		RETURN_FAILURE();
	}

	v->_requests.erase(requestType);

	blog_debug("[WebSocketApi::vendor_request_unregister_cb] [vendorName: %s] Unregistered vendor request: %s", v->_name.c_str(), requestType);

	RETURN_SUCCESS();
}

void WebSocketApi::vendor_event_emit_cb(void *priv_data, calldata_t *cd)
{
	auto c = static_cast<WebSocketApi*>(priv_data);

	Vendor *v = get_vendor(cd);
	if (!v)
		RETURN_FAILURE();

	const char *eventType;
	if (!calldata_get_string(cd, "type", &eventType) || strlen(eventType) == 0) {
		blog(LOG_WARNING, "[WebSocketApi::vendor_event_emit_cb] [vendorName: %s] Failed due to missing `type` string.", v->_name.c_str());
		RETURN_FAILURE();
	}

	void *voidEventData;
	if (!calldata_get_ptr(cd, "data", &voidEventData)) {
		blog(LOG_WARNING, "[WebSocketApi::vendor_event_emit_cb] [vendorName: %s] Failed due to missing `data` pointer.", v->_name.c_str());
		RETURN_FAILURE();
	}

	auto eventData = static_cast<obs_data_t*>(voidEventData);

	if (!c->_eventCallback)
		RETURN_FAILURE();

	c->_eventCallback(v->_name, eventType, eventData);

	RETURN_SUCCESS();
}
