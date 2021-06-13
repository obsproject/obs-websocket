#pragma once

#include <obs.hpp>
#include <string>
#include <QString>
#include <QSystemTrayIcon>
#include <nlohmann/json.hpp>

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
		bool GetCommandLineFlagSet(QString arg);
		void SendTrayNotification(QSystemTrayIcon::MessageIcon icon, QString title, QString body);
	}

	namespace Obs {
		namespace StringHelper {
			std::string GetObsVersionString();
			std::string GetCurrentSceneCollection();
			std::string GetCurrentProfile();
			std::string GetSourceTypeString(obs_source_t *source);
			std::string GetInputMonitorTypeString(obs_source_t *input);
			std::string GetMediaInputStateString(obs_source_t *input);
			std::string GetLastReplayBufferFilePath();
		}

		namespace ListHelper {
			std::vector<std::string> GetSceneCollectionList();
			std::vector<std::string> GetProfileList();
			std::vector<obs_hotkey_t *> GetHotkeyList();
			std::vector<std::string> GetHotkeyNameList();
			std::vector<json> GetSceneList();
			std::vector<json> GetSceneItemList(obs_scene_t *scene, bool basic = false);
			std::vector<json> GetTransitionList();
		}

		namespace SearchHelper {
			obs_hotkey_t *GetHotkeyByName(std::string name);
		}
	}
}
