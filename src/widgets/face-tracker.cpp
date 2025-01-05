#include "face-tracker.hpp"
#include "settings-dialog.hpp"
#include "../utils/pose-image-data.hpp"
#include "../utils/pose.hpp"

FaceTracker::FaceTracker(QWidget *parent, obs_data_t *savedData, MainWidgetDock *mDockWidget)
	: QWidget(parent),
	  ui(new Ui::FaceTracker)
{
	ui->setupUi(this);

	mainDockWidget = mDockWidget;
	trackerData = QSharedPointer<TrackerData>::create();

	if (savedData) {
		loadTrackerWidgetDataFromOBSSaveData(savedData);
	}

	if (trackerData->getTrackerId().size() == 0) {
		// Get a unique ID for the tracker
		trackerData->setTrackerId(GenerateUniqueID());
	}

	this->setProperty("id", trackerData->getTrackerId());

	setupWidgetUI();

	connectUISignalHandlers();
}

FaceTracker::~FaceTracker() {}

QString FaceTracker::getTrackerID()
{
	return trackerData->getTrackerId();
}

void FaceTracker::setTrackerID(const QString &newId)
{
	if (!newId.isEmpty()) {
		trackerData->setTrackerId(newId);
	}
	ui->trackerNameLabel->setText(trackerData->getTrackerId());
	this->setProperty("id", trackerData->getTrackerId());
}

TrackerData FaceTracker::getTrackerData()
{
	if (trackerData) {           // Always check if the pointer is not null
		return *trackerData; // Dereference to get a copy
	}
	return TrackerData();
}

void FaceTracker::saveTrackerWidgetDataToOBSSaveData(obs_data_t *dataObject)
{
	obs_data_set_string(dataObject, "trackerId", trackerData->getTrackerId().toStdString().c_str());
	obs_data_set_string(dataObject, "selectedImageSource",
			    trackerData->getSelectedImageSource().toStdString().c_str());

	obs_data_set_string(dataObject, "destIpAddress", trackerData->getDestinationIpAddress().toStdString().c_str());
	obs_data_set_int(dataObject, "destPort", trackerData->getDestinationPort());
	obs_data_set_int(dataObject, "port", trackerData->getPort());
	obs_data_set_bool(dataObject, "isEnabled", trackerData->getIsEnabled());

	QString poseListJson = trackerData->poseListToJsonString();
	obs_data_set_string(dataObject, "poseList", poseListJson.toStdString().c_str());
}

void FaceTracker::loadTrackerWidgetDataFromOBSSaveData(obs_data_t *dataObject)
{

	trackerData->setTrackerId((char *)obs_data_get_string(dataObject, "trackerId"));
	trackerData->setSelectedImageSource((char *)obs_data_get_string(dataObject, "selectedImageSource"));

	trackerData->setDestinationIpAddress((char *)obs_data_get_string(dataObject, "destIpAddress"));
	trackerData->setDestinationPort(obs_data_get_int(dataObject, "destPort"));
	trackerData->setPort(obs_data_get_int(dataObject, "port"));
	trackerData->setIsEnabled(obs_data_get_bool(dataObject, "isEnabled"));

	QString poseListString = (char *)obs_data_get_string(dataObject, "poseList");
	trackerData->jsonStringToPoseList(poseListString);

	setTrackerData();

	toggleEnabled(trackerData->getIsEnabled() ? Qt::Checked : Qt::Unchecked);
}

void FaceTracker::updateWidgetStyles()
{
	if (settingsDialogUi) {
		settingsDialogUi->updateStyledUIComponents();
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
	ui->menuToolButton->setMenu(toolButtonMenu);
	ui->menuToolButton->setPopupMode(QToolButton::InstantPopup);
	ui->menuToolButton->setArrowType(Qt::NoArrow);
	ui->menuToolButton->setText("");
	ui->menuToolButton->setEnabled(true);
	ui->menuToolButton->setToolTip(obs_module_text("TrackerSettingsButtonToolTip"));

	ui->menuToolButton->setStyleSheet("QToolButton::menu-indicator {"
					  "    image: none;"
					  "    width: 0px;"
					  "    height: 0px;"
					  "}");
	ui->errorLabel->setVisible(false);
	ui->errorLabel->setStyleSheet("QLabel { "
				      "  color:  rgb(192, 0, 0); "
				      "  border-radius: 6px; "
				      "}");

	ui->mainWidgetFrame->setProperty("class", "bg-base");

	ui->isEnabledCheckBox->setToolTip(obs_module_text("ToggleEnabledTickBoxToolTip"));

	ui->connectionLabel->setFixedSize(12, 12);

	setConnected(false);
}

void FaceTracker::connectUISignalHandlers()
{
	// QObject::connect(ui->deleteToolButton, &QPushButton::clicked, this, &FaceTracker::DeleteButtonClicked);

	// QObject::connect(ui->menuToolButton, &QPushButton::clicked, this, &FaceTracker::SettingsButtonClicked);

	QObject::connect(ui->isEnabledCheckBox, &QCheckBox::stateChanged, this, &FaceTracker::toggleEnabled);
}

void FaceTracker::setTrackerData()
{
	ui->trackerNameLabel->setText(trackerData->getTrackerId());
	ui->isEnabledCheckBox->setChecked(trackerData->getIsEnabled());
	loadPoseData();
}

void FaceTracker::loadPoseData()
{
	for (size_t i = 0; i < static_cast<size_t>(trackerData->getPoseList().size()); i++) {
		QSharedPointer<Pose> selectedPose = trackerData->getPoseAt(i);

		for (size_t i = 0; i < selectedPose->getPoseImageListSize(); ++i) {
			PoseImage poseEnum = static_cast<PoseImage>(i);
			PoseImageData data = selectedPose->getPoseImageAt(i);

			if (!data.getImageUrl().isEmpty()) {
				data.setPixmapItem(new MovablePixmapItem(poseEnum));

				QPixmap pixmap(data.getImageUrl());
				if (!pixmap.isNull()) {
					data.getPixmapItem()->setPixmap(pixmap);
				} else {
					obs_log(LOG_ERROR, "Failed to load pixmap from URL: %s",
						data.getImageUrl().toStdString().c_str());
					data.clearPixmapItem();
					continue;
				}

				switch (poseEnum) {
				case PoseImage::BODY:
					data.setImagePosition(selectedPose->getBodyPosition().getX(),
							      selectedPose->getBodyPosition().getY());
					break;
				case PoseImage::EYESOPEN:
				case PoseImage::EYESHALFOPEN:
				case PoseImage::EYESCLOSED:
					data.setImagePosition(selectedPose->getEyesPosition().getX(),
							      selectedPose->getEyesPosition().getY());
					break;
				case PoseImage::MOUTHCLOSED:
				case PoseImage::MOUTHOPEN:
				case PoseImage::MOUTHSMILE:
				case PoseImage::TONGUEOUT:
					data.setImagePosition(selectedPose->getMouthPosition().getX(),
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
	isConnected = isConnectedInput;

	if (isConnected) {
		ui->connectionLabel->setStyleSheet("QLabel { "
						   "  background-color:  rgb(0, 128, 0); "
						   "  border-radius: 6px; "
						   "}");
	} else {
		ui->connectionLabel->setStyleSheet("QLabel { "
						   "  background-color:  rgb(192, 0, 0); "
						   "  border-radius: 6px; "
						   "}");
	}
}

void FaceTracker::updateTrackerDataFromDialog(QSharedPointer<TrackerData> newData)
{
	if (newData->getDestinationIpAddress() != trackerData->getDestinationIpAddress() ||
	    newData->getDestinationPort() != trackerData->getDestinationPort() ||
	    newData->getPort() != trackerData->getPort()) {

		trackerData->setDestinationIpAddress(newData->getDestinationIpAddress());
		trackerData->setDestinationPort(newData->getDestinationPort());
		trackerData->setPort(newData->getPort());

		// Renew connection
		initiateNetworkTracking();
	}

	trackerData->setSelectedImageSource(newData->getSelectedImageSource());

	trackerData->copyListToPoseList(newData->getPoseList());

	mainDockWidget->updateTrackerList(trackerData->getTrackerId(), newData->getTrackerId());
}

void FaceTracker::initiateNetworkTracking()
{
	if (!trackerData->getIsEnabled())
		return;

	ui->errorLabel->setVisible(false);
	if (!networkTracking) {
		networkTracking = new NetworkTracking(this, trackerData->getPort(),
						      trackerData->getDestinationIpAddress(),
						      trackerData->getDestinationPort());
		QObject::connect(networkTracking, &NetworkTracking::receivedData, this,
				 &FaceTracker::handleTrackingData);

		QObject::connect(networkTracking, &NetworkTracking::connectionToggle, this, &FaceTracker::setConnected);

		QObject::connect(networkTracking, &NetworkTracking::connectionErrorToggle, this,
				 &FaceTracker::toggleConnectionError);
	} else {
		networkTracking->updateConnection(trackerData->getPort(), trackerData->getDestinationIpAddress(),
						  trackerData->getDestinationPort());
	}
}

void FaceTracker::enableTimer()
{
	trackerData->setIsEnabled(true);
	ui->trackerNameLabel->setEnabled(true);
	initiateNetworkTracking();
}

void FaceTracker::disableTimer()
{
	trackerData->setIsEnabled(false);
	ui->trackerNameLabel->setEnabled(false);
	if (networkTracking) {
		networkTracking->deleteLater();
		networkTracking = nullptr;
		setConnected(false);
	}
}

// ------------------------------- Private Slots ----------------------------------

void FaceTracker::settingsActionSelected()
{
	if (!settingsDialogUi) {
		settingsDialogUi = new SettingsDialog(this, trackerData, mainDockWidget);
		QObject::connect(settingsDialogUi, &SettingsDialog::settingsUpdated, this,
				 &FaceTracker::updateTrackerDataFromDialog);

		settingsDialogUi->setStyleSheet("QPushButton {"
						"   width: auto;"
						"   height: auto;"
						"   padding: 4px 8px;"
						"   margin: 0;"
						"}");
	}
	if (settingsDialogUi->isVisible()) {
		settingsDialogUi->raise();
		settingsDialogUi->activateWindow();
	} else {
		settingsDialogUi->setVisible(true);
	}
}

void FaceTracker::deleteActionSelected()
{
	emit requestDelete(trackerData->getTrackerId());
}

void FaceTracker::handleTrackingData(VTubeStudioData data)
{
	UNUSED_PARAMETER(data);

	// ResetConnectionTimer();

	obs_log(LOG_INFO, "data received!");
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
	ui->errorLabel->setVisible(isError);
}
