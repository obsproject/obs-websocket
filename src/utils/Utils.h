#pragma once

#include <string>
#include <QString>
#include <nlohmann/json.hpp>
#include <obs-data.h>

using json = nlohmann::json;

namespace Utils {
	namespace Json {
		bool JsonArrayIsValidObsArray(json j);
		obs_data_t *JsonToObsData(json j);
		json ObsDataToJson(obs_data_t *d, bool includeDefault = false);
	}

	namespace Crypto {
		std::string GenerateSalt();
		std::string GenerateSecret(std::string password, std::string salt);
		bool CheckAuthenticationString(std::string secret, std::string challenge, std::string authenticationString);
		QString GeneratePassword(size_t length = 16);
	}

	namespace Platform {
		std::string GetLocalAddress();
		QString GetCommandLineArgument(QString arg);
	}

	namespace Obs {
		;
	}
}
