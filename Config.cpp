#include <mbedtls/base64.h>
#include <mbedtls/sha256.h>
#include <obs-frontend-api.h>
#include "Config.h"

#define CONFIG_SECTION_NAME "obs-websocket"
#define CONFIG_PARAM_CHALLENGE "auth_hash"
#define CONFIG_PARAM_SALT "auth_salt"
#define CONFIG_PARAM_AUTHREQUIRED "auth_required"

Config *Config::_instance = new Config();

Config::Config() {
	// Default settings
	AuthRequired = false;
	Challenge = "";
	Salt = "";
	SettingsLoaded = false;

	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_init(&rng);
	mbedtls_ctr_drbg_seed(&rng, mbedtls_entropy_func, &entropy, nullptr, 0);
	//mbedtls_ctr_drbg_set_prediction_resistance(&rng, MBEDTLS_CTR_DRBG_PR_ON);
}

Config::~Config() {
	mbedtls_ctr_drbg_free(&rng);
	mbedtls_entropy_free(&entropy);
}

const char* Config::GenerateSalt(mbedtls_ctr_drbg_context *rng) {
	// Generate 32 random chars
	unsigned char *random_chars = (unsigned char *)bzalloc(32);
	mbedtls_ctr_drbg_random(rng, random_chars, 32);

	// Convert the 32 random chars to a base64 string
	unsigned char *salt = (unsigned char*)bzalloc(64);
	size_t salt_bytes;
	mbedtls_base64_encode(salt, 64, &salt_bytes, random_chars, 32);
	salt[salt_bytes] = 0; // Null-terminate the string

	bfree(random_chars);
	return (char *)salt;
}

const char* Config::GenerateChallenge(const char *password, const char *salt) {
	size_t passwordLength = strlen(password);
	size_t saltLength = strlen(salt);

	// Concatenate the password and the salt
	unsigned char *passAndSalt = (unsigned char*)bzalloc(passwordLength + saltLength);
	memcpy(passAndSalt, password, passwordLength);
	memcpy(passAndSalt + passwordLength, salt, saltLength);
	passAndSalt[passwordLength + saltLength] = 0; // Null-terminate the string

	// Generate a SHA256 hash of the password
	unsigned char *challengeHash = (unsigned char *)bzalloc(32);
	mbedtls_sha256(passAndSalt, passwordLength + saltLength, challengeHash, 0);
	
	// Encode SHA256 hash to Base64
	unsigned char *challenge = (unsigned char*)bzalloc(64);
	size_t challenge_bytes = 0;
	mbedtls_base64_encode(challenge, 64, &challenge_bytes, challengeHash, 32);
	challenge[64] = 0; // Null-terminate the string

	bfree(passAndSalt);
	bfree(challengeHash);
	return (char*)challenge;
}

void Config::SetPassword(const char *password) {
	const char *new_salt = GenerateSalt(&rng);
	const char *new_challenge = GenerateChallenge(password, new_salt);

	this->Salt = new_salt;
	this->Challenge = new_challenge;
}

bool Config::CheckAuth(const char *response) {
	size_t challengeLength = strlen(this->Challenge);
	size_t responseLength = strlen(response);
	
	// Concatenate challenge and auth response
	char *challengeAndResponse = (char*)bzalloc(challengeLength + responseLength);
	memcpy(challengeAndResponse, this->Challenge, challengeLength);
	memcpy(challengeAndResponse + challengeLength, response, responseLength);
	challengeAndResponse[challengeLength + responseLength] = 0; // Null-terminate the string

	// Generate a SHA256 hash of challengeAndResponse
	unsigned char *hash = (unsigned char*)bzalloc(32);
	mbedtls_sha256((unsigned char*)challengeAndResponse, challengeLength + responseLength, hash, 0);

	// Encode the SHA256 hash to Base64
	unsigned char *expected_response = (unsigned char*)bzalloc(64);
	size_t base64_size = 0;
	mbedtls_base64_encode(expected_response, 64, &base64_size, hash, 32);
	expected_response[64] = 0; // Null-terminate the string

	return (strcmp((char*)expected_response, response) == 0);
}

void Config::OBSSaveCallback(obs_data_t *save_data, bool saving, void *private_data) {
	Config *conf = static_cast<Config *>(private_data);

	if (saving) {
		obs_data_t *settings = obs_data_create();
		obs_data_set_bool(settings, CONFIG_PARAM_AUTHREQUIRED, conf->AuthRequired);
		obs_data_set_string(settings, CONFIG_PARAM_CHALLENGE, conf->Challenge);
		obs_data_set_string(settings, CONFIG_PARAM_SALT, conf->Salt);

		obs_data_set_obj(save_data, CONFIG_SECTION_NAME, settings);

		obs_data_release(settings);
	}
	else {
		obs_data_t *settings = obs_data_get_obj(save_data, CONFIG_SECTION_NAME);
		if (settings) {
			conf->AuthRequired = obs_data_get_bool(settings, CONFIG_PARAM_AUTHREQUIRED);
			conf->Challenge = obs_data_get_string(settings, CONFIG_PARAM_CHALLENGE);
			conf->Salt = obs_data_get_string(settings, CONFIG_PARAM_SALT);

			conf->SettingsLoaded = true;
		}
		obs_data_release(settings);
	}
}

Config* Config::Current() {
	return _instance;
}
