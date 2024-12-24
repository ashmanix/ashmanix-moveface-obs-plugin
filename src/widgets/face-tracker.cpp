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
		LoadTimerWidgetDataFromOBSSaveData(savedData);
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

void FaceTracker::SetupWidgetUI()
{
	ui->settingsToolButton->setProperty("themeID", "propertiesIconSmall");
	ui->settingsToolButton->setProperty("class", "icon-gear");
	ui->settingsToolButton->setEnabled(true);
	ui->settingsToolButton->setToolTip(obs_module_text("TrackerSettingsButtonToolTip"));

	ui->deleteToolButton->setProperty("themeID", "removeIconSmall");
	ui->deleteToolButton->setProperty("class", "icon-trash");
	ui->deleteToolButton->setText("");
	ui->deleteToolButton->setEnabled(true);
	ui->deleteToolButton->setToolTip(obs_module_text("DeleteTrackerButtonTip"));
}

void FaceTracker::ConnectUISignalHandlers()
{
	QObject::connect(ui->deleteToolButton, &QPushButton::clicked, this, &FaceTracker::DeleteButtonClicked);

	QObject::connect(ui->settingsToolButton, &QPushButton::clicked, this, &FaceTracker::SettingsButtonClicked);
}

void FaceTracker::SetTrackerData()
{
	ui->trackerNameLabel->setText(trackerData.trackerId);
}

void FaceTracker::LoadTimerWidgetDataFromOBSSaveData(obs_data_t *dataObject)
{

	trackerData.trackerId = (char *)obs_data_get_string(dataObject, "trackerId");

	SetTrackerData();
}

// ------------------------------- Private Slots ----------------------------------

void FaceTracker::SettingsButtonClicked()
{
	if (!settingsDialogUi) {
		settingsDialogUi = new SettingsDialog(this, &trackerData, mainDockWidget);
	}
	if (settingsDialogUi->isVisible()) {
		settingsDialogUi->raise();
		settingsDialogUi->activateWindow();
	} else {
		settingsDialogUi->setVisible(true);
	}
}

void FaceTracker::DeleteButtonClicked()
{
	emit RequestDelete(trackerData.trackerId);
}
