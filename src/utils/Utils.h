#pragma once

#include <string>
#include <QString>
#include <QSystemTrayIcon>
#include <obs.hpp>
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
			std::string GetOutputTimecodeString(obs_output_t *output);
		}

		namespace NumberHelper {
			uint64_t GetOutputDuration(obs_output_t *output);
		}

		namespace ListHelper {
			std::vector<std::string> GetSceneCollectionList();
			std::vector<std::string> GetProfileList();
			std::vector<obs_hotkey_t *> GetHotkeyList();
			std::vector<std::string> GetHotkeyNameList();
			std::vector<json> GetSceneList();
			std::vector<json> GetSceneItemList(obs_scene_t *scene, bool basic = false);
			std::vector<json> GetTransitionList();
			std::vector<json> GetInputList(std::string inputKind = "");
			std::vector<std::string> GetInputKindList(bool unversioned = false, bool includeDisabled = false);
		}

		namespace SearchHelper {
			obs_hotkey_t *GetHotkeyByName(std::string name);
		}

		namespace ActionHelper {
			obs_sceneitem_t *CreateSceneItem(obs_source_t *input, obs_scene_t *scene, bool sceneItemEnabled = true);
			obs_sceneitem_t *CreateInput(std::string inputName, std::string inputKind, obs_data_t *inputSettings, obs_scene_t *scene, bool sceneItemEnabled = true);
		}
	}
}
