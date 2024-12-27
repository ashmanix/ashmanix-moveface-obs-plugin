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

	obs_data_set_string(dataObject, "destIpAddress", trackerData.destIpAddress.toStdString().c_str());
	obs_data_set_int(dataObject, "destPort", trackerData.destPort);
	obs_data_set_int(dataObject, "port", trackerData.port);
	obs_data_set_bool(dataObject, "isEnabled", trackerData.isEnabled);
}

void FaceTracker::LoadTrackerWidgetDataFromOBSSaveData(obs_data_t *dataObject)
{

	trackerData.trackerId = (char *)obs_data_get_string(dataObject, "trackerId");

	trackerData.destIpAddress = (char *)obs_data_get_string(dataObject, "destIpAddress");
	trackerData.destPort = obs_data_get_int(dataObject, "destPort");
	trackerData.port = obs_data_get_int(dataObject, "port");
	trackerData.isEnabled = obs_data_get_bool(dataObject, "isEnabled");

	SetTrackerData();

	ToggleEnabled(trackerData.isEnabled ? Qt::Checked : Qt::Unchecked);
}

// ---------------------------------- Private -------------------------------------

void FaceTracker::SetupWidgetUI()
{
	SetTrackerID();

	ui->menuToolButton->setProperty("themeID", "menuIconSmall");
	ui->menuToolButton->setProperty("class", "icon-dots-vert");
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
				      "  background-color:  rgb(192, 0, 0); "
				      "  border-radius: 6px; "
				      "}");

	ui->connectionLabel->setFixedSize(12, 12);
	//Tool button only used as an icon so we permanantly set it to disabled
	SetConnected(false);

	ui->mainWidgetFrame->setProperty("class", "bg-base");
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
		InitiateNetworkTracking(true);
	}

	mainDockWidget->UpdateTrackerList(trackerData.trackerId, newData->trackerId);
}

void FaceTracker::InitiateNetworkTracking(bool shouldSendRequest)
{
	if (!trackerData.isEnabled)
		return;

	ui->errorLabel->setVisible(false);
	if (!networkTracking) {
		networkTracking = new NetworkTracking(this, trackerData.port);
		QObject::connect(networkTracking, &NetworkTracking::ReceivedData, this,
				 &FaceTracker::HandleTrackingData);
	} else {
		networkTracking->UpdateConnection(trackerData.port);
	}

	if (shouldSendRequest) {
		// We send out the signal periodically as required by vTubeStudio to continue
		// receiving tracking data
		if (!networkTrackingDataRequestTimer) {
			networkTrackingDataRequestTimer = new QTimer();
			QObject::connect(networkTrackingDataRequestTimer, &QTimer::timeout, this,
					 &FaceTracker::RequestTrackingData);
		}
		//Reset timer
		networkTrackingDataRequestTimer->stop();
		int timePeriod = trackingDataPeriodInSecs - 1 >= 0 ? (trackingDataPeriodInSecs - 1) * 1000 : 1000;
		networkTrackingDataRequestTimer->start(timePeriod);
	}
}

void FaceTracker::ResetConnectionTimer()
{
	if (!isConnected)
		SetConnected(true);

	// We send out the signal periodically as required by vTubeStudio
	if (!connectionTimer) {
		connectionTimer = new QTimer();
		QObject::connect(connectionTimer, &QTimer::timeout, [this]() { SetConnected(false); });
	}
	//Reset timer
	connectionTimer->stop();
	int timePeriod = trackingDataPeriodInSecs - 1 >= 0 ? (trackingDataPeriodInSecs - 1) * 1000 : 1000;
	connectionTimer->start(timePeriod);
}

void FaceTracker::EnableTimer()
{
	trackerData.isEnabled = true;
	ui->trackerNameLabel->setEnabled(true);
	InitiateNetworkTracking(true);
}

void FaceTracker::DisableTimer()
{
	trackerData.isEnabled = false;
	ui->trackerNameLabel->setEnabled(false);
	if (networkTrackingDataRequestTimer) {
		networkTrackingDataRequestTimer->stop();
	}
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

void FaceTracker::HandleTrackingData(QString data)
{
	UNUSED_PARAMETER(data);

	ResetConnectionTimer();

	obs_log(LOG_INFO, data.toStdString().c_str());
}

void FaceTracker::RequestTrackingData()
{
	if (trackerData.isEnabled && !trackerData.destIpAddress.isEmpty()) {
		QJsonArray ports;
		ports.append(trackerData.port);
		initiateTrackingObject["ports"] = ports;

		QJsonDocument jsonDoc(initiateTrackingObject);
		QByteArray jsonString = jsonDoc.toJson(QJsonDocument::Indented);
		bool result = networkTracking->SendUDPData(trackerData.destIpAddress, trackerData.destPort, jsonString);
		if (!result) {
			networkErrorCount++;
		}
		if (networkErrorCount >= MAXERRORCOUNT) {
			ui->errorLabel->setVisible(true);
			obs_log(LOG_WARNING, "Send connection max errors reached, cancelling send messages");
			networkTrackingDataRequestTimer->stop();
		}
	}
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
