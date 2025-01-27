#include "tracker-worker.h"
#include <obs.h>
#include "plugin-support.h"
#include "../../classes/poses/pose.h"

TrackerWorker::TrackerWorker(quint16 port, const QString &destIpAddress, quint16 destPort, QObject *parent)
	: QObject(parent)
{
	// Initialize NetworkTracking without a parent; ownership managed manually
	networkTracking = QSharedPointer<NetworkTracking>::create(nullptr, port, destIpAddress, destPort);

	// Connect signals from NetworkTracking to Worker slots
	connect(networkTracking.data(), &NetworkTracking::receivedData, this, &TrackerWorker::processTrackingData);

	connect(networkTracking.data(), &NetworkTracking::connectionErrorToggle, this, &TrackerWorker::errorOccurred);

	connect(networkTracking.data(), &NetworkTracking::connectionToggle, this, &TrackerWorker::connectionToggle);
}

TrackerWorker::~TrackerWorker()
{
	stop();
}

void TrackerWorker::start()
{
	if (!running) {
		running = true;
		bool connected = networkTracking->startConnection();
		if (!connected) {
			emit errorOccurred("Failed to start network connection.");
		} else {
			obs_log(LOG_INFO, "Tracker worker started and network connection established.");
		}
	}
}

void TrackerWorker::stop()
{
	if (running) {
		running = false;
		if (networkTracking) {
			networkTracking->deleteLater();
			networkTracking = nullptr;
		}
		emit finished();
		obs_log(LOG_INFO, "Tracker worker stopped.");
	}
}

void TrackerWorker::processTrackingData(const VTubeStudioData &data)
{
	if (!running)
		return;

	QMutexLocker locker(&m_mutex);
	if (!m_trackerData) {
		emit errorOccurred("TrackerData is not set.");
		return;
	}

	QSharedPointer<Pose> selectedPose = findAppropriatePose(data);
	if (!selectedPose) {
		obs_log(LOG_WARNING, "No suitable pose found for the received tracking data.");
		return;
	}

	obs_log(LOG_INFO, "Pose: %s selected!", selectedPose->getPoseId().toStdString().c_str());

	// Perform data processing and image composition
	// Example placeholder for image composition logic
	QImage composedImage(800, 600, QImage::Format_ARGB32);
	composedImage.fill(Qt::transparent); // Start with a transparent image

	// TODO: Implement your actual image composition using 'data'
	// For demonstration, we'll just fill it with a solid color based on data
	if (data.getFaceFound()) {
		composedImage.fill(Qt::green);
	} else {
		composedImage.fill(Qt::red);
	}

	obs_log(LOG_INFO, "Data received!");

	// Emit the composed image to the UI thread
	emit imageReady(composedImage);
}

void TrackerWorker::updateConnection(quint16 newPort, const QString &newDestIpAddress, quint16 newDestPort)
{
	QMutexLocker locker(&m_mutex);
	if (networkTracking) {
		networkTracking->updateConnection(newPort, newDestIpAddress, newDestPort);
		obs_log(LOG_INFO, "Tracker worker connection updated to port: %d, IP: %s, dest port: %d", (int)newPort,
			newDestIpAddress.toStdString().c_str(), (int)newDestPort);
	} else {
		emit errorOccurred(true);
	}
}

void TrackerWorker::updateTrackerData(const QSharedPointer<TrackerData> &newTrackerData)
{
	QMutexLocker locker(&m_mutex);
	if (newTrackerData) {
		m_trackerData = newTrackerData;
		obs_log(LOG_INFO, "TrackerData has been updated in TrackerWorker.");
	} else {
		emit errorOccurred("Received null TrackerData.");
	}
}

// ---------------------------------- Private -------------------------------------

QSharedPointer<Pose> TrackerWorker::findAppropriatePose(const VTubeStudioData &data) const
{
	for (const QSharedPointer<Pose> &pose : m_trackerData->getPoseList()) {
		if (pose && pose->shouldUsePose(data.getBlendshapes())) {
			return pose;
		}
	}
	return QSharedPointer<Pose>();
}
