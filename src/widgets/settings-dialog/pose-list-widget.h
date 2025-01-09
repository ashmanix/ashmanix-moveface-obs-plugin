#ifndef POSELISTWIDGET_H
#define POSELISTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QStandardItemModel>
#include <QFileDialog>

#include <obs-frontend-api.h>

#include "plugin-support.h"
#include "../../classes/poses/pose-image.h"
#include "../../classes/tracking/tracker-data.h"
#include "../../ui/settings-dialog/ui_PoseListView.h"

// Forward declerations

class PoseListWidget : public QWidget {
	Q_OBJECT
public:
	explicit PoseListWidget(QWidget *parent = nullptr, QSharedPointer<TrackerData> tData = nullptr);
	~PoseListWidget();

	void clearList();
	int getSelectedRow();
	void setupListData(QSharedPointer<TrackerData> settingsDialogData);

private:
	Ui::PoseListView *ui;
	QStandardItemModel *poseListModel = nullptr;
	bool isMovingPoseListRows = false;

	void connectUISignalHandlers();
	void setupListUI(QSharedPointer<TrackerData> settingsDialogData = nullptr);
	void updateStyledUIComponents();

signals:
	void rowSelected(int row);
	void rowInserted(int row, QString id);
	void rowRemoved(int row);
	void rowDataChanged(int row, QString newId);

private slots:
	void syncPoseListToModel();
	void addPose();
	void deletePose();
	void handleMovePose(bool isDirectionUp = false);
	void handlePoseListClick(QModelIndex modelIndex);
	void onPoseRowsInserted(const QModelIndex &parent, int first, int last);
	void onPoseRowsRemoved(const QModelIndex &parent, int first, int last);
	void onPoseDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
			       const QList<int> &roles = QList<int>());
};

#endif // POSELISTWIDGET_H
