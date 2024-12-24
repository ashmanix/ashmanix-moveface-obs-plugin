#include "settings-dialog.hpp"

SettingsDialog::SettingsDialog(QWidget *parent, TrackerDataStruct *tData, MainWidgetDock *mWidget)
	: QDialog(parent),
	  ui(new Ui::FaceTrackerDialog)
{
	ui->setupUi(this);
	trackerData = tData;
	mainWidget = mWidget;
	SetTitle();

	// SetupDialogUI(timerData);

	// ConnectUISignalHandlers();
}

SettingsDialog::~SettingsDialog()
{
	this->deleteLater();
}

void SettingsDialog::SetTitle()
{
	QString dialogTitle = QString("Tracker %1").arg(trackerData->trackerId);
	this->setWindowTitle(dialogTitle);
}
