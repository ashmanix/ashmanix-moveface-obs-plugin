#include "tracker-utils.hpp"

QString GenerateUniqueID()
{
	QUuid uuid = QUuid::createUuid();
	QByteArray hash = QCryptographicHash::hash(uuid.toByteArray(), QCryptographicHash::Md5);
	return QString(hash.toHex().left(8)); // We take the first 8 characters of the hash
}

bool FileExists(QString path)
{
	QFileInfo check_file(path);
	// check if file exists and if yes: Is it really a file and not a directory?
	if (check_file.exists() && check_file.isFile()) {
		return true;
	} else {
		return false;
	}
}
