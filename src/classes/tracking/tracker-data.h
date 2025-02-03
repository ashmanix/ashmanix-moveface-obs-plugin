#ifndef TRACKERDATA_H
#define TRACKERDATA_H

#include <QString>
#include <QList>
#include <QSharedPointer>
#include <QJsonDocument>
#include <QMutex>
#include <QMutexLocker>

#include <obs.h>

#include "../../plugin-support.h"

// Forward declarations
class Pose;

class TrackerData {
public:
	TrackerData(QString trackerId = "", QString selectedImageSource = "", QString destIpAddress = "");
	~TrackerData() = default;

	// Implement custom copy constructor and copy assignment
	TrackerData(const TrackerData &other);
	TrackerData &operator=(const TrackerData &other);

	QString poseListToJsonString() const;
	void jsonStringToPoseList(const QString &jsonString);

	QString getTrackerId();
	QString getSelectedImageSource();
	QString getDestinationIpAddress();
	int getDestinationPort();
	int getPort();
	bool getIsEnabled();
	QSharedPointer<Pose> getPoseAt(int index);
	QList<QSharedPointer<Pose>> getPoseList();

	void setTrackerId(QString newId);
	void setSelectedImageSource(QString newImageSource);
	void setDestinationIpAddress(QString newIpAddress);
	void setDestinationPort(int newDestPort);
	void setPort(int newPort);
	void setIsEnabled(bool isEnabled);
	void setPoseAt(int index, QSharedPointer<Pose> newPose);

	void copyListToPoseList(QList<QSharedPointer<Pose>> newList);

private:
	mutable QMutex m_mutex; // Mutex to protect data

	QString m_trackerId;
	QString m_selectedImageSource;
	QString m_destIpAddress = "";
	int m_destPort = 21412;
	int m_port = 21412;
	bool m_isEnabled = false;
	QList<QSharedPointer<Pose>> m_poseList;
};

#endif // TRACKERDATA_H
