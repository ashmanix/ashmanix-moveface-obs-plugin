#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QValidator>
#include <QMenu>
#include <QVector>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>

#include "../main-widget-dock.hpp"
#include "../utils/tracker-utils.hpp"
#include "../utils/network-tracking.hpp"
#include "../ui/ui_FaceTrackerDialog.h"

class SettingsDialog : public QDialog {
	Q_OBJECT
public:
	explicit SettingsDialog(QWidget *parent = nullptr, TrackerDataStruct *tData = nullptr,
				MainWidgetDock *mWidget = nullptr);
	~SettingsDialog();
	void SetFormDetails(TrackerDataStruct *settingsDialogData);

private:
	Ui::FaceTrackerDialog *ui;
	MainWidgetDock *mainWidget;
	TrackerDataStruct *trackerData;
	bool isError = false;
	QString formErrorStyling = "border: 1px solid rgb(192, 0, 0);";

	void ConnectUISignalHandlers();
	void ConnectObsSignalHandlers();
	void SetTitle();
	void SetupDialogUI(TrackerDataStruct *settingsDialogData);
	void ApplyFormChanges();
	Result ValidateTrackerID();
	Result ValidateDestIPAddress();

	void GetOBSSourceList();
	static bool GetImageSources(void *list_property, obs_source_t *source);
	static void OBSSourceCreated(void *param, calldata_t *calldata);
	static void OBSSourceDeleted(void *param, calldata_t *calldata);
	static void OBSSourceRenamed(void *param, calldata_t *calldata);
	static int CheckIfImageSourceType(obs_source_t *source);

	void AddImageToQGraphics();

protected:
	void showEvent(QShowEvent *event) override;

signals:
	void SettingsUpdated(TrackerDataStruct *newData);

private slots:
	void FormChangeDetected();

	void ApplyButtonClicked();
	void CancelButtonClicked();
	void OkButtonClicked();
	void AddNewPose();
};

#endif // SETTINGSDIALOG_H
