#ifndef POSELISTWIDGET_H
#define POSELISTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QList>

#include <obs-frontend-api.h>

#include "plugin-support.h"
#include "../../classes/poses/pose-image.h"
#include "../../classes/tracking/tracker-data.h"
#include "../../ui/settings-dialog/ui_PoseListView.h"


class PoseListWidget : public QWidget {
	Q_OBJECT
public:
	explicit PoseListWidget(QWidget *parent = nullptr, QSharedPointer<TrackerData> tData = nullptr);
	~PoseListWidget();

	void clearList();
	int getSelectedRow();
	void addRow(QString rowId);
	void clearSelection();

private:
	Ui::PoseListView *ui;
	QStandardItemModel *poseListModel = nullptr;
	bool isMovingPoseListRows = false;

	void connectUISignalHandlers();
	void setupListUI();
	void updateStyledUIComponents();

signals:
	void rowsSelected(int row);
	void rowsInserted(QMap<int, QString> rowMap);
	void rowsRemoved(QList<int> row);
	void rowMoved(int sourceRow, int targetRow);
	void rowsDataChanged(QMap<int, QString> rowMap);

private slots:
	void addPose();
	void deletePose();
	void handleMovePose(bool isDirectionUp = false);
	void handlePoseListClick(QModelIndex modelIndex);
	void onRowsInserted(const QModelIndex &parent, int first, int last);
	void onRowsRemoved(const QModelIndex &parent, int first, int last);
	void onRowsDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
			       const QList<int> &roles = QList<int>());
};

#endif // POSELISTWIDGET_H
