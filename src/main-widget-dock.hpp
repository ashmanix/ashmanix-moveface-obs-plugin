#ifndef MAINWIDGETDOCK_H
#define MAINWIDGETDOCK_H

#include <QDockWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QMetaType>
#include <QPushButton>

#include <util/platform.h>
#include <obs.h>
#include <obs.hpp>
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs-websocket-api.h>

#include "plugin-support.h"
#include "./utils/obs-utils.hpp"
#include "./utils/tracker-utils.hpp"
#include "widgets/obs-dock-wrapper.hpp"
#include "ui/ui_MainWidget.h"


// Forward declerations
class FaceTracker;

class MainWidgetDock : public OBSDock {
	Q_OBJECT
public:
	explicit MainWidgetDock(QWidget *parent = nullptr);
	~MainWidgetDock();
	void ConfigureWebSocketConnection();

	Result ValidateNewTrackerID(QString id);
	Result UpdateTrackerList(QString oldId, QString newId);

	// struct WebsocketCallbackData {
	// 	MainWidgetDock *instance;
	// 	WebsocketRequestType requestType;
	// 	const char *requestDataKey;
	// 	const char *requestTrackerIdKey;
	// };

private:
	QMap<QString, FaceTracker *> trackerWidgetMap;
	QVBoxLayout *trackerListLayout;
	int addTrackerHotkeyId = -1;

	static inline const char *addTrackerHotkeyName = "Ashmanix_MoveFace_Add_Tracker";

	static inline const char *VENDORNAME = "ashmanix-moveface-plugin";
	static inline const char *TRACKERIDKEY = "tracker_id";

	obs_websocket_vendor vendor = nullptr;
	Ui::MainWidget *ui;

	void SetupCountdownWidgetUI();
	void ConnectUISignalHandlers();
	void ConnectTrackerSignalHandlers(FaceTracker *faceTracker);
	void SaveSettings();
	int GetNumberOfTimers();
	void RegisterAllHotkeys(obs_data_t *savedData);
	void UnregisterAllHotkeys();
	void AddTracker(obs_data_t *savedData = nullptr);

	static void OBSFrontendEventHandler(enum obs_frontend_event event, void *private_data);
	static void UpdateWidgetStyles(MainWidgetDock *mainWidgetDock);
	static void LoadSavedSettings(MainWidgetDock *trackerWidgetDock);

signals:

private slots:
	void AddTrackerButtonClicked();
	void RemoveTrackerButtonClicked(QString id);
};

#endif // MAINWIDGETDOCK_H
