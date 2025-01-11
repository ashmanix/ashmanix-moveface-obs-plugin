#include "pose-list-widget.h"
#include <obs-module.h>
#include "../../utils/utils.h"

PoseListWidget::PoseListWidget(QWidget *parent, QSharedPointer<TrackerData> tData)
	: QWidget(parent),
	  ui(new Ui::PoseListView)
{
	UNUSED_PARAMETER(tData);
	ui->setupUi(this);
	setupListUI();

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

void PoseListWidget::addRow(QString rowId)
{
	QStandardItem *item = new QStandardItem(rowId);
	poseListModel->appendRow(item);
}

void PoseListWidget::clearSelection()
{
	ui->poseListView->clearSelection();
}

//  ------------------------------------------------- Private --------------------------------------------------

void PoseListWidget::connectUISignalHandlers()
{

	QObject::connect(ui->addPoseToolButton, &QToolButton::clicked, this, &PoseListWidget::addPose);
	QObject::connect(ui->deletePoseToolButton, &QToolButton::clicked, this, &PoseListWidget::deletePose);
	QObject::connect(ui->movePoseUpToolButton, &QToolButton::clicked, this, [this]() { handleMovePose(true); });
	QObject::connect(ui->movePoseDownToolButton, &QToolButton::clicked, this, [this]() { handleMovePose(); });

	QObject::connect(ui->poseListView, &QListView::clicked, this, &PoseListWidget::handlePoseListClick);

	QObject::connect(poseListModel, &QAbstractItemModel::rowsInserted, this, &PoseListWidget::onRowsInserted);
	QObject::connect(poseListModel, &QAbstractItemModel::rowsRemoved, this, &PoseListWidget::onRowsRemoved);
	QObject::connect(poseListModel, &QAbstractItemModel::dataChanged, this, &PoseListWidget::onRowsDataChanged);
}

void PoseListWidget::setupListUI()
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
}

void PoseListWidget::deletePose()
{
	QModelIndex modelIndex = ui->poseListView->currentIndex();
	int rowIndex = modelIndex.row();

	obs_log(LOG_INFO, "Row To Delete: %d", rowIndex);

	if (rowIndex != -1) {
		poseListModel->takeRow(rowIndex);
	}
}

void PoseListWidget::handleMovePose(bool isDirectionUp)
{
	if (!poseListModel)
		return;

	isMovingPoseListRows = true;

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
	emit rowMoved(selectedRow, targetRow);

	isMovingPoseListRows = false;
}

void PoseListWidget::handlePoseListClick(QModelIndex modelIndex)
{
	emit rowsSelected(modelIndex.row());
}

void PoseListWidget::onRowsInserted(const QModelIndex &parent, int first, int last)
{
	Q_UNUSED(parent);

	if (isMovingPoseListRows)
		return;

	QMap<int, QString> rowDataMap = {};

	for (int row = first; row <= last; ++row) {
		// E.g. read the poseId from the model
		QModelIndex idx = poseListModel->index(row, 0);
		QString poseId = idx.data(Qt::DisplayRole).toString();
		rowDataMap.insert(row, poseId);
	}

	if (rowDataMap.size() > 0) {
		emit rowsInserted(rowDataMap);
	}
}

void PoseListWidget::onRowsRemoved(const QModelIndex &parent, int first, int last)
{
	Q_UNUSED(parent);

	if (isMovingPoseListRows)
		return;

	QList<int> rowList = {};

	for (int row = last; row >= first; --row) {
		rowList.append(row);
	}

	if (rowList.length() > 0) {
		emit rowsRemoved(rowList);
	}
}

void PoseListWidget::onRowsDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
				       const QList<int> &roles)
{
	Q_UNUSED(&roles);

	if (isMovingPoseListRows)
		return;

	QMap<int, QString> rowDataMap = {};

	for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
		// Read changed data from the model and update the Pose
		QModelIndex idx = poseListModel->index(row, 0);
		QString newPoseId = idx.data(Qt::DisplayRole).toString();
		rowDataMap.insert(row, newPoseId);
	}

	if (rowDataMap.size() > 0) {
		emit rowsDataChanged(rowDataMap);
	}
}
