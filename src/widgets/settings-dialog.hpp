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
	void UpdateStyledUIComponents();

private:
	Ui::FaceTrackerDialog *ui;
	MainWidgetDock *mainWidget;
	TrackerDataStruct *trackerData;
	bool isError = false;
	QString formErrorStyling = "border: 1px solid rgb(192, 0, 0);";

	QStandardItemModel *poseListModel = nullptr;
	QList<Pose> settingsPoseList;
	QGraphicsScene *avatarPreviewScene = nullptr;

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

	void AddImageToQGraphics(QString fileUrl, bool clearScene = false);
	void ClearCurrentPoseConfig();
	void LoadSelectedPoseConfig();
	int GetSelectedRow();

protected:
	void showEvent(QShowEvent *event) override;

signals:
	void SettingsUpdated(TrackerDataStruct *newData);

private slots:
	void FormChangeDetected();

	void ApplyButtonClicked();
	void CancelButtonClicked();
	void OkButtonClicked();

	void SyncPoseListToModel();
	void AddPose();
	void DeletePose();
	void HandleImageUrlButtonClicked(PoseImage image);
	void HandleClearImageUrl(PoseImage image);
	void HandlePoseListClick(QModelIndex modelIndex);
	void OnPoseRowsInserted(const QModelIndex &parent, int first, int last);
	void OnPoseRowsRemoved(const QModelIndex &parent, int first, int last);
	void OnPoseDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
				const QList<int> &roles = QList<int>());
};

#endif // SETTINGSDIALOG_H
