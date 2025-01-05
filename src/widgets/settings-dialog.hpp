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

#include "../widgets/pixmap-item.hpp"
#include "../main-widget-dock.hpp"
#include "../utils/blendshape-key.hpp"
#include "../utils/tracker-utils.hpp"
#include "../utils/tracker-data.hpp"
#include "../utils/pose-image-data.hpp"
#include "../utils/pose-image.hpp"
#include "../utils/pose.hpp"
#include "../utils/network-tracking.hpp"
#include "../ui/ui_FaceTrackerDialog.h"

class SettingsDialog : public QDialog {
	Q_OBJECT
public:
	explicit SettingsDialog(QWidget *parent = nullptr, QSharedPointer<TrackerData> tData = nullptr,
				MainWidgetDock *mWidget = nullptr);
	~SettingsDialog();
	void setFormDetails(QSharedPointer<TrackerData> settingsDialogData);
	void updateStyledUIComponents();

private:
	Ui::FaceTrackerDialog *ui;
	MainWidgetDock *mainWidget;
	QSharedPointer<TrackerData> trackerData;
	bool isError = false;
	QString formErrorStyling = "border: 1px solid rgb(192, 0, 0);";

	QStandardItemModel *poseListModel = nullptr;
	QList<QSharedPointer<Pose>> settingsPoseList;
	QMap<PoseImage, QLineEdit *> poseImageLineEdits;
	QGraphicsScene *avatarPreviewScene = nullptr;
	int previouslySelectedPoseIndex = -1;
	bool isMovingPoseListRows = false;

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

	void addImageToScene(PoseImageData *imageData, bool useImagePos = false, bool clearScene = false);
	void clearScene();
	void clearCurrentPoseConfig();
	void loadSelectedPoseConfig();
	int getSelectedRow();
	void resetPoseUITab();
	void centerSceneOnItems();

protected:
	void showEvent(QShowEvent *event) override;
	void closeEvent(QCloseEvent *) override;

signals:
	void settingsUpdated(QSharedPointer<TrackerData> newData);

private slots:
	void formChangeDetected();

	void applyButtonClicked();
	void cancelButtonClicked();
	void okButtonClicked();

	void syncPoseListToModel();
	void addPose();
	void deletePose();
	void handleMovePose(bool isDirectionUp = false);
	void handleImageUrlButtonClicked(PoseImage image);
	void handleClearImageUrl(PoseImage image);
	void handlePoseListClick(QModelIndex modelIndex);
	void onPoseRowsInserted(const QModelIndex &parent, int first, int last);
	void onPoseRowsRemoved(const QModelIndex &parent, int first, int last);
	void onPoseDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
			       const QList<int> &roles = QList<int>());
	void handleCenterViewButtonClick();
	void handleMoveImageUpClick();
	void handleMoveImageDownClick();
	void handleImageZoomClick(bool isZoomOut = false);
	void handleImageMove(qreal x, qreal y, qreal z, PoseImage pImageType);
};

#endif // SETTINGSDIALOG_H
