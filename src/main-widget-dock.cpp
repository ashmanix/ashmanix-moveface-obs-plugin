#include "main-widget-dock.hpp"
#include "widgets/face-tracker.hpp"

MainWidgetDock::MainWidgetDock(QWidget *parent) : OBSDock(parent), ui(new Ui::MainWidget)
{
	// Register custom type for signals and slots
	qRegisterMetaType<obs_data_t *>("obs_data_t*");

	ui->setupUi(this);
	trackerListLayout = ui->trackerListLayout;

	SetupCountdownWidgetUI();
	// resize(300, 380);

	obs_frontend_add_event_callback(OBSFrontendEventHandler, this);

	ConnectUISignalHandlers();
}

MainWidgetDock::~MainWidgetDock()
{
	// SaveSettings();
	// UnregisterAllHotkeys();
}

void MainWidgetDock::ConfigureWebSocketConnection()
{
	vendor = obs_websocket_register_vendor(VENDORNAME);

	if (!vendor) {
		obs_log(LOG_ERROR, "Error registering vendor to websocket!");
		return;
	}
}

void MainWidgetDock::SetupCountdownWidgetUI()
{
	ui->addTrackerButton->setProperty("themeID", "addIconSmall");
	ui->addTrackerButton->setProperty("class", "icon-plus");
	ui->addTrackerButton->setEnabled(true);
	ui->addTrackerButton->setToolTip(obs_module_text("AddTrackerButtonTip"));

	this->setStyleSheet("#dialogMainWidget QDialogButtonBox QPushButton {"
			    "   width: auto;"
			    "   height: auto;"
			    "   padding: 4px 8px;"
			    "   margin: 0;"
			    "}"
			    "#mainTrackerWidget {"
			    "   border-left: none;"
			    "   border-right: none;"
			    "}");
}

void MainWidgetDock::ConnectUISignalHandlers()
{
	QObject::connect(ui->addTrackerButton, &QPushButton::clicked, this, &MainWidgetDock::AddTrackerButtonClicked);
}

void MainWidgetDock::ConnectTrackerSignalHandlers(FaceTracker *faceTracker)
{
	connect(faceTracker, &FaceTracker::RequestDelete, this, &MainWidgetDock::RemoveTrackerButtonClicked);
}

void MainWidgetDock::OBSFrontendEventHandler(enum obs_frontend_event event, void *private_data)
{

	MainWidgetDock *mainDockWidget = (MainWidgetDock *)private_data;

	switch (event) {
	case OBS_FRONTEND_EVENT_FINISHED_LOADING:
		// MainWidgetDock::LoadSavedSettings(mainDockWidget);
		break;
	case OBS_FRONTEND_EVENT_THEME_CHANGED:
		MainWidgetDock::UpdateWidgetStyles(mainDockWidget);
		break;
	default:
		break;
	}
}

void MainWidgetDock::UpdateWidgetStyles(MainWidgetDock *countdownDockWidget)
{
	int trackerWidgetCount = countdownDockWidget->ui->trackerListLayout->count();
	for (int i = 0; i < trackerWidgetCount; i++) {
		// AshmanixTimer *timerWidget = static_cast<AshmanixTimer *>(
		// 	countdownDockWidget->ui->trackerListLayout->itemAt(i)
		// 		->widget());
		// if (timerWidget) {
		// 	timerWidget->UpdateStyles();
		// }
	}
}

void MainWidgetDock::AddTracker(obs_data_t *savedData)
{
	FaceTracker *newFaceTracker = new FaceTracker(this, savedData, this);

	trackerWidgetMap.insert(newFaceTracker->GetTrackerID(), newFaceTracker);
	ConnectTrackerSignalHandlers(newFaceTracker);

	trackerListLayout->addWidget(newFaceTracker);

	// UpdateTrackerListMoveButtonState();
}

// --------------------------------- Private Slots ----------------------------------

void MainWidgetDock::AddTrackerButtonClicked()
{
	AddTracker();
}

void MainWidgetDock::RemoveTrackerButtonClicked(QString id)
{
	FaceTracker *itemToBeRemoved = trackerWidgetMap.value(id, nullptr);

	if (itemToBeRemoved) {
		itemToBeRemoved->deleteLater();
		trackerWidgetMap.remove(id);
		obs_log(LOG_INFO, (QString("Tracker %1 deleted").arg(id)).toStdString().c_str());
	}

	// ToggleUIForMultipleTimers();
	// UpdateTimerListMoveButtonState();
}
