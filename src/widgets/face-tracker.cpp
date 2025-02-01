#include "face-tracker.h"
#include "./settings-dialog/settings-dialog.h"
#include "../classes/poses/pose-image-data.h"
#include "../classes/poses/pose.h"

FaceTracker::FaceTracker(QWidget *parent, obs_data_t *savedData, MainWidgetDock *mDockWidget)
	: QWidget(parent),
	  m_ui(new Ui::FaceTracker)
{
	m_ui->setupUi(this);

	m_mainDockWidget = mDockWidget;
	m_trackerData = QSharedPointer<TrackerData>::create();

	if (savedData) {
		loadTrackerWidgetDataFromOBSSaveData(savedData);
	}

	if (m_trackerData->getTrackerId().size() == 0) {
		// Get a unique ID for the tracker
		m_trackerData->setTrackerId(generateUniqueID());
	}

	this->setProperty("id", m_trackerData->getTrackerId());

	setupWidgetUI();

	connectUISignalHandlers();
}

FaceTracker::~FaceTracker()
{
	m_trackerWorker->stop();
	delete m_ui;
}

QString FaceTracker::getTrackerID()
{
	return m_trackerData->getTrackerId();
}

void FaceTracker::setTrackerID(const QString &newId)
{
	if (!newId.isEmpty()) {
		m_trackerData->setTrackerId(newId);
	}
	m_ui->trackerNameLabel->setText(m_trackerData->getTrackerId());
	this->setProperty("id", m_trackerData->getTrackerId());
}

TrackerData FaceTracker::getTrackerData()
{
	if (m_trackerData) {           // Always check if the pointer is not null
		return *m_trackerData; // Dereference to get a copy
	}
	return TrackerData();
}

void FaceTracker::saveTrackerWidgetDataToOBSSaveData(obs_data_t *dataObject)
{
	obs_data_set_string(dataObject, "trackerId", m_trackerData->getTrackerId().toStdString().c_str());
	obs_data_set_string(dataObject, "selectedImageSource",
			    m_trackerData->getSelectedImageSource().toStdString().c_str());

	obs_data_set_string(dataObject, "destIpAddress",
			    m_trackerData->getDestinationIpAddress().toStdString().c_str());
	obs_data_set_int(dataObject, "destPort", m_trackerData->getDestinationPort());
	obs_data_set_int(dataObject, "port", m_trackerData->getPort());
	obs_data_set_bool(dataObject, "isEnabled", m_trackerData->getIsEnabled());

	QString poseListJson = m_trackerData->poseListToJsonString();
	obs_data_set_string(dataObject, "poseList", poseListJson.toStdString().c_str());
}

void FaceTracker::loadTrackerWidgetDataFromOBSSaveData(obs_data_t *dataObject)
{

	m_trackerData->setTrackerId((char *)obs_data_get_string(dataObject, "trackerId"));
	m_trackerData->setSelectedImageSource((char *)obs_data_get_string(dataObject, "selectedImageSource"));

	m_trackerData->setDestinationIpAddress((char *)obs_data_get_string(dataObject, "destIpAddress"));
	m_trackerData->setDestinationPort(obs_data_get_int(dataObject, "destPort"));
	m_trackerData->setPort(obs_data_get_int(dataObject, "port"));
	m_trackerData->setIsEnabled(obs_data_get_bool(dataObject, "isEnabled"));

	QString poseListString = (char *)obs_data_get_string(dataObject, "poseList");
	m_trackerData->jsonStringToPoseList(poseListString);

	setTrackerData();

	toggleEnabled(m_trackerData->getIsEnabled() ? Qt::Checked : Qt::Unchecked);
}

void FaceTracker::updateWidgetStyles()
{
	if (m_settingsDialogUi) {
		m_settingsDialogUi->updateStyledUIComponents();
	}
}

// ---------------------------------- Private -------------------------------------

void FaceTracker::setupWidgetUI()
{
	setTrackerID();

	QMenu *toolButtonMenu = new QMenu(this);
	toolButtonMenu->addAction("Settings", this, &FaceTracker::settingsActionSelected);
	toolButtonMenu->addSeparator();
	toolButtonMenu->addAction("Delete", this, &FaceTracker::deleteActionSelected);
	m_ui->menuToolButton->setMenu(toolButtonMenu);
	m_ui->menuToolButton->setPopupMode(QToolButton::InstantPopup);
	m_ui->menuToolButton->setArrowType(Qt::NoArrow);
	m_ui->menuToolButton->setText("");
	m_ui->menuToolButton->setEnabled(true);
	m_ui->menuToolButton->setToolTip(obs_module_text("TrackerSettingsButtonToolTip"));

	m_ui->errorLabel->setVisible(false);

	m_ui->mainWidgetFrame->setProperty("class", "bg-base");

	m_ui->isEnabledCheckBox->setToolTip(obs_module_text("ToggleEnabledTickBoxToolTip"));

	m_ui->connectionLabel->setFixedSize(12, 12);

	setConnected(false);
}

void FaceTracker::connectUISignalHandlers()
{
	QObject::connect(m_ui->isEnabledCheckBox, &QCheckBox::stateChanged, this, &FaceTracker::toggleEnabled);
	QObject::connect(m_trackerWorker, &TrackerWorker::imageReady, this, &FaceTracker::handleDisplayNewImage);
}

void FaceTracker::setTrackerData()
{
	m_ui->trackerNameLabel->setText(m_trackerData->getTrackerId());
	m_ui->isEnabledCheckBox->setChecked(m_trackerData->getIsEnabled());
	loadPoseData();
}

void FaceTracker::loadPoseData()
{
	for (qsizetype i = 0; i < m_trackerData->getPoseList().size(); i++) {
		QSharedPointer<Pose> selectedPose = m_trackerData->getPoseAt(i);

		for (size_t j = 0; j < selectedPose->getPoseImageListSize(); ++j) {
			PoseImage poseEnum = static_cast<PoseImage>(j);
			PoseImageData *data = selectedPose->getPoseImageAt(j);

			if (!data->getImageUrl().isEmpty()) {
				data->setPixmapItem(QSharedPointer<MovablePixmapItem>(new MovablePixmapItem(poseEnum)));

				QPixmap pixmap(data->getImageUrl());
				if (!pixmap.isNull()) {
					data->getPixmapItem()->setPixmap(pixmap);
				} else {
					obs_log(LOG_ERROR, "Failed to load pixmap from URL: %s",
						data->getImageUrl().toStdString().c_str());
					data->clearPixmapItem();
					continue;
				}

				switch (poseEnum) {
				case PoseImage::BODY:
					data->setImagePosition(selectedPose->getBodyPosition().getX(),
							       selectedPose->getBodyPosition().getY());
					break;
				case PoseImage::EYESOPEN:
				case PoseImage::EYESHALFOPEN:
				case PoseImage::EYESCLOSED:
					data->setImagePosition(selectedPose->getEyesPosition().getX(),
							       selectedPose->getEyesPosition().getY());
					break;
				case PoseImage::MOUTHCLOSED:
				case PoseImage::MOUTHOPEN:
				case PoseImage::MOUTHSMILE:
				case PoseImage::TONGUEOUT:
					data->setImagePosition(selectedPose->getMouthPosition().getX(),
							       selectedPose->getMouthPosition().getY());
					break;
				default:
					break;
				}
			}
		}
	}
}

void FaceTracker::setConnected(bool isConnectedInput)
{
	m_isConnected = isConnectedInput;

	if (m_isConnected) {
		m_ui->connectionLabel->setStyleSheet("QLabel { "
						     "  background-color:  rgb(0, 128, 0); "
						     "  border-radius: 6px; "
						     "}");
	} else {
		m_ui->connectionLabel->setStyleSheet("QLabel { "
						     "  background-color:  rgb(192, 0, 0); "
						     "  border-radius: 6px; "
						     "}");
	}
}

void FaceTracker::updateTrackerDataFromDialog(QSharedPointer<TrackerData> newData)
{
	if (newData->getDestinationIpAddress() != m_trackerData->getDestinationIpAddress() ||
	    newData->getDestinationPort() != m_trackerData->getDestinationPort() ||
	    newData->getPort() != m_trackerData->getPort() || newData->getPoseList() != m_trackerData->getPoseList()) {

		m_trackerData->setDestinationIpAddress(newData->getDestinationIpAddress());
		m_trackerData->setDestinationPort(newData->getDestinationPort());
		m_trackerData->setPort(newData->getPort());

		m_trackerData->copyListToPoseList(newData->getPoseList());

		// Renew connection
		initiateTracking();
	}

	m_trackerData->setSelectedImageSource(newData->getSelectedImageSource());
	m_mainDockWidget->updateTrackerList(m_trackerData->getTrackerId(), newData->getTrackerId());
}

void FaceTracker::initiateTracking()
{
	if (!m_trackerData->getIsEnabled())
		return;

	m_ui->errorLabel->setVisible(false);
	if (!m_trackerWorker) {
		m_trackerWorker = new TrackerWorker(m_trackerData->getPort(), m_trackerData->getDestinationIpAddress(),
						    m_trackerData->getDestinationPort(), this);

		QObject::connect(m_trackerWorker, &TrackerWorker::connectionToggle, this, &FaceTracker::setConnected);

		QObject::connect(m_trackerWorker, &TrackerWorker::errorOccurred, this,
				 &FaceTracker::toggleConnectionError);

		// Set tracker data for worker
		m_trackerWorker->start();
	} else {
		m_trackerWorker->updateConnection(m_trackerData->getPort(), m_trackerData->getDestinationIpAddress(),
						  m_trackerData->getDestinationPort());
	}
	m_trackerWorker->updateTrackerData(m_trackerData);
}

void FaceTracker::enableTimer()
{
	m_trackerData->setIsEnabled(true);
	m_ui->trackerNameLabel->setEnabled(true);
	initiateTracking();
}

void FaceTracker::disableTimer()
{
	m_trackerData->setIsEnabled(false);
	m_ui->trackerNameLabel->setEnabled(false);
	if (m_trackerWorker) {
		m_trackerWorker->finished();
		m_trackerWorker = nullptr;
		setConnected(false);
	}
}

// ------------------------------- Private Slots ----------------------------------

void FaceTracker::settingsActionSelected()
{
	if (!m_settingsDialogUi) {
		m_settingsDialogUi = QSharedPointer<SettingsDialog>::create(this, m_trackerData, m_mainDockWidget);
		QObject::connect(m_settingsDialogUi.data(), &SettingsDialog::settingsUpdated, this,
				 &FaceTracker::updateTrackerDataFromDialog);
	}
	if (m_settingsDialogUi->isVisible()) {
		m_settingsDialogUi->raise();
		m_settingsDialogUi->activateWindow();
	} else {
		m_settingsDialogUi->setVisible(true);
	}
}

void FaceTracker::deleteActionSelected()
{
	emit requestDelete(m_trackerData->getTrackerId());
}

void FaceTracker::handleTrackingData(VTubeStudioData data)
{
	UNUSED_PARAMETER(data);
	obs_log(LOG_INFO, "data received!");
}

void FaceTracker::handleDisplayNewImage(QImage imageData)
{
	UNUSED_PARAMETER(imageData);
	obs_log(LOG_INFO, "Image data received!");
}

void FaceTracker::toggleEnabled(int checkState)
{
	switch (checkState) {
	case Qt::Checked:
		enableTimer();
		break;

	case Qt::Unchecked:
	default:
		disableTimer();
		break;
	}
}

void FaceTracker::toggleConnectionError(bool isError)
{
	m_ui->errorLabel->setVisible(isError);
}
