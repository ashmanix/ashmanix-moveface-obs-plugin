#include "pose-list-widget.h"
#include <obs-module.h>
#include "../../utils/utils.h"

PoseListWidget::PoseListWidget(QWidget *parent, QSharedPointer<TrackerData> tData)
	: QWidget(parent),
	  m_ui(new Ui::PoseListView)
{
	UNUSED_PARAMETER(tData);
	m_ui->setupUi(this);
	setupListUI();

	connectUISignalHandlers();
}

PoseListWidget::~PoseListWidget() {}

void PoseListWidget::clearList()
{
	if (m_poseListModel)
		m_poseListModel->clear();
}

int PoseListWidget::getSelectedRow()
{
	QModelIndex modelIndex = m_ui->poseListView->currentIndex();
	return modelIndex.row();
}

void PoseListWidget::addRow(QString rowId)
{
	QStandardItem *item = new QStandardItem(rowId);
	m_poseListModel->appendRow(item);
}

void PoseListWidget::clearSelection()
{
	m_ui->poseListView->clearSelection();
}

//  ------------------------------------------------- Private --------------------------------------------------

void PoseListWidget::connectUISignalHandlers()
{

	QObject::connect(m_ui->addPoseToolButton, &QToolButton::clicked, this, &PoseListWidget::addPose);
	QObject::connect(m_ui->deletePoseToolButton, &QToolButton::clicked, this, &PoseListWidget::deletePose);
	QObject::connect(m_ui->movePoseUpToolButton, &QToolButton::clicked, this, [this]() { handleMovePose(true); });
	QObject::connect(m_ui->movePoseDownToolButton, &QToolButton::clicked, this, [this]() { handleMovePose(); });

	QObject::connect(m_ui->poseListView, &QListView::clicked, this, &PoseListWidget::handlePoseListClick);

	QObject::connect(m_poseListModel, &QAbstractItemModel::rowsInserted, this, &PoseListWidget::onRowsInserted);
	QObject::connect(m_poseListModel, &QAbstractItemModel::rowsRemoved, this, &PoseListWidget::onRowsRemoved);
	QObject::connect(m_poseListModel, &QAbstractItemModel::dataChanged, this, &PoseListWidget::onRowsDataChanged);
}

void PoseListWidget::setupListUI()
{

	m_ui->addPoseToolButton->setToolTip(obs_module_text("DialogAddPoseToolTip"));
	m_ui->deletePoseToolButton->setToolTip(obs_module_text("DialogDeletePoseToolTip"));
	m_ui->movePoseUpToolButton->setToolTip(obs_module_text("DialogMovePoseUpToolTip"));
	m_ui->movePoseDownToolButton->setToolTip(obs_module_text("DialogMovePoseDownToolTip"));

	m_ui->poseListLabel->setText(obs_module_text("DialogPoseListLabel"));
	m_ui->poseListLabel->setToolTip(obs_module_text("DialogPoseListLabelToolTip"));

	m_poseListModel = new QStandardItemModel(this);
	m_ui->poseListView->setModel(m_poseListModel);
	m_ui->poseListView->setDragEnabled(false);
	m_ui->poseListView->setAcceptDrops(false);
	m_ui->poseListView->setDragDropOverwriteMode(false);

	updateStyledUIComponents();
}

void PoseListWidget::updateStyledUIComponents()
{
	QString baseUrl = obs_frontend_is_theme_dark() ? getDataFolderPath() + "/icons/dark/"
						       : getDataFolderPath() + "/icons/light/";

	QString plusIconPath = QDir::fromNativeSeparators(baseUrl + "plus.svg");
	if (QFileInfo::exists(plusIconPath)) {
		QIcon plusIcon(plusIconPath);

		m_ui->addPoseToolButton->setIcon(plusIcon);
	}

	QString minusIconPath = QDir::fromNativeSeparators(baseUrl + "minus.svg");
	if (QFileInfo::exists(minusIconPath)) {
		QIcon minusIcon(minusIconPath);
	}

	QString trashIconPath = QDir::fromNativeSeparators(baseUrl + "trash.svg");
	if (QFileInfo::exists(trashIconPath)) {
		QIcon trashIcon(trashIconPath);

		m_ui->deletePoseToolButton->setIcon(trashIcon);
	}

	QString upIconPath = QDir::fromNativeSeparators(baseUrl + "up.svg");
	if (QFileInfo::exists(upIconPath)) {
		QIcon upIcon(upIconPath);
		m_ui->movePoseUpToolButton->setIcon(upIcon);
	}

	QString downIconPath = QDir::fromNativeSeparators(baseUrl + "down.svg");
	if (QFileInfo::exists(downIconPath)) {
		QIcon downIcon(downIconPath);
		m_ui->movePoseDownToolButton->setIcon(downIcon);
	}

	QString centerIconPath = QDir::fromNativeSeparators(baseUrl + "center.svg");
	if (QFileInfo::exists(centerIconPath)) {
		QIcon centerIcon(centerIconPath);
	}

	m_ui->poseListView->setStyleSheet("QListView::item {"
					  "   padding-top: 5px;"
					  "   padding-bottom: 5px;"
					  "}");
	m_ui->poseListLabel->setStyleSheet("font-weight: bold;");
}

//  ---------------------------------------------- Private Slots -----------------------------------------------

void PoseListWidget::addPose()
{
	if (!m_poseListModel)
		return obs_log(LOG_ERROR, "No list model found!");

	int count = m_poseListModel->rowCount();
	QStandardItem *poseItem = new QStandardItem(QString("New Pose %1").arg(count));
	m_poseListModel->appendRow(poseItem);
}

void PoseListWidget::deletePose()
{
	QModelIndex modelIndex = m_ui->poseListView->currentIndex();
	int rowIndex = modelIndex.row();

	obs_log(LOG_INFO, "Row To Delete: %d", rowIndex);

	if (rowIndex != -1) {
		m_poseListModel->takeRow(rowIndex);
	}
}

void PoseListWidget::handleMovePose(bool isDirectionUp)
{
	if (!m_poseListModel)
		return;

	m_isMovingPoseListRows = true;

	int selectedRow = getSelectedRow();
	if (selectedRow == -1)
		return;

	int targetRow = isDirectionUp ? selectedRow - 1 : selectedRow + 1;

	if (targetRow < 0 || targetRow >= m_poseListModel->rowCount())
		return;

	QStandardItem *currentItem = m_poseListModel->item(selectedRow, 0);
	QStandardItem *targetItem = m_poseListModel->item(targetRow, 0);

	if (!currentItem || !targetItem)
		return;

	QString tempText = currentItem->text();
	currentItem->setText(targetItem->text());
	targetItem->setText(tempText);

	m_ui->poseListView->setCurrentIndex(m_poseListModel->index(targetRow, 0));

	// SIGNAL that pose was moved
	emit rowMoved(selectedRow, targetRow);

	m_isMovingPoseListRows = false;
}

void PoseListWidget::handlePoseListClick(QModelIndex modelIndex)
{
	emit rowsSelected(modelIndex.row());
}

void PoseListWidget::onRowsInserted(const QModelIndex &parent, int first, int last)
{
	Q_UNUSED(parent);

	if (m_isMovingPoseListRows)
		return;

	QMap<int, QString> rowDataMap = {};

	for (int row = first; row <= last; ++row) {
		// E.g. read the poseId from the model
		QModelIndex idx = m_poseListModel->index(row, 0);
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

	if (m_isMovingPoseListRows)
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

	if (m_isMovingPoseListRows)
		return;

	QMap<int, QString> rowDataMap = {};

	for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
		// Read changed data from the model and update the Pose
		QModelIndex idx = m_poseListModel->index(row, 0);
		QString newPoseId = idx.data(Qt::DisplayRole).toString();
		rowDataMap.insert(row, newPoseId);
	}

	if (rowDataMap.size() > 0) {
		emit rowsDataChanged(rowDataMap);
	}
}
