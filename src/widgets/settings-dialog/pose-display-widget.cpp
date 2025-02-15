#include "pose-display-widget.h"
#include <obs-module.h>

PoseDisplayWidget::PoseDisplayWidget(QWidget *parent, QSharedPointer<Pose> pose)
	: QWidget(parent),
	  m_ui(new Ui::PoseImageDisplayWidget),
	  m_pose(pose)
{
	m_ui->setupUi(this);

	if (pose)
		setData(pose);

	setupWidgetUI();

	connectUISignalHandlers();
}

PoseDisplayWidget::~PoseDisplayWidget() {}

void PoseDisplayWidget::clearSelection()
{
	if (m_avatarPreviewScene) {
		QList<QGraphicsItem *> items = m_avatarPreviewScene->items();
		for (QGraphicsItem *item : items) {
			if (dynamic_cast<MovablePixmapItem *>(item)) {
				m_avatarPreviewScene->removeItem(item);
			}
		}
	}
}

void PoseDisplayWidget::updateStyledUIComponents()
{
	QString baseUrl = obs_frontend_is_theme_dark() ? getDataFolderPath() + "/icons/dark/"
						       : getDataFolderPath() + "/icons/light/";

	QString plusIconPath = QDir::fromNativeSeparators(baseUrl + "plus.svg");
	if (QFileInfo::exists(plusIconPath)) {
		QIcon plusIcon(plusIconPath);

		m_ui->zoomInToolButton->setIcon(plusIcon);
	}

	QString minusIconPath = QDir::fromNativeSeparators(baseUrl + "minus.svg");
	if (QFileInfo::exists(minusIconPath)) {
		QIcon minusIcon(minusIconPath);

		m_ui->zoomOutToolButton->setIcon(minusIcon);
	}

	QString upIconPath = QDir::fromNativeSeparators(baseUrl + "up.svg");
	if (QFileInfo::exists(upIconPath)) {
		QIcon upIcon(upIconPath);
		m_ui->moveImageUpLevelToolButton->setIcon(upIcon);
	}

	QString downIconPath = QDir::fromNativeSeparators(baseUrl + "down.svg");
	if (QFileInfo::exists(downIconPath)) {
		QIcon downIcon(downIconPath);
		m_ui->moveImageDownLevelToolButton->setIcon(downIcon);
	}

	QString centerIconPath = QDir::fromNativeSeparators(baseUrl + "center.svg");
	if (QFileInfo::exists(centerIconPath)) {
		QIcon centerIcon(centerIconPath);
		m_ui->centerOnImagesToolButton->setIcon(centerIcon);
	}
}

void PoseDisplayWidget::addImageToScene(PoseImageData *imageData, bool useImagePos, bool clearScene)
{
	if (!m_avatarPreviewScene || clearScene) {
		QGraphicsView *view = m_ui->avatarGraphicsView;
		m_avatarPreviewScene = QSharedPointer<QGraphicsScene>::create(this);

		m_avatarPreviewScene->setSceneRect(0, 0, 3000, 5000);
		view->scale(0.1, 0.1);

		view->setScene(m_avatarPreviewScene.data());
		view->setRenderHint(QPainter::Antialiasing);
		// Create a QGraphicsView to visualize the scene
		view->setDragMode(QGraphicsView::ScrollHandDrag);
	}

	if (imageData->getPixmapItem() == nullptr)
		return obs_log(LOG_ERROR, QString("Image data for file: %1 not found!")
						  .arg(imageData->getImageUrl())
						  .toStdString()
						  .c_str());

	if (!useImagePos) {
		// Place image in center of scene
		QRectF sceneRect = m_avatarPreviewScene->sceneRect();
		qreal centerX = sceneRect.width() / 2;
		qreal centerY = sceneRect.height() / 2;

		auto *imagePixmap = static_cast<QGraphicsPixmapItem *>(imageData->getPixmapItem().data());

		qreal imagePixmapX = centerX - imagePixmap->pixmap().width() / 2;
		qreal imagePixmapY = centerY - imagePixmap->pixmap().height() / 2;

		imageData->setImagePosition(imagePixmapX, imagePixmapY);
	}
	m_avatarPreviewScene->addItem(imageData->getPixmapItem().data());

	QObject::connect(imageData->getPixmapItem().data(), &MovablePixmapItem::positionChanged, this,
			 &PoseDisplayWidget::handleImageMove);

	centerSceneOnItems();
}

void PoseDisplayWidget::setData(QSharedPointer<Pose> in_pose)
{
	clearSelection();

	if (in_pose)
		m_pose = in_pose;

	if (!m_pose)
		return obs_log(LOG_WARNING, "No pose data found when loading pose images!");

	for (size_t i = 0; i < static_cast<size_t>(PoseImage::COUNT); ++i) {
		auto poseEnum = static_cast<PoseImage>(i);
		QString fileName = m_pose->getPoseImageData(poseEnum)->getImageUrl();

		if (fileName.isEmpty())
			continue;

		if (!fileExists(fileName)) {
			obs_log(LOG_WARNING, QString("Image file: %1 not found when loading pose!")
						     .arg(fileName)
						     .toStdString()
						     .c_str());
			continue;
		}

		PoseImageData *imageData = m_pose->getPoseImageData(poseEnum);

		switch (poseEnum) {
		case PoseImage::BODY:
			addImageToScene(imageData, true);
			break;
		case PoseImage::MOUTHCLOSED:
			addImageToScene(imageData, true);
			break;
		case PoseImage::EYESOPEN:
			addImageToScene(imageData, true);
			break;

		default:
			break;
		}
	}
}

void PoseDisplayWidget::centerSceneOnItems()
{
	if (!m_avatarPreviewScene)
		return;

	if (m_avatarPreviewScene->items().isEmpty()) {
		return; // No items to center on
	}

	// Calculate the bounding rectangle of all items
	QRectF boundingRect = m_avatarPreviewScene->itemsBoundingRect();

	// Center the view on the center of the bounding rectangle
	m_ui->avatarGraphicsView->centerOn(boundingRect.center());

	QRectF paddedRect = boundingRect.adjusted(-10, -10, 10, 10);
	m_ui->avatarGraphicsView->fitInView(paddedRect, Qt::KeepAspectRatio);
}

//  ------------------------------------------------- Private --------------------------------------------------

void PoseDisplayWidget::connectUISignalHandlers()
{
	QObject::connect(m_ui->centerOnImagesToolButton, &QToolButton::clicked, this,
			 &PoseDisplayWidget::handleCenterViewButtonClick);
	QObject::connect(m_ui->moveImageUpLevelToolButton, &QToolButton::clicked, this,
			 &PoseDisplayWidget::handleMoveImageUpClick);
	QObject::connect(m_ui->moveImageDownLevelToolButton, &QToolButton::clicked, this,
			 &PoseDisplayWidget::handleMoveImageDownClick);

	QObject::connect(m_ui->zoomInToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageZoomClick(false); });
	QObject::connect(m_ui->zoomOutToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageZoomClick(true); });
}

void PoseDisplayWidget::setupWidgetUI()
{
	m_ui->centerOnImagesToolButton->setToolTip(obs_module_text("DialogCenterViewOnImagesToolTip"));
	m_ui->moveImageUpLevelToolButton->setToolTip(obs_module_text("DialogMoveImageUpLevelToolTip"));
	m_ui->moveImageDownLevelToolButton->setToolTip(obs_module_text("DialogMoveImageDownLevelToolTip"));
	m_ui->centerOnImagesToolButton->setToolTip(obs_module_text("DialogCenterViewOnImagesToolTip"));
	m_ui->zoomOutToolButton->setToolTip(obs_module_text("DialogZoomOutToolTip"));
	m_ui->zoomInToolButton->setToolTip(obs_module_text("DialogZoomInToolTip"));

	updateStyledUIComponents();
}

//  ----------------------------------------------- Public Slots -----------------------------------------------

void PoseDisplayWidget::handleSetImageUrl(PoseImage poseEnum, QString fileName)
{
	if (!m_pose)
		return;

	auto imageIndex = static_cast<int>(poseEnum);

	emit poseImagesChanged();

	if (m_pose->getPoseImageAt(imageIndex)->getPixmapItem()) {
		m_pose->getPoseImageAt(imageIndex)->clearPixmapItem();
	}

	QPixmap pixmap(fileName);
	if (pixmap.isNull()) {
		obs_log(LOG_WARNING,
			QString("Failed to load pixmap from file: %1").arg(fileName).toStdString().c_str());
		m_pose->getPoseImageAt(imageIndex)->clearPixmapItem();
		return;
	}

	m_pose->getPoseImageAt(imageIndex)
		->setPixmapItem(QSharedPointer<MovablePixmapItem>(new MovablePixmapItem(pixmap, this, poseEnum)));

	switch (poseEnum) {
	case PoseImage::BODY:
	case PoseImage::MOUTHCLOSED:
	case PoseImage::EYESOPEN:
		addImageToScene(m_pose->getPoseImageData(poseEnum));
		break;

	default:
		break;
	}
}

void PoseDisplayWidget::handleClearImageUrl(int imageIndex)
{
	if (!m_pose)
		return;

	emit poseImagesChanged();

	PoseImageData *poseData = m_pose->getPoseImageAt(imageIndex);

	if (poseData->getPixmapItem()) {
		poseData->clearPixmapItem();
	}
}

//  ---------------------------------------------- Private Slots -----------------------------------------------

void PoseDisplayWidget::setImagesFromPose(PoseImage poseEnum, QString fileName)
{

	if (!fileExists(fileName)) {
		obs_log(LOG_WARNING, QString("Image file: %1 not found!").arg(fileName).toStdString().c_str());
		return;
	}

	auto imageIndex = static_cast<int>(poseEnum);
	m_pose->getPoseImageAt(imageIndex)->setImageUrl(fileName);
}

void PoseDisplayWidget::clearAllImages(PoseImage poseEnum)
{
	int imageIndex = static_cast<int>(poseEnum);

	if (imageIndex < 0)
		return;

	if (m_pose && imageIndex < static_cast<int>(PoseImage::COUNT)) {
		PoseImageData *poseData = m_pose->getPoseImageAt(imageIndex);
		poseData->setImageUrl(QString());
	} else {
		obs_log(LOG_WARNING, "Invalid PoseImage enum value.");
	}
}

void PoseDisplayWidget::handleCenterViewButtonClick()
{
	centerSceneOnItems();
}

void PoseDisplayWidget::handleMoveImageUpClick()
{
	if (!m_avatarPreviewScene)
		return;

	if (m_avatarPreviewScene->items().isEmpty() || m_avatarPreviewScene->selectedItems().isEmpty()) {
		return;
	}

	qreal maxZIndex = 0;
	for (QGraphicsItem *item : m_avatarPreviewScene->items()) {
		maxZIndex = std::max(maxZIndex, item->zValue());
	}

	int itemsAtMaxZ = 0;
	for (QGraphicsItem *item : m_avatarPreviewScene->items()) {
		if (item->zValue() == maxZIndex)
			itemsAtMaxZ++;
	}

	obs_log(LOG_INFO, "Max Z Level: %d", static_cast<int>(maxZIndex));

	for (QGraphicsItem *item : m_avatarPreviewScene->selectedItems()) {
		qreal itemZValue = item->zValue();
		if (itemZValue < maxZIndex || (itemZValue == maxZIndex && itemsAtMaxZ > 1)) {
			item->setZValue(item->zValue() + 1);
		}
		obs_log(LOG_INFO, "New Z Level: %d", static_cast<int>(item->zValue()));
	}
}

void PoseDisplayWidget::handleMoveImageDownClick()
{
	if (!m_avatarPreviewScene)
		return;

	if (m_avatarPreviewScene->items().isEmpty() || m_avatarPreviewScene->selectedItems().isEmpty()) {
		return;
	}
	for (QGraphicsItem *item : m_avatarPreviewScene->selectedItems()) {
		qreal zIndex = item->zValue();
		if (zIndex != 0) {
			item->setZValue(item->zValue() - 1);
		}
		obs_log(LOG_INFO, "New Z Level: %d", static_cast<int>(item->zValue()));
	}
}

void PoseDisplayWidget::handleImageZoomClick(bool isZoomOut)
{
	if (isZoomOut) {
		m_ui->avatarGraphicsView->scale(1.0 / zoomScaleFactor, 1.0 / zoomScaleFactor);
	} else {
		m_ui->avatarGraphicsView->scale(zoomScaleFactor, zoomScaleFactor);
	}
}

void PoseDisplayWidget::handleImageMove(qreal x, qreal y, qreal z, PoseImage pImageType)
{
	if (!m_pose)
		return;

	obs_log(LOG_INFO, QString("Updated Pose %1: x=%2, y=%3, z=%4").arg(x).arg(y).arg(z).toStdString().c_str());

	switch (pImageType) {
	case PoseImage::BODY:
		m_pose->setBodyPosition({x, y, z});
		break;
	case PoseImage::MOUTHCLOSED:
		m_pose->setMouthPosition({x, y, z});
		break;
	case PoseImage::EYESOPEN:
		m_pose->setEyesPosition({x, y, z});
		break;

	default:
		break;
	}

	// Log the update
	// obs_log(LOG_INFO, QString("Updated Pose %1: Type: %2 x=%3, y=%4, z=%5")
	// 			  .arg(m_pose->getPoseId())
	// 			  .arg(poseImageToString(pImageType))
	// 			  .arg(x)
	// 			  .arg(y)
	// 			  .arg(z)
	// 			  .toStdString()
	// 			  .c_str());

	emit poseImagesChanged();
}
