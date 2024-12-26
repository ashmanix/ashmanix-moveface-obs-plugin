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
}

void FaceTracker::LoadTrackerWidgetDataFromOBSSaveData(obs_data_t *dataObject)
{

	trackerData.trackerId = (char *)obs_data_get_string(dataObject, "trackerId");

	trackerData.destIpAddress = (char *)obs_data_get_string(dataObject, "destIpAddress");
	trackerData.destPort = obs_data_get_int(dataObject, "destPort");
	trackerData.port = obs_data_get_int(dataObject, "port");

	SetTrackerData();

	InitiateNetworkConnection(true);
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

	ui->connectionLabel->setFixedSize(12, 12);
	//Tool button only used as an icon so we permanantly set it to disabled
	SetConnectionLabel(false);

	ui->mainWidgetFrame->setProperty("class", "bg-base");
}

void FaceTracker::ConnectUISignalHandlers()
{
	// QObject::connect(ui->deleteToolButton, &QPushButton::clicked, this, &FaceTracker::DeleteButtonClicked);

	// QObject::connect(ui->menuToolButton, &QPushButton::clicked, this, &FaceTracker::SettingsButtonClicked);
}

void FaceTracker::SetTrackerData()
{
	ui->trackerNameLabel->setText(trackerData.trackerId);
}

void FaceTracker::SetConnectionLabel(bool isConnected)
{
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
		InitiateNetworkConnection(true);
	}

	mainDockWidget->UpdateTrackerList(trackerData.trackerId, newData->trackerId);
}

void FaceTracker::InitiateNetworkConnection(bool shouldSendRequest)
{
	if (!networkConnection) {
		networkConnection = new NetworkConnection(this, trackerData.port);
		QObject::connect(networkConnection, &NetworkConnection::ReceivedData, this,
				 &FaceTracker::HandleTrackingData);
	} else {
		networkConnection->UpdateConnection(trackerData.port);
	}

	if (shouldSendRequest) {
		QJsonArray ports;
		ports.append(trackerData.port);
		initiateTrackingObject["ports"] = ports;

		QJsonDocument jsonDoc(initiateTrackingObject);
		QByteArray jsonString = jsonDoc.toJson(QJsonDocument::Indented);

        obs_log(LOG_INFO, "Here first: %s", jsonString.toStdString().c_str());

		networkConnection->SendUDPData(trackerData.destIpAddress, trackerData.destPort, jsonString);
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

	obs_log(LOG_INFO, data.toStdString().c_str());
	obs_log(LOG_INFO, "Yo");
}
