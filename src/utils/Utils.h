#pragma once

#include <QString>
#include <nlohmann/json.hpp>
#include <obs-data.h>

using json = nlohmann::json;

namespace Utils {
	namespace Json {
		bool JsonArrayIsValidObsArray(json j);
		obs_data_t *JsonToObsData(json j);
		json ObsDataToJson(obs_data_t *d, bool includeDefault = false);
	};

	namespace Crypto {
		QString GenerateSalt();
		QString GenerateSecret(QString password, QString salt);
		bool CheckAuthenticationString(QString secret, QString challenge, QString authenticationString);
	};
};
