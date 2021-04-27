#include "JsonUtils.h"

bool JsonUtils::JsonArrayIsValidObsArray(json j)
{
	for (auto it : j) {
		if (!it.is_object())
			return false;
	}

	return true;
}

void obs_data_set_json_object_item(obs_data_t *d, json j);

void obs_data_set_json_object(obs_data_t *d, const char *key, json j)
{
	OBSDataAutoRelease subObj = obs_data_create();
	obs_data_set_json_object_item(subObj, j);
	obs_data_set_obj(d, key, subObj);
}

void obs_data_set_json_array(obs_data_t *d, const char *key, json j)
{
	OBSDataArrayAutoRelease array = obs_data_array_create();

	for (auto& [key, value] : j.items()) {
		if (!value.is_object())
			continue;

		OBSDataAutoRelease item = obs_data_create();
		obs_data_set_json_object_item(item, value);
		obs_data_array_push_back(array, item);
	}

	obs_data_set_array(d, key, array);
}

void obs_data_set_json_object_item(obs_data_t *d, json j)
{
	for (auto& [key, value] : j.items()) {
		if (value.is_object()) {
			obs_data_set_json_object(d, key.c_str(), value);
		} else if (value.is_array()) {
			obs_data_set_json_array(d, key.c_str(), value);
		} else if (value.is_string()) {
			obs_data_set_string(d, key.c_str(), value.get<std::string>().c_str());
		} else if (value.is_number_integer()) {
			obs_data_set_int(d, key.c_str(), value.get<int64_t>());
		} else if (value.is_number_float()) {
			obs_data_set_double(d, key.c_str(), value.get<double>());
		} else if (value.is_boolean()) {
			obs_data_set_bool(d, key.c_str(), value.get<bool>());
		}
	}
}

obs_data_t *JsonUtils::JsonToObsData(json j)
{
	obs_data_t *data = obs_data_create();

	if (!j.is_object()) {
		obs_data_release(data);
		return nullptr;
	}

	obs_data_set_json_object_item(data, j);

	return data;
}

void set_json_string(json *j, const char *name, obs_data_item_t *item)
{
	const char *val = obs_data_item_get_string(item);
	j->emplace(name, val);
}
void set_json_number(json *j, const char *name, obs_data_item_t *item)
{
	enum obs_data_number_type type = obs_data_item_numtype(item);

	if (type == OBS_DATA_NUM_INT) {
		long long val = obs_data_item_get_int(item);
		j->emplace(name, val);
	} else {
		double val = obs_data_item_get_double(item);
		j->emplace(name, val);
	}
}
void set_json_bool(json *j, const char *name, obs_data_item_t *item)
{
	bool val = obs_data_item_get_bool(item);
	j->emplace(name, val);
}
void set_json_object(json *j, const char *name, obs_data_item_t *item, bool includeDefault)
{
	obs_data_t *obj = obs_data_item_get_obj(item);
	j->emplace(name, JsonUtils::ObsDataToJson(obj, includeDefault));
	obs_data_release(obj);
}
void set_json_array(json *j, const char *name, obs_data_item_t *item, bool includeDefault)
{
	json jArray = json::array();
	OBSDataArrayAutoRelease array = obs_data_item_get_array(item);
	size_t count = obs_data_array_count(array);

	for (size_t idx = 0; idx < count; idx++) {
		OBSDataAutoRelease subItem = obs_data_array_item(array, idx);
		json jItem = JsonUtils::ObsDataToJson(subItem, includeDefault);
		jArray.push_back(jItem);
	}

	j->emplace(name, jArray);
}

json JsonUtils::ObsDataToJson(obs_data_t *d, bool includeDefault)
{
	json j;
	obs_data_item_t *item = nullptr;

	for (item = obs_data_first(d); item; obs_data_item_next(&item)) {
		enum obs_data_type type = obs_data_item_gettype(item);
		const char *name = obs_data_item_get_name(item);

		if (!obs_data_item_has_user_value(item) && !includeDefault)
			continue;

		switch (type) {
			case OBS_DATA_STRING:
				set_json_string(&j, name, item);
				break;
			case OBS_DATA_NUMBER:
				set_json_number(&j, name, item);
				break;
			case OBS_DATA_BOOLEAN:
				set_json_bool(&j, name, item);
				break;
			case OBS_DATA_OBJECT:
				set_json_object(&j, name, item, includeDefault);
				break;
			case OBS_DATA_ARRAY:
				set_json_array(&j, name, item, includeDefault);
				break;
			default:
				;
		}
	}

	return j;
}