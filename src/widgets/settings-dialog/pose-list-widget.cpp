#include "pose-list-widget.h"
#include <obs-module.h>
#include "../../utils/utils.h"

PoseListWidget::PoseListWidget(QWidget *parent, QSharedPointer<TrackerData> tData)
	: QWidget(parent),
	  ui(new Ui::PoseListView)
{
	ui->setupUi(this);
	setupListUI();
	if (tData)
		setupListData(tData);

	connectUISignalHandlers();
}

PoseListWidget::~PoseListWidget() {}

void PoseListWidget::clearList()
{
	if (poseListModel)
		poseListModel->clear();
}

int PoseListWidget::getSelectedRow()
{
	QModelIndex modelIndex = ui->poseListView->currentIndex();
	return modelIndex.row();
}

void PoseListWidget::setupListData(QSharedPointer<TrackerData> settingsDialogData)
{
	if (poseListModel)
		poseListModel->clear();
	// loop through data and add ID's to list
	
}

//  ------------------------------------------------- Private --------------------------------------------------

void PoseListWidget::connectUISignalHandlers()
{

	QObject::connect(ui->addPoseToolButton, &QToolButton::clicked, this, &PoseListWidget::addPose);
	QObject::connect(ui->deletePoseToolButton, &QToolButton::clicked, this, &PoseListWidget::deletePose);
	QObject::connect(ui->movePoseUpToolButton, &QToolButton::clicked, this, [this]() { handleMovePose(true); });
	QObject::connect(ui->movePoseDownToolButton, &QToolButton::clicked, this, [this]() { handleMovePose(); });

	QObject::connect(ui->poseListView, &QListView::clicked, this, &PoseListWidget::handlePoseListClick);

	QObject::connect(poseListModel, &QAbstractItemModel::rowsInserted, this, &PoseListWidget::onPoseRowsInserted);
	QObject::connect(poseListModel, &QAbstractItemModel::rowsRemoved, this, &PoseListWidget::onPoseRowsRemoved);
	QObject::connect(poseListModel, &QAbstractItemModel::dataChanged, this, &PoseListWidget::onPoseDataChanged);
}

void PoseListWidget::setupListUI(QSharedPointer<TrackerData> PoseListWidgetData)
{

	ui->addPoseToolButton->setToolTip(obs_module_text("DialogAddPoseToolTip"));
	ui->deletePoseToolButton->setToolTip(obs_module_text("DialogDeletePoseToolTip"));
	ui->movePoseUpToolButton->setToolTip(obs_module_text("DialogMovePoseUpToolTip"));
	ui->movePoseDownToolButton->setToolTip(obs_module_text("DialogMovePoseDownToolTip"));

	ui->poseListLabel->setText(obs_module_text("DialogPostListLabel"));

	poseListModel = new QStandardItemModel(this);
	ui->poseListView->setModel(poseListModel);
	ui->poseListView->setDragEnabled(false);
	ui->poseListView->setAcceptDrops(false);
	ui->poseListView->setDragDropOverwriteMode(false);

	updateStyledUIComponents();
}

void PoseListWidget::updateStyledUIComponents()
{
	QString baseUrl = obs_frontend_is_theme_dark() ? getDataFolderPath() + "/icons/dark/"
						       : getDataFolderPath() + "/icons/light/";

	QString plusIconPath = QDir::fromNativeSeparators(baseUrl + "plus.svg");
	if (QFileInfo::exists(plusIconPath)) {
		QIcon plusIcon(plusIconPath);

		ui->addPoseToolButton->setIcon(plusIcon);
	}

	QString minusIconPath = QDir::fromNativeSeparators(baseUrl + "minus.svg");
	if (QFileInfo::exists(minusIconPath)) {
		QIcon minusIcon(minusIconPath);
	}

	QString trashIconPath = QDir::fromNativeSeparators(baseUrl + "trash.svg");
	if (QFileInfo::exists(trashIconPath)) {
		QIcon trashIcon(trashIconPath);

		ui->deletePoseToolButton->setIcon(trashIcon);
	}

	QString upIconPath = QDir::fromNativeSeparators(baseUrl + "up.svg");
	if (QFileInfo::exists(upIconPath)) {
		QIcon upIcon(upIconPath);
		ui->movePoseUpToolButton->setIcon(upIcon);
	}

	QString downIconPath = QDir::fromNativeSeparators(baseUrl + "down.svg");
	if (QFileInfo::exists(downIconPath)) {
		QIcon downIcon(downIconPath);
		ui->movePoseDownToolButton->setIcon(downIcon);
	}

	QString centerIconPath = QDir::fromNativeSeparators(baseUrl + "center.svg");
	if (QFileInfo::exists(centerIconPath)) {
		QIcon centerIcon(centerIconPath);
	}

	ui->poseListView->setStyleSheet("QListView::item {"
					"   padding-top: 5px;"
					"   padding-bottom: 5px;"
					"}");
}

//  ---------------------------------------------- Private Slots -----------------------------------------------

void PoseListWidget::addPose()
{
	if (!poseListModel)
		return obs_log(LOG_ERROR, "No list model found!");

	int count = poseListModel->rowCount();
	QStandardItem *poseItem = new QStandardItem(QString("New Pose %1").arg(count));
	poseListModel->appendRow(poseItem);

	// SIGNAL that pose was added
}

void PoseListWidget::deletePose()
{
	QModelIndex modelIndex = ui->poseListView->currentIndex();
	int rowIndex = modelIndex.row();

	obs_log(LOG_INFO, "Row To Delete: %d", rowIndex);

	if (rowIndex != -1) {
		poseListModel->takeRow(rowIndex);
	}

	// SIGNAL that a pose was deleted
}

void PoseListWidget::handleMovePose(bool isDirectionUp)
{
	if (!poseListModel)
		return;

	int selectedRow = getSelectedRow();
	if (selectedRow == -1)
		return;

	int targetRow = isDirectionUp ? selectedRow - 1 : selectedRow + 1;

	if (targetRow < 0 || targetRow >= poseListModel->rowCount())
		return;

	QStandardItem *currentItem = poseListModel->item(selectedRow, 0);
	QStandardItem *targetItem = poseListModel->item(targetRow, 0);

	if (!currentItem || !targetItem)
		return;

	QString tempText = currentItem->text();
	currentItem->setText(targetItem->text());
	targetItem->setText(tempText);

	ui->poseListView->setCurrentIndex(poseListModel->index(targetRow, 0));

	// SIGNAL that pose was moved
}

void PoseListWidget::handlePoseListClick(QModelIndex modelIndex)
{
	obs_log(LOG_INFO, "Row: %d", modelIndex.row());

	// SIGNAL that a pose was selected
	emit rowSelected(modelIndex.row());
}

void PoseListWidget::onPoseRowsInserted(const QModelIndex &parent, int first, int last)
{
	Q_UNUSED(parent);

	if (isMovingPoseListRows)
		return;

	//SIGNAL that pose row was inserted
	for (int row = first; row <= last; ++row) {
		// E.g. read the poseId from the model
		QModelIndex idx = poseListModel->index(row, 0);
		QString poseId = idx.data(Qt::DisplayRole).toString();

		emit rowInserted(row, poseId);
	}
}

void PoseListWidget::onPoseRowsRemoved(const QModelIndex &parent, int first, int last)
{
	Q_UNUSED(parent);

	if (isMovingPoseListRows)
		return;

	// SIGNAL that pose was removed
	for (int row = last; row >= first; --row) {
		emit rowRemoved(row);
	}
}

void PoseListWidget::onPoseDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
				       const QList<int> &roles)
{
	Q_UNUSED(&roles);
	for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
		// Read changed data from the model and update the Pose
		QModelIndex idx = poseListModel->index(row, 0);
		QString newPoseId = idx.data(Qt::DisplayRole).toString();
		// SIGNAL new pose ID and row
		emit rowDataChanged(row, newPoseId);
	}
}
