/*
obs-websocket
Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <mbedtls/base64.h>
#include <mbedtls/sha256.h>
#include <obs-frontend-api.h>
#include "Config.h"

#define CONFIG_SECTION_NAME "obs-websocket"
#define CONFIG_PARAM_SECRET "auth_hash"
#define CONFIG_PARAM_SALT "auth_salt"
#define CONFIG_PARAM_AUTHREQUIRED "auth_required"

Config *Config::_instance = new Config();

Config::Config() {
	// Default settings
	AuthRequired = false;
	Secret = "";
	Salt = "";
	SettingsLoaded = false;

	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_init(&rng);
	mbedtls_ctr_drbg_seed(&rng, mbedtls_entropy_func, &entropy, nullptr, 0);
	//mbedtls_ctr_drbg_set_prediction_resistance(&rng, MBEDTLS_CTR_DRBG_PR_ON);

	SessionChallenge = GenerateSalt();
}

Config::~Config() {
	mbedtls_ctr_drbg_free(&rng);
	mbedtls_entropy_free(&entropy);
}

const char* Config::GenerateSalt() {
	// Generate 32 random chars
	unsigned char *random_chars = (unsigned char *)bzalloc(32);
	mbedtls_ctr_drbg_random(&rng, random_chars, 32);

	// Convert the 32 random chars to a base64 string
	unsigned char *salt = (unsigned char*)bzalloc(64);
	size_t salt_bytes;
	mbedtls_base64_encode(salt, 64, &salt_bytes, random_chars, 32);
	salt[salt_bytes] = 0; // Null-terminate the string

	bfree(random_chars);
	return (char *)salt;
}

const char* Config::GenerateSecret(const char *password, const char *salt) {
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
	const char *new_salt = GenerateSalt();
	const char *new_challenge = GenerateSecret(password, new_salt);

	this->Salt = new_salt;
	this->Secret = new_challenge;
}

bool Config::CheckAuth(const char *response) {
	size_t secretLength = strlen(this->Secret);
	size_t sessChallengeLength = strlen(this->SessionChallenge);
	
	// Concatenate auth secret with the challenge sent to the user
	char *challengeAndResponse = (char*)bzalloc(secretLength + sessChallengeLength);
	memcpy(challengeAndResponse, this->Secret, secretLength);
	memcpy(challengeAndResponse + secretLength, this->SessionChallenge, sessChallengeLength);
	challengeAndResponse[secretLength + sessChallengeLength] = 0; // Null-terminate the string

	// Generate a SHA256 hash of challengeAndResponse
	unsigned char *hash = (unsigned char*)bzalloc(32);
	mbedtls_sha256((unsigned char*)challengeAndResponse, secretLength + sessChallengeLength, hash, 0);

	// Encode the SHA256 hash to Base64
	unsigned char *expected_response = (unsigned char*)bzalloc(64);
	size_t base64_size = 0;
	mbedtls_base64_encode(expected_response, 64, &base64_size, hash, 32);
	expected_response[64] = 0; // Null-terminate the string

	if (strcmp((char*)expected_response, response) == 0) {
		SessionChallenge = GenerateSalt();
		return true;
	}
	else {
		return false;
	}
}

void Config::OBSSaveCallback(obs_data_t *save_data, bool saving, void *private_data) {
	Config *conf = static_cast<Config *>(private_data);

	if (saving) {
		obs_data_t *settings = obs_data_create();
		obs_data_set_bool(settings, CONFIG_PARAM_AUTHREQUIRED, conf->AuthRequired);
		obs_data_set_string(settings, CONFIG_PARAM_SECRET, conf->Secret);
		obs_data_set_string(settings, CONFIG_PARAM_SALT, conf->Salt);

		obs_data_set_obj(save_data, CONFIG_SECTION_NAME, settings);
	}
	else {
		obs_data_t *settings = obs_data_get_obj(save_data, CONFIG_SECTION_NAME);
		if (settings) {
			conf->AuthRequired = obs_data_get_bool(settings, CONFIG_PARAM_AUTHREQUIRED);
			conf->Secret = obs_data_get_string(settings, CONFIG_PARAM_SECRET);
			conf->Salt = obs_data_get_string(settings, CONFIG_PARAM_SALT);

			conf->SettingsLoaded = true;
		}
	}
}

Config* Config::Current() {
	return _instance;
}
