#include "main-widget-dock.h"
#include "./widgets/face-tracker.h"

MainWidgetDock::MainWidgetDock(QWidget *parent) : OBSDock(parent), m_ui(new Ui::MainWidget)
{
	// Register custom type for signals and slots
	qRegisterMetaType<obs_data_t *>("obs_data_t*");

	m_ui->setupUi(this);
	trackerListLayout = m_ui->trackerListLayout;

	setupCountdownWidgetUI();

	obs_frontend_add_event_callback(obsFrontendEventHandler, this);

	connectUISignalHandlers();
}

MainWidgetDock::~MainWidgetDock()
{
	saveSettings();
	unregisterAllHotkeys();
}

void MainWidgetDock::configureWebSocketConnection()
{
	m_vendor = obs_websocket_register_vendor(VENDORNAME);

	if (!m_vendor) {
		obs_log(LOG_ERROR, "Error registering m_vendor to websocket!");
		return;
	}
}

Result MainWidgetDock::validateNewTrackerID(QString id)
{
	if (id.isEmpty())
		return {false, obs_module_text("DialogTrackerIdUpdateError")};

	bool duplicateCheck = m_trackerWidgetMap.value(id, nullptr) != nullptr ? true : false;
	if (duplicateCheck)
		return {false, obs_module_text("DialogTrackerDuplicateIdError")};

	return {true, ""};
}

Result MainWidgetDock::updateTrackerList(QString oldId, QString newId)
{
	FaceTracker *foundTracker = m_trackerWidgetMap.take(oldId);
	Result result = {false, ""};

	if (!foundTracker) {
		obs_log(LOG_ERROR, "Could not find tracker ID %s in saved list!", oldId.toStdString().c_str());
		result = {false, obs_module_text("DialogTrackerIdUpdateError")};
	} else {
		foundTracker->setTrackerID(newId);
		m_trackerWidgetMap.insert(newId, foundTracker);
		result = {true, ""};
	}
	return result;
}

void MainWidgetDock::setupCountdownWidgetUI()
{
	m_ui->addTrackerButton->setEnabled(true);
	m_ui->addTrackerButton->setToolTip(obs_module_text("AddTrackerButtonToolTip"));

	this->setStyleSheet(getStyleSheetDataFromFilePath("main-styles.qss"));
}

void MainWidgetDock::connectUISignalHandlers()
{
	QObject::connect(m_ui->addTrackerButton, &QPushButton::clicked, this, &MainWidgetDock::addTrackerButtonClicked);
}

void MainWidgetDock::connectTrackerSignalHandlers(FaceTracker *faceTracker)
{
	connect(faceTracker, &FaceTracker::requestDelete, this, &MainWidgetDock::removeTrackerButtonClicked);
}

void MainWidgetDock::saveSettings()
{
	obs_data_t *settings = obs_data_create();
	obs_data_array_t *obsDataArray = obs_data_array_create();

	QVBoxLayout *mainLayout = m_ui->trackerListLayout;

	for (int i = 0; i < mainLayout->count(); ++i) {
		QLayoutItem *item = mainLayout->itemAt(i);
		if (item) {
			FaceTracker *trackerWidget = qobject_cast<FaceTracker *>(item->widget());
			if (trackerWidget) {
				TrackerData trackerData = trackerWidget->getTrackerData();
				obs_data_t *dataObject = obs_data_create();
				trackerWidget->saveTrackerWidgetDataToOBSSaveData(dataObject);
				obs_data_array_push_back(obsDataArray, dataObject);

				obs_data_release(dataObject);
			}
		}
	}

	obs_data_set_array(settings, "tracker_widgets", obsDataArray);

	// ----------------------------------- Save Hotkeys -----------------------------------
	saveHotkey(settings, m_addTrackerHotkeyId, ADDTRACKERHOTKEYNAME);
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

int MainWidgetDock::getNumberOfTimers()
{
	return static_cast<int>(m_trackerWidgetMap.size());
}

void MainWidgetDock::obsFrontendEventHandler(enum obs_frontend_event event, void *private_data)
{

	MainWidgetDock *trackerWidgetDock = (MainWidgetDock *)private_data;

	switch (event) {
	case OBS_FRONTEND_EVENT_FINISHED_LOADING:
		MainWidgetDock::loadSavedSettings(trackerWidgetDock);
		break;
	case OBS_FRONTEND_EVENT_THEME_CHANGED:
		MainWidgetDock::updateWidgetStyles(trackerWidgetDock);
		break;
		break;
	default:
		break;
	}
}

void MainWidgetDock::updateWidgetStyles(MainWidgetDock *trackerWidgetDock)
{
	int trackerWidgetCount = trackerWidgetDock->m_ui->trackerListLayout->count();
	for (int i = 0; i < trackerWidgetCount; i++) {
		FaceTracker *trackerWidget =
			static_cast<FaceTracker *>(trackerWidgetDock->m_ui->trackerListLayout->itemAt(i)->widget());
		if (trackerWidget) {
			trackerWidget->updateWidgetStyles();
		}
	}
}

void MainWidgetDock::registerAllHotkeys(obs_data_t *savedData)
{
	loadHotkey(
		m_addTrackerHotkeyId, ADDTRACKERHOTKEYNAME, obs_module_text("AddTrackerHotkeyDescription"),
		[this]() { m_ui->addTrackerButton->click(); }, "Add Timer Hotkey Pressed", savedData);
}

void MainWidgetDock::unregisterAllHotkeys()
{
	if (m_addTrackerHotkeyId)
		obs_hotkey_unregister(m_addTrackerHotkeyId);
}

void MainWidgetDock::addTracker(obs_data_t *savedData)
{
	FaceTracker *newFaceTracker = new FaceTracker(this, savedData, this);

	m_trackerWidgetMap.insert(newFaceTracker->getTrackerID(), newFaceTracker);
	connectTrackerSignalHandlers(newFaceTracker);

	trackerListLayout->addWidget(newFaceTracker);

	// UpdateTrackerListMoveButtonState();
}

void MainWidgetDock::loadSavedSettings(MainWidgetDock *dockWidget)
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
				dockWidget->addTracker(timerDataObj);
			}
		}

		// Add widget if none were loaded from save file
		// as we must have at least 1 timer
		if (dockWidget->getNumberOfTimers() == 0) {
			dockWidget->addTracker();
		}

		dockWidget->registerAllHotkeys(data);

		obs_data_release(data);
	}
}

// --------------------------------- Private Slots ----------------------------------

void MainWidgetDock::addTrackerButtonClicked()
{
	addTracker();
}

void MainWidgetDock::removeTrackerButtonClicked(QString id)
{
	FaceTracker *itemToBeRemoved = m_trackerWidgetMap.value(id, nullptr);

	if (itemToBeRemoved) {
		itemToBeRemoved->removeFromList();
		m_trackerWidgetMap.remove(id);
		obs_log(LOG_INFO, (QString("Tracker %1 deleted").arg(id)).toStdString().c_str());
	}

	// ToggleUIForMultipleTrackers();
	// UpdateTrackerListMoveButtonState();
}
