#ifndef CONFIG_H
#define CONFIG_H

#include <obs-module.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

class Config {
	public:
		Config();
		~Config();
		void SetPassword(const char *password);
		bool CheckAuth(const char *userChallenge);
		static const char* GenerateSalt(mbedtls_ctr_drbg_context *rng);
		static const char* GenerateChallenge(const char *password, const char *salt);
		static void OBSSaveCallback(obs_data_t *save_data, bool saving, void *);

		bool AuthRequired;
		const char *Challenge;
		const char *Salt;
		bool SettingsLoaded;
		
		static Config* Current();

	private:
		static Config *_instance;
		mbedtls_entropy_context entropy;
		mbedtls_ctr_drbg_context rng;
};

#endif // CONFIG_H