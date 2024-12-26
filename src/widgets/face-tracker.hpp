#ifndef FACETRACKER_H
#define FACETRACKER_H

#include <QDockWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QMetaType>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <obs.h>
#include <obs.hpp>
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs-websocket-api.h>

#include "plugin-support.h"
#include "../ui/ui_FaceTracker.h"
#include "../utils/tracker-utils.hpp"
#include "../utils/network-connection.hpp"
#include "../main-widget-dock.hpp"

// Forward declarations
class SettingsDialog;
class MainWidgetDock;

class FaceTracker : public QWidget {
	Q_OBJECT
public:
	explicit FaceTracker(QWidget *parent = nullptr, obs_data_t *savedData = nullptr,
			     MainWidgetDock *mDockWidget = nullptr);
	~FaceTracker();
	QString GetTrackerID();
	void SetTrackerID(const QString &newId = QString());
	TrackerDataStruct *GetTrackerData();

	void SaveTrackerWidgetDataToOBSSaveData(obs_data_t *dataObject);
	void LoadTrackerWidgetDataFromOBSSaveData(obs_data_t *dataObject);

private:
	Ui::FaceTracker *ui = nullptr;
	SettingsDialog *settingsDialogUi = nullptr;

	QString title;
	TrackerDataStruct trackerData;
	MainWidgetDock *mainDockWidget = nullptr;

	NetworkConnection *networkConnection = nullptr;
	QTimer *connectionTimerTimeout = nullptr;

	QJsonObject initiateTrackingObject = {
		{"messageType", "iOSTrackingDataRequest"},
		{"time", 1},
		{"sentBy", "vTuberApp"},
	};

	void SetupWidgetUI();
	void ConnectUISignalHandlers();
	void SetTrackerData();
	void SetConnectionLabel(bool isConnected);
	void UpdateTrackerDataFromDialog(TrackerDataStruct *newData);
	void InitiateNetworkConnection(bool shouldSendRequest = false);

signals:
	void RequestDelete(QString id);

private slots:
	void SettingsActionSelected();
	void DeleteActionSelected();
	void HandleTrackingData(QString data);
};

#endif // FACETRACKER_H
