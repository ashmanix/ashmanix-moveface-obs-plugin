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
#include <QTimer>

#include <obs.h>
#include <obs.hpp>
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs-websocket-api.h>

#include "plugin-support.h"
#include "../ui/ui_FaceTracker.h"
#include "../utils/tracker-utils.hpp"
#include "../utils/network-tracking.hpp"
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
	bool isConnected;
	TrackerDataStruct trackerData;
	MainWidgetDock *mainDockWidget = nullptr;

	NetworkTracking *networkTracking = nullptr;
	const int MAXERRORCOUNT = 3;
	int networkErrorCount = 0;
	QTimer *networkTrackingDataRequestTimer = nullptr;
	QTimer *connectionTimer = nullptr;
	const int trackingDataPeriodInSecs = 5;

	QJsonObject initiateTrackingObject = {
		{"messageType", "iOSTrackingDataRequest"},
		{"time", trackingDataPeriodInSecs},
		{"sentBy", "vTuberApp"},
	};

	void SetupWidgetUI();
	void ConnectUISignalHandlers();
	void SetTrackerData();
	void SetConnected(bool isConnectedInput);
	void UpdateTrackerDataFromDialog(TrackerDataStruct *newData);
	void InitiateNetworkTracking(bool shouldSendRequest = false);
	void ResetConnectionTimer();
	void EnableTimer();
	void DisableTimer();

signals:
	void RequestDelete(QString id);

private slots:
	void SettingsActionSelected();
	void DeleteActionSelected();
	void HandleTrackingData(QString data);
	void RequestTrackingData();
	void ToggleEnabled(int checkState);
};

#endif // FACETRACKER_H
