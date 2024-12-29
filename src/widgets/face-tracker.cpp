#include "face-tracker.hpp"
#include "settings-dialog.hpp"

FaceTracker::FaceTracker(QWidget *parent, obs_data_t *savedData, MainWidgetDock *mDockWidget)
	: QWidget(parent),
	  ui(new Ui::FaceTracker)
{
	ui->setupUi(this);

	mainDockWidget = mDockWidget;
	trackerData = TrackerDataStruct();

	if (savedData) {
		LoadTrackerWidgetDataFromOBSSaveData(savedData);
	}

	if (trackerData.trackerId.size() == 0) {
		// Get a unique ID for the tracker
		trackerData.trackerId = GenerateUniqueID();
	}

	this->setProperty("id", trackerData.trackerId);

	SetupWidgetUI();

	ConnectUISignalHandlers();
}

FaceTracker::~FaceTracker() {}

QString FaceTracker::GetTrackerID()
{
	return trackerData.trackerId;
}

void FaceTracker::SetTrackerID(const QString &newId)
{
	if (!newId.isEmpty()) {
		trackerData.trackerId = newId;
	}
	ui->trackerNameLabel->setText(trackerData.trackerId);
	this->setProperty("id", trackerData.trackerId);
}

TrackerDataStruct *FaceTracker::GetTrackerData()
{
	return &trackerData;
}

void FaceTracker::SaveTrackerWidgetDataToOBSSaveData(obs_data_t *dataObject)
{
	obs_data_set_string(dataObject, "trackerId", trackerData.trackerId.toStdString().c_str());
	obs_data_set_string(dataObject, "selectedImageSource", trackerData.selectedImageSource.toStdString().c_str());

	obs_data_set_string(dataObject, "destIpAddress", trackerData.destIpAddress.toStdString().c_str());
	obs_data_set_int(dataObject, "destPort", trackerData.destPort);
	obs_data_set_int(dataObject, "port", trackerData.port);
	obs_data_set_bool(dataObject, "isEnabled", trackerData.isEnabled);

	QString poseListJson = trackerData.poseListToJsonString();
	obs_data_set_string(dataObject, "poseList", poseListJson.toStdString().c_str());
}

void FaceTracker::LoadTrackerWidgetDataFromOBSSaveData(obs_data_t *dataObject)
{

	trackerData.trackerId = (char *)obs_data_get_string(dataObject, "trackerId");
	trackerData.selectedImageSource = (char *)obs_data_get_string(dataObject, "selectedImageSource");

	trackerData.destIpAddress = (char *)obs_data_get_string(dataObject, "destIpAddress");
	trackerData.destPort = obs_data_get_int(dataObject, "destPort");
	trackerData.port = obs_data_get_int(dataObject, "port");
	trackerData.isEnabled = obs_data_get_bool(dataObject, "isEnabled");

	QString poseListString = (char *)obs_data_get_string(dataObject, "poseList");
	trackerData.jsonStringToPoseList(poseListString);

	SetTrackerData();

	ToggleEnabled(trackerData.isEnabled ? Qt::Checked : Qt::Unchecked);
}

// ---------------------------------- Private -------------------------------------

void FaceTracker::SetupWidgetUI()
{
	SetTrackerID();

	QMenu *toolButtonMenu = new QMenu(this);
	toolButtonMenu->addAction("Settings", this, SLOT(SettingsActionSelected()));
	toolButtonMenu->addSeparator();
	toolButtonMenu->addAction("Delete", this, SLOT(DeleteActionSelected()));
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

	SetConnected(false);
}

void FaceTracker::ConnectUISignalHandlers()
{
	// QObject::connect(ui->deleteToolButton, &QPushButton::clicked, this, &FaceTracker::DeleteButtonClicked);

	// QObject::connect(ui->menuToolButton, &QPushButton::clicked, this, &FaceTracker::SettingsButtonClicked);

	QObject::connect(ui->isEnabledCheckBox, &QCheckBox::stateChanged, this, &FaceTracker::ToggleEnabled);
}

void FaceTracker::SetTrackerData()
{
	ui->trackerNameLabel->setText(trackerData.trackerId);
	ui->isEnabledCheckBox->setChecked(trackerData.isEnabled);
}

void FaceTracker::SetConnected(bool isConnectedInput)
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

void FaceTracker::UpdateTrackerDataFromDialog(TrackerDataStruct *newData)
{
	if (newData->destIpAddress != trackerData.destIpAddress || newData->destPort != trackerData.destPort ||
	    newData->port != trackerData.port) {

		trackerData.destIpAddress = newData->destIpAddress;
		trackerData.destPort = newData->destPort;
		trackerData.port = newData->port;

		// Renew connection
		InitiateNetworkTracking();
	}

	trackerData.selectedImageSource = newData->selectedImageSource;

	mainDockWidget->UpdateTrackerList(trackerData.trackerId, newData->trackerId);
}

void FaceTracker::InitiateNetworkTracking()
{
	if (!trackerData.isEnabled)
		return;

	ui->errorLabel->setVisible(false);
	if (!networkTracking) {
		networkTracking =
			new NetworkTracking(this, trackerData.port, trackerData.destIpAddress, trackerData.destPort);
		QObject::connect(networkTracking, &NetworkTracking::ReceivedData, this,
				 &FaceTracker::HandleTrackingData);

		QObject::connect(networkTracking, &NetworkTracking::ConnectionToggle, this, &FaceTracker::SetConnected);

		QObject::connect(networkTracking, &NetworkTracking::ConnectionErrorToggle, this,
				 &FaceTracker::ToggleConnectionError);
	} else {
		networkTracking->UpdateConnection(trackerData.port, trackerData.destIpAddress, trackerData.destPort);
	}
}

void FaceTracker::EnableTimer()
{
	trackerData.isEnabled = true;
	ui->trackerNameLabel->setEnabled(true);
	InitiateNetworkTracking();
}

void FaceTracker::DisableTimer()
{
	trackerData.isEnabled = false;
	ui->trackerNameLabel->setEnabled(false);
	if (networkTracking) {
		networkTracking->deleteLater();
		networkTracking = nullptr;
		SetConnected(false);
	}
}

// ------------------------------- Private Slots ----------------------------------

void FaceTracker::SettingsActionSelected()
{
	if (!settingsDialogUi) {
		settingsDialogUi = new SettingsDialog(this, &trackerData, mainDockWidget);
		QObject::connect(settingsDialogUi, &SettingsDialog::SettingsUpdated, this,
				 &FaceTracker::UpdateTrackerDataFromDialog);

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

void FaceTracker::DeleteActionSelected()
{
	emit RequestDelete(trackerData.trackerId);
}

void FaceTracker::HandleTrackingData(VTubeStudioTrackingData data)
{
	UNUSED_PARAMETER(data);

	// ResetConnectionTimer();

	obs_log(LOG_INFO, "data received!");
}

void FaceTracker::ToggleEnabled(int checkState)
{
	switch (checkState) {
	case Qt::Checked:
		EnableTimer();
		break;

	case Qt::Unchecked:
	default:
		DisableTimer();
		break;
	}
}

void FaceTracker::ToggleConnectionError(bool isError)
{
	ui->errorLabel->setVisible(isError);
}
