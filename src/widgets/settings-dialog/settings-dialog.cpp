#include "settings-dialog.h"

SettingsDialog::SettingsDialog(QWidget *parent, QSharedPointer<TrackerData> tData, MainWidgetDock *mWidget)
	: QDialog(parent),
	  ui(new Ui::FaceTrackerDialog)
{
	ui->setupUi(this);
	trackerData = tData;
	mainWidget = mWidget;
	poseListWidget = new PoseListWidget(this, tData = trackerData);
	imageFilesWidget = new ImageFilesWidget(this, tData = trackerData);
	setTitle();

	connectUISignalHandlers();
	connectObsSignalHandlers();

	setupDialogUI(trackerData);
}

SettingsDialog::~SettingsDialog()
{
	// Disconnect OBS signals before the dialog is destroyed
	signal_handler_disconnect(obs_get_signal_handler(), "source_create", obsSourceCreated, ui);
	signal_handler_disconnect(obs_get_signal_handler(), "source_destroy", obsSourceDeleted, ui);
	signal_handler_disconnect(obs_get_signal_handler(), "source_rename", obsSourceRenamed, ui);

	delete ui;
	settingsPoseList.clear();
}

void SettingsDialog::updateStyledUIComponents()
{
	QString baseUrl = obs_frontend_is_theme_dark() ? getDataFolderPath() + "/icons/dark/"
						       : getDataFolderPath() + "/icons/light/";

	QString plusIconPath = QDir::fromNativeSeparators(baseUrl + "plus.svg");
	if (QFileInfo::exists(plusIconPath)) {
		QIcon plusIcon(plusIconPath);

		ui->zoomInToolButton->setIcon(plusIcon);
	}

	QString minusIconPath = QDir::fromNativeSeparators(baseUrl + "minus.svg");
	if (QFileInfo::exists(minusIconPath)) {
		QIcon minusIcon(minusIconPath);

		ui->zoomOutToolButton->setIcon(minusIcon);
	}

	QString upIconPath = QDir::fromNativeSeparators(baseUrl + "up.svg");
	if (QFileInfo::exists(upIconPath)) {
		QIcon upIcon(upIconPath);
		ui->moveImageUpLevelToolButton->setIcon(upIcon);
	}

	QString downIconPath = QDir::fromNativeSeparators(baseUrl + "down.svg");
	if (QFileInfo::exists(downIconPath)) {
		QIcon downIcon(downIconPath);
		ui->moveImageDownLevelToolButton->setIcon(downIcon);
	}

	QString centerIconPath = QDir::fromNativeSeparators(baseUrl + "center.svg");
	if (QFileInfo::exists(centerIconPath)) {
		QIcon centerIcon(centerIconPath);
		ui->centerOnImagesToolButton->setIcon(centerIcon);
	}

	ui->avatarGraphicsView->setStyleSheet("background-color: rgb(0, 0, 0);");
}

void SettingsDialog::setFormDetails(QSharedPointer<TrackerData> settingsDialogData)
{
	ui->ipAddressViewLabel->setText(NetworkTracking::getIpAddresses());

	if (settingsDialogData != nullptr) {
		ui->trackerIdLineEdit->setText(settingsDialogData->getTrackerId());

		int selectedImageSource =
			ui->imageSourceDropdownList->findText(settingsDialogData->getSelectedImageSource());
		if (selectedImageSource != -1) {
			ui->imageSourceDropdownList->setCurrentIndex(selectedImageSource);
		} else {
			ui->imageSourceDropdownList->setCurrentIndex(0);
		}

		ui->portSpinBox->setValue(settingsDialogData->getPort());
		ui->destIpAddressLineEdit->setText(settingsDialogData->getDestinationIpAddress());
		ui->destPortSpinBox->setValue(settingsDialogData->getDestinationPort());

		ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

		syncPoseListToModel();
	} else {
		obs_log(LOG_WARNING, "No tracker data found!");
	}
}

void SettingsDialog::connectUISignalHandlers()
{
	QObject::connect(ui->trackerIdLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::formChangeDetected);

	QObject::connect(ui->imageSourceDropdownList, &QComboBox::currentTextChanged, this,
			 &SettingsDialog::formChangeDetected);

	QObject::connect(ui->destIpAddressLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::formChangeDetected);

	QObject::connect(ui->destPortSpinBox, &QSpinBox::valueChanged, this, &SettingsDialog::formChangeDetected);

	QObject::connect(ui->portSpinBox, &QSpinBox::valueChanged, this, &SettingsDialog::formChangeDetected);

	QObject::connect(ui->dialogButtonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::okButtonClicked);

	QObject::connect(ui->dialogButtonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::cancelButtonClicked);

	QObject::connect(imageFilesWidget, &ImageFilesWidget::imageUrlSet, this, &SettingsDialog::handleSetImageUrl);
	QObject::connect(imageFilesWidget, &ImageFilesWidget::imageUrlCleared, this,
			 &SettingsDialog::handleClearImageUrl);
	QObject::connect(imageFilesWidget, &ImageFilesWidget::raiseWindow, this, &SettingsDialog::handleRaisedWindow);

	QObject::connect(ui->centerOnImagesToolButton, &QToolButton::clicked, this,
			 &SettingsDialog::handleCenterViewButtonClick);
	QObject::connect(ui->moveImageUpLevelToolButton, &QToolButton::clicked, this,
			 &SettingsDialog::handleMoveImageUpClick);
	QObject::connect(ui->moveImageDownLevelToolButton, &QToolButton::clicked, this,
			 &SettingsDialog::handleMoveImageDownClick);

	QObject::connect(ui->zoomInToolButton, &QToolButton::clicked, this, [this]() { handleImageZoomClick(false); });
	QObject::connect(ui->zoomOutToolButton, &QToolButton::clicked, this, [this]() { handleImageZoomClick(true); });

	QObject::connect(poseListWidget, &PoseListWidget::rowsInserted, this, &SettingsDialog::onPoseRowsInserted);
	QObject::connect(poseListWidget, &PoseListWidget::rowsRemoved, this, &SettingsDialog::onPoseRowsRemoved);
	QObject::connect(poseListWidget, &PoseListWidget::rowsDataChanged, this, &SettingsDialog::onPoseDataChanged);
	QObject::connect(poseListWidget, &PoseListWidget::rowsSelected, this, &SettingsDialog::onPoseSelected);
	QObject::connect(poseListWidget, &PoseListWidget::rowMoved, this, &SettingsDialog::onPoseRowMoved);

	QPushButton *applyButton = ui->dialogButtonBox->button(QDialogButtonBox::Apply);
	if (applyButton) {
		connect(applyButton, &QPushButton::clicked, this, &SettingsDialog::applyButtonClicked);
	}
}

void SettingsDialog::connectObsSignalHandlers()
{
	// Source Signals
	signal_handler_connect(obs_get_signal_handler(), "source_create", obsSourceCreated, ui);

	signal_handler_connect(obs_get_signal_handler(), "source_destroy", obsSourceDeleted, ui);

	signal_handler_connect(obs_get_signal_handler(), "source_rename", obsSourceRenamed, ui);
}

void SettingsDialog::setTitle()
{
	QString dialogTitle = QString("Tracker %1").arg(trackerData->getTrackerId());
	this->setWindowTitle(dialogTitle);
}

void SettingsDialog::setupDialogUI(QSharedPointer<TrackerData> settingsDialogData)
{
	// ------------------------------------------------ General Tab ------------------------------------------------

	ui->generalGroupBox->setTitle(obs_module_text("DialogGeneralGroupBoxTitle"));
	ui->connectionGroupBox->setTitle(obs_module_text("DialogConnectionGroupBoxTitle"));

	ui->trackerIdLabel->setText(obs_module_text("DialogIdLabel"));

	ui->imageSourceDropdownList->setToolTip(obs_module_text("SourceDropdownToolTip"));
	ui->imageSourceDropdownList->addItem("");
	ui->imageSourceNameLabel->setText(obs_module_text("DialogSourceLabel"));

	ui->settingsTabWidget->setTabText(0, obs_module_text("DialogGeneralTabName"));
	ui->settingsTabWidget->setTabText(1, obs_module_text("DialogTrackingTabName"));

	ui->destIpAddressLabel->setText(obs_module_text("DialogDestIpAddressLabel"));
	ui->destIpAddressLineEdit->setValidator(new QRegularExpressionValidator(
		QRegularExpression(R"(^((25[0-5]|2[0-4]\d|[01]?\d?\d)\.){3}(25[0-5]|2[0-4]\d|[01]?\d?\d)$)"), this));

	ui->portLabel->setText(obs_module_text("DialogPortAddress"));
	ui->portSpinBox->setRange(0, 65535);

	ui->ipAddressLabel->setText(obs_module_text("DialogIpAddressLabel"));
	ui->ipAddressViewLabel->setText(NetworkTracking::getIpAddresses());

	ui->destPortLabel->setText(obs_module_text("DialogDestPortAddressLabel"));
	ui->destPortSpinBox->setRange(0, 65535);

	ui->byLabel->setText(obs_module_text("DialogInfoByLabel"));
	ui->contributorsLabel->setText(obs_module_text("DialogInfoConstributorsLabel"));
	ui->versionLabel->setText(obs_module_text("DialogInfoVersionLabel"));
	ui->versionTextLabel->setText(PLUGIN_VERSION);

	ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setText(obs_module_text("DialogButtonApplyLabel"));
	ui->dialogButtonBox->button(QDialogButtonBox::Ok)->setText(obs_module_text("DialogButtonOkLabel"));
	ui->dialogButtonBox->button(QDialogButtonBox::Cancel)->setText(obs_module_text("DialogButtonCancelLabel"));

	// ----------------------------------------------- Tracking Tab -----------------------------------------------

	ui->poseVerticalLayout->addWidget(poseListWidget);
	ui->poseVerticalLayout->addWidget(imageFilesWidget);

	ui->centerOnImagesToolButton->setToolTip(obs_module_text("DialogCenterViewOnImagesToolTip"));
	ui->moveImageUpLevelToolButton->setToolTip(obs_module_text("DialogMoveImageUpLevelToolTip"));
	ui->moveImageDownLevelToolButton->setToolTip(obs_module_text("DialogMoveImageDownLevelToolTip"));
	ui->centerOnImagesToolButton->setToolTip(obs_module_text("DialogCenterViewOnImagesToolTip"));
	ui->zoomOutToolButton->setToolTip(obs_module_text("DialogZoomOutToolTip"));
	ui->zoomInToolButton->setToolTip(obs_module_text("DialogZoomInToolTip"));

	updateStyledUIComponents();

	getOBSSourceList();

	// Set form based on tracker data
	setFormDetails(settingsDialogData);
}

void SettingsDialog::applyFormChanges()
{
	isError = false;
	if (trackerData != nullptr) {
		QVector<Result> validationResults;

		validationResults.append(validateTrackerID());
		validationResults.append(validateDestIPAddress());

		QString errorMessages = "";

		for (const Result &res : validationResults) {
			if (!res.success) {
				isError = true;
				errorMessages += "-" + res.errorMessage + "\n\n";
			}
		}

		if (isError) {
			QMessageBox::warning(this, obs_module_text("DialogErrorPopupTitle"), errorMessages);
			return;
		}

		QSharedPointer<TrackerData> newData = QSharedPointer<TrackerData>::create();

		newData->setTrackerId(ui->trackerIdLineEdit->text());
		newData->setSelectedImageSource(ui->imageSourceDropdownList->currentText());
		newData->setPort(ui->portSpinBox->text().toInt());
		newData->setDestinationIpAddress(ui->destIpAddressLineEdit->text());
		newData->setDestinationPort(ui->destPortSpinBox->text().toInt());

		newData->copyListToPoseList(settingsPoseList);

		ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
		emit settingsUpdated(newData);
	} else {
		obs_log(LOG_WARNING, "No tracker data found!");
	}
}

Result SettingsDialog::validateTrackerID()
{
	Result updateIdResult = {true, ""};
	QLineEdit *idLineEdit = ui->trackerIdLineEdit;
	QString setTrackerId = idLineEdit->text();
	if ((setTrackerId != trackerData->getTrackerId() && mainWidget)) {
		updateIdResult = mainWidget->validateNewTrackerID(setTrackerId);
		if (updateIdResult.success == true) {
			idLineEdit->setStyleSheet("");
			setTitle();
		} else {
			idLineEdit->setStyleSheet(formErrorStyling);
			obs_log(LOG_WARNING, updateIdResult.errorMessage.toStdString().c_str());
		}
	}
	return updateIdResult;
}

Result SettingsDialog::validateDestIPAddress()
{
	Result destinationIdValidation = {true, ""};
	QLineEdit *lineEdit = ui->destIpAddressLineEdit;

	const QValidator *validator = lineEdit->validator();
	if (!validator) {
		// No validator set
		obs_log(LOG_WARNING, "No validator found!");
		return {false, ""};
	}

	QString text = lineEdit->text();
	if (text.isEmpty()) {
		return {true, ""};
	}

	int pos = 0;
	QValidator::State state = validator->validate(text, pos);

	if (state == QValidator::Acceptable) {
		lineEdit->setStyleSheet("");
	} else {
		lineEdit->setStyleSheet(formErrorStyling);
		destinationIdValidation = {false, obs_module_text("DialogTrackerDestIpValidationError")};
		obs_log(LOG_WARNING, destinationIdValidation.errorMessage.toStdString().c_str());
	}

	return destinationIdValidation;
}

void SettingsDialog::getOBSSourceList()
{
	// Get All Image Sources
	obs_enum_sources(getImageSources, ui->imageSourceDropdownList);
}

void SettingsDialog::obsSourceCreated(void *param, calldata_t *calldata)
{
	auto ui = static_cast<Ui::FaceTrackerDialog *>(param);
	obs_source_t *source;
	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	bool isImageSourceType = checkIfImageSourceType(source);
	// If not sourceType we need;
	if (!isImageSourceType)
		return;

	const char *name = obs_source_get_name(source);
	ui->imageSourceDropdownList->addItem(name);
};

void SettingsDialog::obsSourceDeleted(void *param, calldata_t *calldata)
{
	auto ui = static_cast<Ui::FaceTrackerDialog *>(param);

	obs_source_t *source;

	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	bool isImageSourceType = checkIfImageSourceType(source);
	// If not sourceType we need;
	if (!isImageSourceType)
		return;

	const char *name = obs_source_get_name(source);

	int textIndexToRemove = ui->imageSourceDropdownList->findText(name);
	ui->imageSourceDropdownList->removeItem(textIndexToRemove);
};

bool SettingsDialog::getImageSources(void *list_property, obs_source_t *source)
{
	if (!source)
		return true;
	bool isImageSourceType = checkIfImageSourceType(source);
	// If not sourceType we need;
	if (!isImageSourceType)
		return true;

	QComboBox *sourceListUi = static_cast<QComboBox *>(list_property);

	const char *name = obs_source_get_name(source);
	sourceListUi->addItem(name);
	return true;
}

void SettingsDialog::obsSourceRenamed(void *param, calldata_t *calldata)
{
	auto ui = static_cast<Ui::FaceTrackerDialog *>(param);

	obs_source_t *source;
	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	bool isImageSourceType = checkIfImageSourceType(source);
	// If not sourceType we need;
	if (!isImageSourceType)
		return;

	const char *newName = calldata_string(calldata, "new_name");
	const char *oldName = calldata_string(calldata, "prev_name");

	int textListIndex = ui->imageSourceDropdownList->findText(oldName);
	if (textListIndex == -1)
		return;
	ui->imageSourceDropdownList->setItemText(textListIndex, newName);
};

int SettingsDialog::checkIfImageSourceType(obs_source_t *source)
{
	const char *source_id = obs_source_get_unversioned_id(source);

	if (strcmp(source_id, "image_source") == 0)
		return true;

	return false;
}

void SettingsDialog::addImageToScene(PoseImageData *imageData, bool useImagePos, bool clearScene)
{
	if (!avatarPreviewScene || clearScene) {
		QGraphicsView *view = ui->avatarGraphicsView;
		avatarPreviewScene = new QGraphicsScene(this);

		avatarPreviewScene->setSceneRect(0, 0, 3000, 5000);
		view->scale(0.1, 0.1);

		view->setScene(avatarPreviewScene);
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
		QRectF sceneRect = avatarPreviewScene->sceneRect();
		qreal centerX = sceneRect.width() / 2;
		qreal centerY = sceneRect.height() / 2;

		auto *imagePixmap = static_cast<QGraphicsPixmapItem *>(imageData->getPixmapItem().data());

		qreal imagePixmapX = centerX - imagePixmap->pixmap().width() / 2;
		qreal imagePixmapY = centerY - imagePixmap->pixmap().height() / 2;

		imageData->setImagePosition(imagePixmapX, imagePixmapY);
	}
	avatarPreviewScene->addItem(imageData->getPixmapItem().data());

	QObject::connect(imageData->getPixmapItem().data(), &MovablePixmapItem::positionChanged, this,
			 &SettingsDialog::handleImageMove);

	centerSceneOnItems();
}

void SettingsDialog::clearScene()
{
	if (avatarPreviewScene) {
		QList<QGraphicsItem *> items = avatarPreviewScene->items();
		for (QGraphicsItem *item : items) {
			if (dynamic_cast<MovablePixmapItem *>(item)) {
				avatarPreviewScene->removeItem(item);
			}
		}
	}
}

void SettingsDialog::clearCurrentPoseConfig()
{
	imageFilesWidget->clearSelection();
	clearScene();
}

void SettingsDialog::loadSelectedPoseConfig()
{
	int selectedRow = getSelectedRow();
	if (selectedRow == -1)
		return;

	if (selectedRow == previouslySelectedPoseIndex)
		return;

	previouslySelectedPoseIndex = selectedRow;

	QSharedPointer<Pose> selectedPose = settingsPoseList[selectedRow];
	clearCurrentPoseConfig();
	clearScene();
	imageFilesWidget->toggleVisible(true);

	for (size_t i = 0; i < static_cast<size_t>(PoseImage::COUNT); ++i) {
		auto poseEnum = static_cast<PoseImage>(i);
		QMap<PoseImage, QLineEdit *> poseImageLineEdits = imageFilesWidget->getposeLineEditsMap();
		auto it = poseImageLineEdits.find(poseEnum);
		if (it != poseImageLineEdits.end()) {
			QLineEdit *lineEdit = it.value();
			QString fileName = selectedPose->getPoseImageData(poseEnum)->getImageUrl();
			if (lineEdit) {
				lineEdit->setText(fileName);
			}

			if (fileName.isEmpty())
				return;

			if (!FileExists(fileName)) {
				obs_log(LOG_WARNING, QString("Image file: %1 not found when loading pose!")
							     .arg(fileName)
							     .toStdString()
							     .c_str());
				break;
			}

			PoseImageData *imageData = selectedPose->getPoseImageData(poseEnum);

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
}

int SettingsDialog::getSelectedRow()
{
	return poseListWidget->getSelectedRow();
}

void SettingsDialog::resetPoseUITab()
{
	previouslySelectedPoseIndex = -1;
	clearCurrentPoseConfig();
	imageFilesWidget->toggleVisible(false);
}

void SettingsDialog::centerSceneOnItems()
{
	if (!avatarPreviewScene)
		return;

	if (avatarPreviewScene->items().isEmpty()) {
		return; // No items to center on
	}

	// Calculate the bounding rectangle of all items
	QRectF boundingRect = avatarPreviewScene->itemsBoundingRect();

	// Center the view on the center of the bounding rectangle
	ui->avatarGraphicsView->centerOn(boundingRect.center());

	QRectF paddedRect = boundingRect.adjusted(-10, -10, 10, 10);
	ui->avatarGraphicsView->fitInView(paddedRect, Qt::KeepAspectRatio);
}

//  ------------------------------------------------ Protected ------------------------------------------------

void SettingsDialog::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);

	// Reset stylings
	ui->trackerIdLineEdit->setStyleSheet("");
	ui->destIpAddressLineEdit->setStyleSheet("");

	ui->ipAddressViewLabel->setText(NetworkTracking::getIpAddresses());

	clearCurrentPoseConfig();
}

void SettingsDialog::closeEvent(QCloseEvent *)
{
	cancelButtonClicked();
}

//  ---------------------------------------------- Private Slots -----------------------------------------------

void SettingsDialog::handleRaisedWindow()
{
	this->raise();
	this->activateWindow();
}

void SettingsDialog::formChangeDetected()
{
	ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void SettingsDialog::applyButtonClicked()
{
	applyFormChanges();
}

void SettingsDialog::cancelButtonClicked()
{
	setFormDetails(trackerData);
	resetPoseUITab();
	this->reject();
}

void SettingsDialog::okButtonClicked()
{
	applyFormChanges();
	resetPoseUITab();
	if (!isError)
		this->reject();
}

void SettingsDialog::syncPoseListToModel()
{
	// Disconnect setting row update signal during sync with tracker data
	QObject::disconnect(poseListWidget, &PoseListWidget::rowsInserted, this, &SettingsDialog::onPoseRowsInserted);

	poseListWidget->clearList();

	settingsPoseList.clear();
	previouslySelectedPoseIndex = -1;

	for (int i = 0; i < trackerData->getPoseList().size(); ++i) {
		QSharedPointer<Pose> originalPosePtr = trackerData->getPoseList()[i];
		if (!originalPosePtr) {
			obs_log(LOG_WARNING, QString("Pose pointer at index %1 is null.").arg(i).toStdString().c_str());
			continue; // Skip null pointers
		}

		// Clone the Pose to ensure an independent copy
		QSharedPointer<Pose> clonedPosePtr = originalPosePtr->clone();
		settingsPoseList.append(clonedPosePtr);

		poseListWidget->addRow(clonedPosePtr->getPoseId());
	}
	// Reenable signal
	QObject::connect(poseListWidget, &PoseListWidget::rowsInserted, this, &SettingsDialog::onPoseRowsInserted);
}

void SettingsDialog::handleSetImageUrl(PoseImage poseEnum, QString fileName)
{
	int selectedRow = getSelectedRow();
	if (selectedRow == -1)
		return;

	QSharedPointer<Pose> selectedPose = settingsPoseList[selectedRow];

	auto imageIndex = static_cast<int>(poseEnum);

	formChangeDetected();

	selectedPose->getPoseImageAt(imageIndex)->setImageUrl(fileName);

	if (selectedPose->getPoseImageAt(imageIndex)->getPixmapItem()) {
		selectedPose->getPoseImageAt(imageIndex)->clearPixmapItem();
	}

	QPixmap pixmap(fileName);
	if (pixmap.isNull()) {
		obs_log(LOG_WARNING,
			QString("Failed to load pixmap from file: %1").arg(fileName).toStdString().c_str());
		selectedPose->getPoseImageAt(imageIndex)->clearPixmapItem();
		return;
	}

	selectedPose->getPoseImageAt(imageIndex)
		->setPixmapItem(QSharedPointer<MovablePixmapItem>(new MovablePixmapItem(pixmap)));

	switch (poseEnum) {
	case PoseImage::BODY:
	case PoseImage::MOUTHCLOSED:
	case PoseImage::EYESOPEN:
		addImageToScene(selectedPose->getPoseImageData(poseEnum));
		break;

	default:
		break;
	}
}

void SettingsDialog::handleClearImageUrl(int imageIndex)
{
	int selectedRow = getSelectedRow();
	if (selectedRow == -1)
		return;

	formChangeDetected();

	QSharedPointer<Pose> selectedPose = settingsPoseList[selectedRow];
	PoseImageData *poseData = selectedPose->getPoseImageAt(imageIndex);

	poseData->setImageUrl(QString());
	if (poseData->getPixmapItem()) {
		poseData->clearPixmapItem();
	}
}

void SettingsDialog::onPoseSelected(int rowIndex)
{
	UNUSED_PARAMETER(rowIndex);
	obs_log(LOG_INFO, "Pose selected: %d", rowIndex);
	loadSelectedPoseConfig();
}

void SettingsDialog::onPoseRowsInserted(QMap<int, QString> rowMap)
{
	if (isMovingPoseListRows)
		return;

	// Sort into ascending order to prevent indices lower down affecting indices higher
	QList<int> sortedKeys = rowMap.keys();
	std::sort(sortedKeys.begin(), sortedKeys.end());

	QString result = "{ ";
	for (auto it = rowMap.constBegin(); it != rowMap.constEnd(); ++it) {
		result += QString::number(it.key()) + ": \"" + it.value() + "\", ";
	}
	if (rowMap.isEmpty())
		result.chop(2); // Remove the trailing ", "
	result += " }";

	obs_log(LOG_INFO, "Map: %s", result.toStdString().c_str());

	for (const int &row : sortedKeys) {
		if (row < 0 || row > settingsPoseList.size()) {
			obs_log(LOG_WARNING, "Invalid pose at row: %d", row);
			continue;
		}
		obs_log(LOG_INFO, "Adding row: %d", row);
		QSharedPointer<Pose> newPose = QSharedPointer<Pose>::create();
		newPose->setPoseId(rowMap.value(row));
		settingsPoseList.insert(row, newPose);
	}

	loadSelectedPoseConfig();
	formChangeDetected();
}

void SettingsDialog::onPoseRowsRemoved(QList<int> rowsList)
{
	if (isMovingPoseListRows)
		return;

	// Sort from greatest index to lowest to ensure no issues
	// deleting indices that dont already exist
	QList<int> sortedRows = rowsList;
	std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());

	for (const int &row : sortedRows) {
		if (row >= 0 && row < settingsPoseList.size()) {
			settingsPoseList.removeAt(row);
			obs_log(LOG_INFO, "Pose removed: %d", row);
		} else {
			obs_log(LOG_WARNING, "Invalid row index: %d", row);
		}
	}
	previouslySelectedPoseIndex = -1;
	loadSelectedPoseConfig();
	formChangeDetected();
}

void SettingsDialog::onPoseDataChanged(QMap<int, QString> rowMap)
{
	for (auto i = rowMap.cbegin(), end = rowMap.cend(); i != end; i++) {
		int row = i.key();
		if (row >= 0 && row < settingsPoseList.size()) {
			obs_log(LOG_INFO, "Pose data changed: %d", row);
			settingsPoseList[row]->setPoseId(i.value());
		}
	}

	formChangeDetected();
}

void SettingsDialog::onPoseRowMoved(int sourceRow, int targetRow)
{
	obs_log(LOG_INFO, "Swapping from row: %d to row: %d", sourceRow, targetRow);

	obs_log(LOG_INFO, "[SettingsDialog] Before swapping:");
	for (int i = 0; i < settingsPoseList.size(); ++i) {
		obs_log(LOG_INFO, "Row %d : %s", i, settingsPoseList[i]->getPoseId().toStdString().c_str());
	}

	settingsPoseList.swapItemsAt(sourceRow, targetRow);
	previouslySelectedPoseIndex = targetRow;

	obs_log(LOG_INFO, "[SettingsDialog] After swapping:");
	for (int i = 0; i < settingsPoseList.size(); ++i) {
		obs_log(LOG_INFO, "Row %d : %s", i, settingsPoseList[i]->getPoseId().toStdString().c_str());
	}

	loadSelectedPoseConfig();
	formChangeDetected();
}

void SettingsDialog::handleCenterViewButtonClick()
{
	centerSceneOnItems();
}

void SettingsDialog::handleMoveImageUpClick()
{
	if (!avatarPreviewScene)
		return;

	if (avatarPreviewScene->items().isEmpty() || avatarPreviewScene->selectedItems().isEmpty()) {
		return;
	}

	qreal maxZIndex = 0;
	for (QGraphicsItem *item : avatarPreviewScene->items()) {
		maxZIndex = std::max(maxZIndex, item->zValue());
	}

	int itemsAtMaxZ = 0;
	for (QGraphicsItem *item : avatarPreviewScene->items()) {
		if (item->zValue() == maxZIndex)
			itemsAtMaxZ++;
	}

	obs_log(LOG_INFO, "Max Z Level: %d", static_cast<int>(maxZIndex));

	for (QGraphicsItem *item : avatarPreviewScene->selectedItems()) {
		qreal itemZValue = item->zValue();
		if (itemZValue < maxZIndex || (itemZValue == maxZIndex && itemsAtMaxZ > 1)) {
			item->setZValue(item->zValue() + 1);
		}
		obs_log(LOG_INFO, "New Z Level: %d", static_cast<int>(item->zValue()));
	}
}

void SettingsDialog::handleMoveImageDownClick()
{
	if (!avatarPreviewScene)
		return;

	if (avatarPreviewScene->items().isEmpty() || avatarPreviewScene->selectedItems().isEmpty()) {
		return;
	}
	for (QGraphicsItem *item : avatarPreviewScene->selectedItems()) {
		qreal zIndex = item->zValue();
		if (zIndex != 0) {
			item->setZValue(item->zValue() - 1);
		}
		obs_log(LOG_INFO, "New Z Level: %d", static_cast<int>(item->zValue()));
	}
}

void SettingsDialog::handleImageZoomClick(bool isZoomOut)
{
	double zoomScaleFactor = 1.15;

	if (isZoomOut) {
		ui->avatarGraphicsView->scale(1.0 / zoomScaleFactor, 1.0 / zoomScaleFactor);
	} else {
		ui->avatarGraphicsView->scale(zoomScaleFactor, zoomScaleFactor);
	}
}

void SettingsDialog::handleImageMove(qreal x, qreal y, qreal z, PoseImage pImageType)
{
	int selectedRow = getSelectedRow();

	obs_log(LOG_INFO, QString("Updated Pose %1: x=%2, y=%3, z=%4").arg(x).arg(y).arg(z).toStdString().c_str());

	if (selectedRow != -1) {
		QSharedPointer<Pose> selectedPose = settingsPoseList[selectedRow];
		switch (pImageType) {
		case PoseImage::BODY:
			selectedPose->setBodyPosition({x, y, z});
			/* code */
			break;
		case PoseImage::MOUTHCLOSED:
			selectedPose->setMouthPosition({x, y, z});
			break;
		case PoseImage::EYESOPEN:
			selectedPose->setEyesPosition({x, y, z});
			break;

		default:
			break;
		}
		// Optionally, mark the form as changed
		formChangeDetected();
		// Log the update
		obs_log(LOG_INFO, QString("Updated Pose %1: x=%2, y=%3, z=%4")
					  .arg(selectedPose->getPoseId())
					  .arg(x)
					  .arg(y)
					  .arg(z)
					  .toStdString()
					  .c_str());
	}

	formChangeDetected();
}
