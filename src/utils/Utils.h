#pragma once

#include <nlohmann/json.hpp>
#include <obs-data.h>

using json = nlohmann::json;

namespace Utils {
	namespace Json {
		bool JsonArrayIsValidObsArray(json j);
		obs_data_t *JsonToObsData(json j);
		json ObsDataToJson(obs_data_t *d, bool includeDefault = false);
	};
};
