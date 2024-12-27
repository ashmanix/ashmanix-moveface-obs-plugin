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
	SaveSettings();
	UnregisterAllHotkeys();
}

void MainWidgetDock::ConfigureWebSocketConnection()
{
	vendor = obs_websocket_register_vendor(VENDORNAME);

	if (!vendor) {
		obs_log(LOG_ERROR, "Error registering vendor to websocket!");
		return;
	}
}

Result MainWidgetDock::ValidateNewTrackerID(QString id)
{
	if (id.isEmpty())
		return {false, obs_module_text("DialogTrackerIdUpdateError")};

	bool duplicateCheck = trackerWidgetMap.value(id, nullptr) != nullptr ? true : false;
	if (duplicateCheck)
		return {false, obs_module_text("DialogTrackerDuplicateIdError")};

	return {true, ""};
}

Result MainWidgetDock::UpdateTrackerList(QString oldId, QString newId)
{
	FaceTracker *foundTracker = trackerWidgetMap.take(oldId);
	Result result = {false, ""};

	if (!foundTracker) {
		obs_log(LOG_ERROR, "Could not find tracker ID %s in saved list!", oldId.toStdString().c_str());
		result = {false, obs_module_text("DialogTrackerIdUpdateError")};
	} else {
		foundTracker->SetTrackerID(newId);
		trackerWidgetMap.insert(newId, foundTracker);
		result = {true, ""};
	}
	return result;
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

void MainWidgetDock::SaveSettings()
{
	obs_data_t *settings = obs_data_create();
	obs_data_array_t *obsDataArray = obs_data_array_create();

	QVBoxLayout *mainLayout = ui->trackerListLayout;

	for (int i = 0; i < mainLayout->count(); ++i) {
		QLayoutItem *item = mainLayout->itemAt(i);
		if (item) {
			FaceTracker *trackerWidget = qobject_cast<FaceTracker *>(item->widget());
			if (trackerWidget) {
				TrackerDataStruct *trackerData = trackerWidget->GetTrackerData();
				if (trackerData) {
					obs_data_t *dataObject = obs_data_create();
					trackerWidget->SaveTrackerWidgetDataToOBSSaveData(dataObject);
					obs_data_array_push_back(obsDataArray, dataObject);

					obs_data_release(dataObject);
				}
			}
		}
	}

	obs_data_set_array(settings, "tracker_widgets", obsDataArray);

	// ----------------------------------- Save Hotkeys -----------------------------------
	SaveHotkey(settings, addTrackerHotkeyId, addTrackerHotkeyName);
	// ------------------------------------------------------------------------------------

	char *file = obs_module_config_path(CONFIG);
	if (!obs_data_save_json(settings, file)) {
		char *path = obs_module_config_path("");
		if (path) {
			os_mkdirs(path);
			bfree(path);
		}
		obs_data_save_json(settings, file);
	}
	obs_data_array_release(obsDataArray);
	obs_data_release(settings);
	bfree(file);
}

int MainWidgetDock::GetNumberOfTimers()
{
	return static_cast<int>(trackerWidgetMap.size());
}

void MainWidgetDock::OBSFrontendEventHandler(enum obs_frontend_event event, void *private_data)
{

	MainWidgetDock *trackerWidgetDock = (MainWidgetDock *)private_data;

	switch (event) {
	case OBS_FRONTEND_EVENT_FINISHED_LOADING:
		MainWidgetDock::LoadSavedSettings(trackerWidgetDock);
		break;
	case OBS_FRONTEND_EVENT_THEME_CHANGED:
		MainWidgetDock::UpdateWidgetStyles(trackerWidgetDock);
		break;
		break;
	default:
		break;
	}
}

void MainWidgetDock::UpdateWidgetStyles(MainWidgetDock *trackerWidgetDock)
{
	int trackerWidgetCount = trackerWidgetDock->ui->trackerListLayout->count();
	for (int i = 0; i < trackerWidgetCount; i++) {
		// FaceTracker *trackerWidget = static_cast<FaceTracker *>(
		// 	trackerWidgetDock->ui->trackerListLayout->itemAt(i)
		// 		->widget());
		// if (trackerWidget) {
		// 	trackerWidget->UpdateStyles();
		// }
	}
}

void MainWidgetDock::RegisterAllHotkeys(obs_data_t *savedData)
{
	LoadHotkey(
		addTrackerHotkeyId, addTrackerHotkeyName, obs_module_text("AddTrackerHotkeyDescription"),
		[this]() { ui->addTrackerButton->click(); }, "Add Timer Hotkey Pressed", savedData);
}

void MainWidgetDock::UnregisterAllHotkeys()
{
	if (addTrackerHotkeyId)
		obs_hotkey_unregister(addTrackerHotkeyId);
}

void MainWidgetDock::AddTracker(obs_data_t *savedData)
{
	FaceTracker *newFaceTracker = new FaceTracker(this, savedData, this);

	trackerWidgetMap.insert(newFaceTracker->GetTrackerID(), newFaceTracker);
	ConnectTrackerSignalHandlers(newFaceTracker);

	trackerListLayout->addWidget(newFaceTracker);

	// UpdateTrackerListMoveButtonState();
}

void MainWidgetDock::LoadSavedSettings(MainWidgetDock *dockWidget)
{
	char *file = obs_module_config_path(CONFIG);
	obs_data_t *data = nullptr;
	if (file) {
		data = obs_data_create_from_json_file(file);
		bfree(file);
	}
	if (data) {
		// Get Save Data
		obs_data_array_t *timersArray = obs_data_get_array(data, "tracker_widgets");
		if (timersArray) {
			size_t count = obs_data_array_count(timersArray);
			for (size_t i = 0; i < count; ++i) {
				obs_data_t *timerDataObj = obs_data_array_item(timersArray, i);
				dockWidget->AddTracker(timerDataObj);
			}
		}

		// Add widget if none were loaded from save file
		// as we must have at least 1 timer
		if (dockWidget->GetNumberOfTimers() == 0) {
			dockWidget->AddTracker();
		}

		dockWidget->RegisterAllHotkeys(data);

		obs_data_release(data);
	}
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

	// ToggleUIForMultipleTrackers();
	// UpdateTrackerListMoveButtonState();
}
