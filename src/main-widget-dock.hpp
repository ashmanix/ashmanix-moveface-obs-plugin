#ifndef MAINWIDGETDOCK_H
#define MAINWIDGETDOCK_H

#include <QDockWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QMetaType>
#include <QPushButton>

#include <obs.h>
#include <obs.hpp>
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs-websocket-api.h>

#include "plugin-support.h"
#include "widgets/obs-dock-wrapper.hpp"
#include "ui/ui_MainWidget.h"

#define CONFIG "config.json"

Q_DECLARE_OPAQUE_POINTER(obs_data_t *)

// Forward declerations
class FaceTracker;

class MainWidgetDock : public OBSDock {
	Q_OBJECT
public:
	explicit MainWidgetDock(QWidget *parent = nullptr);
	~MainWidgetDock();
	void ConfigureWebSocketConnection();

	// struct WebsocketCallbackData {
	// 	MainWidgetDock *instance;
	// 	WebsocketRequestType requestType;
	// 	const char *requestDataKey;
	// 	const char *requestTimerIdKey;
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

	static void OBSFrontendEventHandler(enum obs_frontend_event event, void *private_data);
	static void UpdateWidgetStyles(MainWidgetDock *mainWidgetDock);

	void AddTracker(obs_data_t *savedData = nullptr);

signals:

private slots:
	void AddTrackerButtonClicked();
	void RemoveTrackerButtonClicked(QString id);
};

#endif // MAINWIDGETDOCK_H
