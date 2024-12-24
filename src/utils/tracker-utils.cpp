#include "tracker-utils.hpp"


QString GenerateUniqueID()
{
	QUuid uuid = QUuid::createUuid();
	QByteArray hash = QCryptographicHash::hash(uuid.toByteArray(), QCryptographicHash::Md5);
	return QString(hash.toHex().left(8)); // We take the first 8 characters of the hash
}
