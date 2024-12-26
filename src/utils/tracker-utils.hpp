#ifndef TRACKERUTILS_H
#define TRACKERUTILS_H

#include <QDateTime>
#include <QString>
#include <QUuid>
#include <QCryptographicHash>

struct TrackerDataStruct {
	QString trackerId;
	QString destIpAddress = "255.255.255.255";
	int destPort = 21412;
	int port = 21412;
};

struct Result {
	bool success;
	QString errorMessage;
};

QString GenerateUniqueID();


#endif // TRACKERUTILS_H
