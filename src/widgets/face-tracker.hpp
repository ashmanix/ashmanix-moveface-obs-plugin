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
#include "../utils/tracker-data.hpp"
#include "../utils/vtube-studio-data.hpp"
#include "../utils/network-tracking.hpp"
#include "pixmap-item.hpp"
#include "../main-widget-dock.hpp"

// Forward declarations
class SettingsDialog;
class MainWidgetDock;
class Pose;

class FaceTracker : public QWidget {
	Q_OBJECT
public:
	explicit FaceTracker(QWidget *parent = nullptr, obs_data_t *savedData = nullptr,
			     MainWidgetDock *mDockWidget = nullptr);
	~FaceTracker();
	QString getTrackerID();
	void setTrackerID(const QString &newId = QString());
	TrackerData getTrackerData();

	void saveTrackerWidgetDataToOBSSaveData(obs_data_t *dataObject);
	void loadTrackerWidgetDataFromOBSSaveData(obs_data_t *dataObject);
	void updateWidgetStyles();

private:
	Ui::FaceTracker *ui = nullptr;
	SettingsDialog *settingsDialogUi = nullptr;

	QString title;
	bool isConnected;
	QSharedPointer<TrackerData> trackerData;
	MainWidgetDock *mainDockWidget = nullptr;

	NetworkTracking *networkTracking = nullptr;

	void setupWidgetUI();
	void connectUISignalHandlers();
	void setTrackerData();
	void loadPoseData();
	void setConnected(bool isConnectedInput);
	void updateTrackerDataFromDialog(QSharedPointer<TrackerData> newData);
	void initiateNetworkTracking();
	void enableTimer();
	void disableTimer();

signals:
	void requestDelete(QString id);

private slots:
	void settingsActionSelected();
	void deleteActionSelected();
	void handleTrackingData(VTubeStudioData data);
	void toggleEnabled(int checkState);
	void toggleConnectionError(bool isError);
};

#endif // FACETRACKER_H
