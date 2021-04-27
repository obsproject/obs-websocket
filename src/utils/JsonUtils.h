#pragma once

#include <nlohmann/json.hpp>
#include <obs-data.h>

// For AutoRelease types
#include "../obs-websocket.h"

using json = nlohmann::json;

namespace JsonUtils {
	bool JsonArrayIsValidObsArray(json j);
	obs_data_t *JsonToObsData(json j);
	json ObsDataToJson(obs_data_t *d, bool includeDefault = false);
};
