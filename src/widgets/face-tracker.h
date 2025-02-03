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
#include <QFile>
#include <QTemporaryFile>

#include <obs.h>
#include <obs.hpp>
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs-websocket-api.h>
#include <graphics/graphics.h>

#include "plugin-support.h"
#include "../utils/utils.h"
#include "../classes/tracking/vtube-studio-data.h"
#include "../classes/tracking/tracker-data.h"
#include "./comms/network-tracking.h"
#include "../classes/poses/movable-pixmap-item.h"
#include "./comms/tracker-worker.h"
#include "../obs/my-gs-texture-wrapper.h"
#include "../ui/ui_FaceTracker.h"

// Forward declarations
class SettingsDialog;
class MainWidgetDock;
class Pose;

class FaceTracker : public QWidget {
	Q_OBJECT
public:
	explicit FaceTracker(QWidget *parent = nullptr, obs_data_t *savedData = nullptr,
			     MainWidgetDock *mDockWidget = nullptr);
	~FaceTracker() override;
	QString getTrackerID();
	void setTrackerID(const QString &newId = QString());
	TrackerData getTrackerData();

	void saveTrackerWidgetDataToOBSSaveData(obs_data_t *dataObject);
	void loadTrackerWidgetDataFromOBSSaveData(obs_data_t *dataObject);
	void updateWidgetStyles();

private:
	Ui::FaceTracker *m_ui = nullptr;
	QSharedPointer<SettingsDialog> m_settingsDialogUi = nullptr;

	QString m_title;
	bool m_isConnected;
	QSharedPointer<TrackerData> m_trackerData = nullptr;
	MainWidgetDock *m_mainDockWidget = nullptr;

	NetworkTracking *m_networkTracking = nullptr;
	QSharedPointer<TrackerWorker> m_trackerWorker;

	void setupWidgetUI();
	void connectUISignalHandlers();
	void setTrackerData();
	void loadPoseData();
	void setConnected(bool isConnectedInput);
	void updateTrackerDataFromDialog(QSharedPointer<TrackerData> newData);
	void initiateTracking();
	void enableTimer();
	void disableTimer();

signals:
	void requestDelete(QString id);

private slots:
	void settingsActionSelected();
	void deleteActionSelected();
	void handleDisplayNewImage(MyGSTextureWrapper *imageTexture, int width, int height);
	void toggleEnabled(int checkState);
	void toggleConnectionError(bool isError);
};

#endif // FACETRACKER_H
