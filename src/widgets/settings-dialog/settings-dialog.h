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
#include <QFileDialog>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QList>

#include <string>

#include "../../classes/poses/movable-pixmap-item.h"
#include "../../classes/blendshapes/blendshape-key.h"
#include "../../utils/utils.h"
#include "../../classes/poses/pose-image-data.h"
#include "../../classes/poses/pose-image.h"
#include "../../classes/poses/pose.h"
#include "../comms/network-tracking.h"
#include "../main-widget-dock.h"
#include "pose-list-widget.h"
#include "image-files-widget.h"
#include "face-settings-widget.h"
#include "blendshape-rules-widget.h"
#include "pose-display-widget.h"
#include "../../classes/tracking/tracker-data.h"
#include "../comms/tracker-worker.h"
#include "../../ui/settings-dialog/ui_FaceTrackerDialog.h"

class SettingsDialog : public QDialog {
	Q_OBJECT
public:
	explicit SettingsDialog(QWidget *parent = nullptr, QSharedPointer<TrackerData> tData = nullptr,
				MainWidgetDock *mWidget = nullptr,
				QSharedPointer<TrackerWorker> trackerWorker = nullptr);
	~SettingsDialog() override;
	void setFormDetails(QSharedPointer<TrackerData> settingsDialogData);
	void updateStyledUIComponents();
	void updateTrackerWorker(QSharedPointer<TrackerWorker> trackerWorker);

private:
	Ui::FaceTrackerDialog *m_ui;
	MainWidgetDock *m_mainWidget;
	PoseListWidget *m_poseListWidget;
	ImageFilesWidget *m_imageFilesWidget = new ImageFilesWidget(this);
	FaceSettingsWidget *m_faceSettingsWidget = new FaceSettingsWidget(this);
	BlendshapeRulesWidget *m_blendshapeRulesWidget = new BlendshapeRulesWidget(this);
	PoseDisplayWidget *m_poseDisplayWidget = new PoseDisplayWidget(this);

	QList<SettingsWidgetInterface *> m_trackingWidgetList = {m_imageFilesWidget, m_faceSettingsWidget,
								 m_blendshapeRulesWidget, m_poseDisplayWidget};

	QSharedPointer<TrackerData> m_trackerData;
	QSharedPointer<TrackerWorker> m_trackerWorker;
	bool m_isError = false;
	QString m_formErrorStyling = "border: 1px solid rgb(192, 0, 0);";

	QList<QSharedPointer<Pose>> m_settingsPoseList;
	int m_previouslySelectedPoseIndex = -1;
	bool m_isMovingPoseListRows = false;

	void connectUISignalHandlers();
	void connectObsSignalHandlers();
	void setTitle();
	void setupDialogUI(QSharedPointer<TrackerData> settingsDialogData);
	void applyFormChanges();
	Result validateTrackerID();
	Result validateDestIPAddress();

	void getOBSSourceList();
	static bool getImageSources(void *list_property, obs_source_t *source);
	static void obsSourceCreated(void *param, calldata_t *calldata);
	static void obsSourceDeleted(void *param, calldata_t *calldata);
	static void obsSourceRenamed(void *param, calldata_t *calldata);
	static int checkIfImageSourceType(obs_source_t *source);

	void clearCurrentPoseConfig();
	void loadSelectedPoseConfig();
	int getSelectedRow();
	void resetPoseUITab();

protected:
	void showEvent(QShowEvent *event) override;
	void closeEvent(QCloseEvent *) override;

signals:
	void settingsUpdated(QSharedPointer<TrackerData> newData);
	void trackingUpdated(VTubeStudioData data);

private slots:
	void handleRaisedWindow();
	void formChangeDetected();

	void applyButtonClicked();
	void cancelButtonClicked();
	void okButtonClicked();

	void syncPoseListToModel();

	void onPoseSelected(int rowIndex);
	void onPoseRowsInserted(QMap<int, QString> rowMap);
	void onPoseRowsRemoved(QList<int> rowsList);
	void onPoseDataChanged(QMap<int, QString> rowMap);
	void onPoseRowMoved(int sourceRow, int targetRow);

	void handleTrackingDataUpdate(VTubeStudioData data);
	void handleTrackingConnectionToggle(bool isConnected);
};

#endif // SETTINGSDIALOG_H
