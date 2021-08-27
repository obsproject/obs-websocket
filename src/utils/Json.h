#pragma once

#include <string>
#include <obs.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Utils {
	namespace Json {
		bool JsonArrayIsValidObsArray(json j);
		obs_data_t *JsonToObsData(json j);
		json ObsDataToJson(obs_data_t *d, bool includeDefault = false);
		bool GetJsonFileContent(std::string fileName, json &content);
		bool SetJsonFileContent(std::string fileName, json content, bool createNew = true);
	}
}