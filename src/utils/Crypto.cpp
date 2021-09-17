#include <QByteArray>
#include <QCryptographicHash>
#include <QRandomGenerator>

#include "Crypto.h"
#include "../plugin-macros.generated.h"

static const char allowedChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
static const int allowedCharsCount = static_cast<int>(sizeof(allowedChars) - 1);

std::string Utils::Crypto::GenerateSalt()
{
	// Get OS seeded random number generator
	QRandomGenerator *rng = QRandomGenerator::global();

	// Generate 32 random chars
	const size_t randomCount = 32;
	QByteArray randomChars;
	for (size_t i = 0; i < randomCount; i++)
		randomChars.append((char)rng->bounded(255));

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
	// Get OS random number generator
	QRandomGenerator *rng = QRandomGenerator::system();

	// Fill string with random alphanumeric
	QString ret;
	for (size_t i = 0; i < length; i++)
		ret += allowedChars[rng->bounded(0, allowedCharsCount)];

	return ret;
}
