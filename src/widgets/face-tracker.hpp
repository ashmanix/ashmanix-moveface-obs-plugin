#ifndef FACETRACKER_H
#define FACETRACKER_H

#include <QDockWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QMetaType>
#include <QString>

#include <obs.h>
#include <obs.hpp>
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs-websocket-api.h>

#include "plugin-support.h"
#include "../ui/ui_FaceTracker.h"
#include "../utils/tracker-utils.hpp"
#include "../main-widget-dock.hpp"

// Forward declarations
class SettingsDialog;
class MainWidgetDock;

class FaceTracker : public QWidget {
	Q_OBJECT
public:
	explicit FaceTracker(QWidget *parent = nullptr, obs_data_t *savedData = nullptr,
			     MainWidgetDock *mDockWidget = nullptr);
	;
	~FaceTracker();
	QString GetTrackerID();

private:
	Ui::FaceTracker *ui;
    SettingsDialog *settingsDialogUi = nullptr;

	QString title;
	TrackerDataStruct trackerData;
	MainWidgetDock *mainDockWidget;

	void SetupWidgetUI();
	void ConnectUISignalHandlers();
	void SetTrackerData();
	void LoadTimerWidgetDataFromOBSSaveData(obs_data_t *dataObject);

signals:
	void RequestDelete(QString id);

private slots:
	void SettingsButtonClicked();
	void DeleteButtonClicked();
};

#endif // FACETRACKER_H
