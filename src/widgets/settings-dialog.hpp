#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include <QDialog>

#include "../main-widget-dock.hpp"
#include "../utils/tracker-utils.hpp"
#include "../ui/ui_FaceTrackerDialog.h"

class SettingsDialog : public QDialog {
	Q_OBJECT
public:
	explicit SettingsDialog(QWidget *parent = nullptr, TrackerDataStruct *tData = nullptr,
				MainWidgetDock *mWidget = nullptr);
	~SettingsDialog();

private:
	Ui::FaceTrackerDialog *ui;
	MainWidgetDock *mainWidget;
	TrackerDataStruct *trackerData;

	void SetTitle();

signals:

private slots:
};

#endif // SETTINGSDIALOG_H
