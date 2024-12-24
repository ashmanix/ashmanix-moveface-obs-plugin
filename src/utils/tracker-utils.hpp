#ifndef TRACKERUTILS_H
#define TRACKERUTILS_H

#include <QDateTime>
#include <QString>
#include <QUuid>
#include <QCryptographicHash>

struct TrackerDataStruct {
	QString trackerId;
};

struct Result {
	bool success;
	QString errorMessage;
};

QString GenerateUniqueID();


#endif // TRACKERUTILS_H
