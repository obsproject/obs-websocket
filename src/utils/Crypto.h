#pragma once

#include <string>
#include <QString>

namespace Utils {
	namespace Crypto {
		std::string GenerateSalt();
		std::string GenerateSecret(std::string password, std::string salt);
		bool CheckAuthenticationString(std::string secret, std::string challenge, std::string authenticationString);
		std::string GeneratePassword(size_t length = 16);
	}
}
