#include <QByteArray>
#include <QCryptographicHash>

#include "Utils.h"

#include "../plugin-macros.generated.h"

std::string Utils::Crypto::GenerateSalt()
{
	// Generate 32 random chars
	const size_t randomCount = 32;
	QByteArray randomChars;
	for (size_t i = 0; i < randomCount; i++) {
		randomChars.append((char)qrand());
	}

	// Convert the 32 random chars to a base64 string
	return randomChars.toBase64().toStdString();
}

std::string Utils::Crypto::GenerateSecret(std::string password, std::string salt)
{
	// Concatenate the password and the salt
	QString passAndSalt = "";
	passAndSalt += QString::fromStdString(password);
	passAndSalt += QString::fromStdString(salt);

	// Generate a SHA256 hash of the password and salt
	auto challengeHash = QCryptographicHash::hash(
		passAndSalt.toUtf8(),
		QCryptographicHash::Algorithm::Sha256
	);

	// Encode SHA256 hash to Base64
	return challengeHash.toBase64().toStdString();
}

bool Utils::Crypto::CheckAuthenticationString(std::string secret, std::string challenge, std::string authenticationString)
{
	// Concatenate auth secret with the challenge sent to the user
	QString secretAndChallenge = "";
	secretAndChallenge += QString::fromStdString(secret);
	secretAndChallenge += QString::fromStdString(challenge);

	// Generate a SHA256 hash of secretAndChallenge
	auto hash = QCryptographicHash::hash(
		secretAndChallenge.toUtf8(),
		QCryptographicHash::Algorithm::Sha256
	);

	// Encode the SHA256 hash to Base64
	std::string expectedAuthenticationString = hash.toBase64().toStdString();

	return (authenticationString == expectedAuthenticationString);
}

QString Utils::Crypto::GeneratePassword(size_t length)
{
	QString ret;
	int rand;

	for (size_t i = 0; i < length; i++) {
		while (true) {
			rand = qrand() % ((0x7a + 1) - 0x30) + 0x30;
			if (
				(rand >= 0x30 && rand <= 0x39) ||
				(rand >= 0x41 && rand <= 0x5A) ||
				(rand >= 0x61 && rand <= 0x7A)
			)
				break;
		}

		ret += QString(rand);
	}

	return ret;
}
