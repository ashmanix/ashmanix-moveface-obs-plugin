#ifndef TRACKERWORKER_H
#define TRACKERWORKER_H

#include <QObject>
#include <QList>
#include <QImage>
#include <QSharedPointer>
#include <QMutex>

#include <obs.h>
#include <graphics/graphics.h>

#include "network-tracking.h"
#include "../../classes/tracking/vtube-studio-data.h"
#include "../../classes/tracking/tracker-data.h"
#include "../../classes/poses/pose-image.h"
#include "../../obs/my-gs-texture-wrapper.h"

// The purpose of this tracker worker is to receive data and process pose images
// on a separate thread to the main UI thread to prevent any lag on the UI
class TrackerWorker : public QObject {
	Q_OBJECT
public:
	explicit TrackerWorker(quint16 port, const QString &destIpAddress, quint16 destPort, QObject *parent = nullptr);
	~TrackerWorker() override;

public slots:
	void start();
	void stop();
	void processTrackingData(const VTubeStudioData &data);

	// New slots to update settings
	void updateConnection(quint16 newPort, const QString &newDestIpAddress, quint16 newDestPort);
	void updateTrackerData(const QSharedPointer<TrackerData> &newTrackerData);

signals:
	void imageReady(QImage *image); // Signal emitted when image is ready
	// void finished();
	void errorOccurred(bool isError);
	void connectionToggle(bool isConnected);
	void trackingDataReceived(VTubeStudioData data);

private:
	struct PoseImageSettings {
		QString poseId = "";
		PoseImage eyeBlendshape;
		PoseImage mouthBlendshape;
	};

	PoseImageSettings m_previousPose;
	QSharedPointer<NetworkTracking> networkTracking;
	bool running = false;
	QMutex m_mutex; // Mutex to protect settings

	QSharedPointer<TrackerData> m_trackerData;
	QSharedPointer<Pose> findAppropriatePose(const VTubeStudioData &data) const;
	QImage getPoseImageWithTracking(QSharedPointer<Pose> pose, double in_eyeOpenPos, double in_mouthOpenPos,
					double in_mouthSmilePos, double in_tongueOutPos);

	bool hasPoseChanged(PoseImageSettings const &imageSettings);
};

#endif // TRACKERWORKER_H
