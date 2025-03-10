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
	QObject::connect(networkTracking.data(), &NetworkTracking::receivedData, this,
			 &TrackerWorker::processTrackingData);

	QObject::connect(networkTracking.data(), &NetworkTracking::connectionErrorToggle, this,
			 &TrackerWorker::errorOccurred);

	QObject::connect(networkTracking.data(), &NetworkTracking::connectionToggle, this,
			 &TrackerWorker::connectionToggle);
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
		networkTracking = nullptr;
		obs_log(LOG_INFO, "Tracker worker stopped.");
	}
}

void TrackerWorker::processTrackingData(VTubeStudioData &data)
{
	if (!running)
		return;

	QMutexLocker locker(&m_mutex);

	// Use Kalman filters to smoothen blendshape changes
	smoothenBlendshapeData(data);

	// Emit data so that other widgets can read tracking data
	emit trackingDataReceived(data);

	if (!m_trackerData) {
		emit errorOccurred("TrackerData is not set.");
		return;
	}

	QSharedPointer<Pose> selectedPose = findAppropriatePose(data);
	if (!selectedPose) {
		obs_log(LOG_WARNING, "No suitable pose found for the received tracking data.");
		return;
	}

	QMap<BlendshapeKey, Blendshape> bsMap = data.getBlendshapes();
	Blendshape mouthOpenBlendshape = bsMap.value(BlendshapeKey::JAWOPEN);
	Blendshape tongueOutBlendshape = bsMap.value(BlendshapeKey::TONGUEOUT);
	// TODO: Use average of left and right for both eye and mouth smile
	Blendshape eyeBlinkBlendshape = bsMap.value(BlendshapeKey::EYEBLINK_L);
	Blendshape mouthSmileBlendshape = bsMap.value(BlendshapeKey::MOUTHSMILE_L);

	QImage composedImage = getPoseImageWithTracking(selectedPose, eyeBlinkBlendshape.m_value,
							mouthOpenBlendshape.m_value, mouthSmileBlendshape.m_value,
							tongueOutBlendshape.m_value);

	// Check if empty image and ignore if it is
	if (composedImage.isNull())
		return;

	// We store a cached image to compare the next image with
	// so that we only update the pose if there is a change
	if (cachedImage == composedImage)
		return;

	cachedImage = composedImage;

	obs_log(LOG_INFO, "Pose: %s selected!", selectedPose->getPoseId().toStdString().c_str());

	// Emit the composed image to the UI thread
	emit imageReady(&composedImage);
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

QImage TrackerWorker::getPoseImageWithTracking(QSharedPointer<Pose> pose, double in_eyeOpenPos = 0.0,
					       double in_mouthOpenPos = 0.0, double in_mouthSmilePos = 0.0,
					       double in_tongueOutPos = 0.0)
{

	double eyeOpenLimit = pose->getEyesOpenLimit();
	double eyeHalfOpenLimit = pose->getEyesHalfOpenLimit();
	double mouthOpenLimit = pose->getMouthOpenLimit();
	double mouthSmileLimit = pose->getSmileLimit();
	double tongueOutLimit = pose->getTongueOutLimit();

	PoseImageData *bodyImageData = pose->getPoseImageData(PoseImage::BODY);
	if (!bodyImageData || !bodyImageData->getPixmapItem()) {
		obs_log(LOG_WARNING, "Body image data is invalid.");
		return QImage();
	}

	PoseImage selectedEye = PoseImage::EYESCLOSED;
	if (in_eyeOpenPos > eyeOpenLimit) {
		selectedEye = PoseImage::EYESOPEN;
	} else if (in_eyeOpenPos > eyeHalfOpenLimit) {
		selectedEye = PoseImage::EYESHALFOPEN;
	}

	PoseImage selectedMouth = PoseImage::MOUTHCLOSED;
	if (in_tongueOutPos > tongueOutLimit) {
		selectedMouth = PoseImage::TONGUEOUT;
	} else if (in_mouthSmilePos > mouthSmileLimit) {
		selectedMouth = PoseImage::MOUTHSMILE;
	} else if (in_mouthOpenPos > mouthOpenLimit) {
		selectedMouth = PoseImage::MOUTHOPEN;
	}

	// Before building new image check if image settings matches previous settings
	// and if there's a match then return empty image
	PoseImageSettings currentSettings = {pose->getPoseId(), selectedEye, selectedMouth};
	if (hasPoseChanged(currentSettings))
		return QImage();

	// We set the new previous pose settings
	m_previousPose = currentSettings;

	PoseImageData *eyeImageData = pose->getPoseImageData(selectedEye);
	if (!eyeImageData || !eyeImageData->getPixmapItem()) {
		eyeImageData = pose->getPoseImageData(PoseImage::EYESCLOSED);
		// if (!eyeImageData || !eyeImageData->getPixmapItem()) {
		// 	obs_log(LOG_WARNING, "Eye image data is invalid for pose: %s",
		// 		pose->getPoseId().toStdString().c_str());
		// }
	}

	PoseImageData *mouthImageData = pose->getPoseImageData(selectedMouth);
	if (!mouthImageData || !mouthImageData->getPixmapItem()) {
		mouthImageData = pose->getPoseImageData(PoseImage::MOUTHCLOSED);
		// if (!mouthImageData || !mouthImageData->getPixmapItem()) {
		// 	obs_log(LOG_WARNING, "Mouth image data is invalid for pose: %s",
		// 		pose->getPoseId().toStdString().c_str());
		// }
	}

	QSharedPointer<MovablePixmapItem> bodyPixmapItem = bodyImageData->getPixmapItem();
	if (!bodyPixmapItem)
		bodyPixmapItem = QSharedPointer<MovablePixmapItem>::create();
	QPixmap bodyPixmap = bodyPixmapItem->pixmap();
	QPointF bodyPos = bodyPixmapItem->pos();

	QSharedPointer<MovablePixmapItem> eyePixmapItem = eyeImageData->getPixmapItem();
	if (!eyePixmapItem)
		eyePixmapItem = QSharedPointer<MovablePixmapItem>::create();
	QPixmap eyePixmap = eyePixmapItem->pixmap();
	QPointF eyePos = eyePixmapItem->pos();

	QSharedPointer<MovablePixmapItem> mouthPixmapItem = mouthImageData->getPixmapItem();
	if (!mouthPixmapItem)
		mouthPixmapItem = QSharedPointer<MovablePixmapItem>::create();
	QPixmap mouthPixmap = mouthPixmapItem->pixmap();
	QPointF mouthPos = mouthPixmapItem->pos();

	QRectF rectBody(bodyPos, bodyPixmap.size());
	QRectF rectEye(eyePos, eyePixmap.size());
	QRectF rectMouth(mouthPos, eyePixmap.size());

	QRectF boundingRect = rectBody.united(rectEye).united(rectMouth);

	// Add padding to image
	const int padding = 10;
	boundingRect.adjust(-padding, -padding, padding, padding);

	QSize finalSize = boundingRect.size().toSize();
	if (finalSize.isEmpty()) {
		obs_log(LOG_WARNING, "Final image size is empty for pose: %s", pose->getPoseId().toStdString().c_str());
		return QImage();
	}

	// Fill image with transparent background
	QImage finalImage(finalSize, QImage::Format_ARGB32);
	finalImage.fill(Qt::transparent);

	QPainter painter(&finalImage);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

	QPointF bodyDrawPos = bodyPos - boundingRect.topLeft();
	QPointF eyeDrawPos = eyePos - boundingRect.topLeft();
	QPointF mouthDrawPos = mouthPos - boundingRect.topLeft();

	painter.drawPixmap(bodyDrawPos, bodyPixmap);
	painter.drawPixmap(eyeDrawPos, eyePixmap);
	painter.drawPixmap(mouthDrawPos, mouthPixmap);

	painter.end();
	return finalImage;
}

bool TrackerWorker::hasPoseChanged(PoseImageSettings const &imageSettings)
{
	bool matchesPreviousVersion = false;

	if (imageSettings.poseId == m_previousPose.poseId &&
	    imageSettings.eyeBlendshape == m_previousPose.eyeBlendshape &&
	    imageSettings.mouthBlendshape == m_previousPose.mouthBlendshape)
		matchesPreviousVersion = true;

	return matchesPreviousVersion;
}

void TrackerWorker::smoothenBlendshapeData(VTubeStudioData &data)
{
	QMap<BlendshapeKey, Blendshape> bsMap = data.getBlendshapes();
	for (auto it = bsMap.begin(); it != bsMap.end(); ++it) {
		BlendshapeKey key = it.key();
		Blendshape blendshape = it.value();
		float rawValue = blendshape.m_value;

		// If the blendshape filter doesn't exist yet, create one
		if (m_blendshapeFilters.find(key) == m_blendshapeFilters.end()) {
			m_blendshapeFilters[key] = KalmanFilter(0.01f, 0.1f); // Default params
		}

		// Smooth the value
		float smoothedValue = m_blendshapeFilters[key].update(rawValue);
		blendshape.m_value = smoothedValue;

		// Update the blendshape's value in the QMap
		data.addBlendshape(key, blendshape);
	}
}
