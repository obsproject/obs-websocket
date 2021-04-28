#include <QByteArray>
#include <QCryptographicHash>

#include "Utils.h"

#include "../plugin-macros.generated.h"

QString Utils::Crypto::GenerateSalt()
{
	// Generate 32 random chars
	const size_t randomCount = 32;
	QByteArray randomChars;
	for (size_t i = 0; i < randomCount; i++) {
		randomChars.append((char)qrand());
	}

	// Convert the 32 random chars to a base64 string
	return randomChars.toBase64();
}

QString Utils::Crypto::GenerateSecret(QString password, QString salt)
{
	// Concatenate the password and the salt
	QString passAndSalt = "";
	passAndSalt += password;
	passAndSalt += salt;

	// Generate a SHA256 hash of the password and salt
	auto challengeHash = QCryptographicHash::hash(
		passAndSalt.toUtf8(),
		QCryptographicHash::Algorithm::Sha256
	);

	// Encode SHA256 hash to Base64
	return challengeHash.toBase64();
}

bool Utils::Crypto::CheckAuthenticationString(QString secret, QString challenge, QString authenticationString)
{
	// Concatenate auth secret with the challenge sent to the user
	QString secretAndChallenge = "";
	secretAndChallenge += secret;
	secretAndChallenge += challenge;

	// Generate a SHA256 hash of secretAndChallenge
	auto hash = QCryptographicHash::hash(
		secretAndChallenge.toUtf8(),
		QCryptographicHash::Algorithm::Sha256
	);

	// Encode the SHA256 hash to Base64
	QString expectedAuthenticationString = hash.toBase64();

	return (authenticationString == expectedAuthenticationString);
}
