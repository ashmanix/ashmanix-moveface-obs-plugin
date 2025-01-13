#include "settings-dialog.h"

SettingsDialog::SettingsDialog(QWidget *parent, QSharedPointer<TrackerData> tData, MainWidgetDock *mWidget)
	: QDialog(parent),
	  m_ui(new Ui::FaceTrackerDialog)
{
	m_ui->setupUi(this);
	m_trackerData = tData;
	m_mainWidget = mWidget;
	m_poseListWidget = new PoseListWidget(this, m_trackerData);
	m_imageFilesWidget = new ImageFilesWidget(this);
	m_faceSettingsWidget = new FaceSettingsWidget(this);
	m_blendshapeRulesWidget = new BlendshapeRulesWidget(this);

	setTitle();

	connectUISignalHandlers();
	connectObsSignalHandlers();

	setupDialogUI(m_trackerData);
}

SettingsDialog::~SettingsDialog()
{
	// Disconnect OBS signals before the dialog is destroyed
	signal_handler_disconnect(obs_get_signal_handler(), "source_create", obsSourceCreated, m_ui);
	signal_handler_disconnect(obs_get_signal_handler(), "source_destroy", obsSourceDeleted, m_ui);
	signal_handler_disconnect(obs_get_signal_handler(), "source_rename", obsSourceRenamed, m_ui);

	delete m_ui;
	m_settingsPoseList.clear();
}

void SettingsDialog::setFormDetails(QSharedPointer<TrackerData> settingsDialogData)
{
	m_ui->ipAddressViewLabel->setText(NetworkTracking::getIpAddresses());

	if (settingsDialogData != nullptr) {
		m_ui->trackerIdLineEdit->setText(settingsDialogData->getTrackerId());

		int selectedImageSource =
			m_ui->imageSourceDropdownList->findText(settingsDialogData->getSelectedImageSource());
		if (selectedImageSource != -1) {
			m_ui->imageSourceDropdownList->setCurrentIndex(selectedImageSource);
		} else {
			m_ui->imageSourceDropdownList->setCurrentIndex(0);
		}

		m_ui->portSpinBox->setValue(settingsDialogData->getPort());
		m_ui->destIpAddressLineEdit->setText(settingsDialogData->getDestinationIpAddress());
		m_ui->destPortSpinBox->setValue(settingsDialogData->getDestinationPort());

		m_ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

		syncPoseListToModel();
	} else {
		obs_log(LOG_WARNING, "No tracker data found!");
	}
}

void SettingsDialog::updateStyledUIComponents()
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

	m_ui->avatarGraphicsView->setStyleSheet("background-color: rgb(0, 0, 0);");

	m_imageFilesWidget->updateStyledUIComponents();
	m_faceSettingsWidget->updateStyledUIComponents();
	m_blendshapeRulesWidget->updateStyledUIComponents();
}

void SettingsDialog::connectUISignalHandlers()
{
	QObject::connect(m_ui->trackerIdLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::formChangeDetected);

	QObject::connect(m_ui->imageSourceDropdownList, &QComboBox::currentTextChanged, this,
			 &SettingsDialog::formChangeDetected);

	QObject::connect(m_ui->destIpAddressLineEdit, &QLineEdit::textChanged, this,
			 &SettingsDialog::formChangeDetected);

	QObject::connect(m_ui->destPortSpinBox, &QSpinBox::valueChanged, this, &SettingsDialog::formChangeDetected);

	QObject::connect(m_ui->portSpinBox, &QSpinBox::valueChanged, this, &SettingsDialog::formChangeDetected);

	QObject::connect(m_ui->dialogButtonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::okButtonClicked);

	QObject::connect(m_ui->dialogButtonBox, &QDialogButtonBox::rejected, this,
			 &SettingsDialog::cancelButtonClicked);

	QObject::connect(m_imageFilesWidget, &ImageFilesWidget::imageUrlSet, this, &SettingsDialog::handleSetImageUrl);
	QObject::connect(m_imageFilesWidget, &ImageFilesWidget::imageUrlCleared, this,
			 &SettingsDialog::handleClearImageUrl);
	QObject::connect(m_imageFilesWidget, &ImageFilesWidget::raiseWindow, this, &SettingsDialog::handleRaisedWindow);

	QObject::connect(m_ui->centerOnImagesToolButton, &QToolButton::clicked, this,
			 &SettingsDialog::handleCenterViewButtonClick);
	QObject::connect(m_ui->moveImageUpLevelToolButton, &QToolButton::clicked, this,
			 &SettingsDialog::handleMoveImageUpClick);
	QObject::connect(m_ui->moveImageDownLevelToolButton, &QToolButton::clicked, this,
			 &SettingsDialog::handleMoveImageDownClick);

	QObject::connect(m_ui->zoomInToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageZoomClick(false); });
	QObject::connect(m_ui->zoomOutToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageZoomClick(true); });

	QObject::connect(m_poseListWidget, &PoseListWidget::rowsInserted, this, &SettingsDialog::onPoseRowsInserted);
	QObject::connect(m_poseListWidget, &PoseListWidget::rowsRemoved, this, &SettingsDialog::onPoseRowsRemoved);
	QObject::connect(m_poseListWidget, &PoseListWidget::rowsDataChanged, this, &SettingsDialog::onPoseDataChanged);
	QObject::connect(m_poseListWidget, &PoseListWidget::rowsSelected, this, &SettingsDialog::onPoseSelected);
	QObject::connect(m_poseListWidget, &PoseListWidget::rowMoved, this, &SettingsDialog::onPoseRowMoved);

	QObject::connect(m_faceSettingsWidget, &FaceSettingsWidget::blendshapeLimitChanged, this,
			 &SettingsDialog::handleBlendshapelimitChange);

	QObject::connect(m_blendshapeRulesWidget, &BlendshapeRulesWidget::blendshapeRuleChanged, this,
			 &SettingsDialog::formChangeDetected);

	QPushButton *applyButton = m_ui->dialogButtonBox->button(QDialogButtonBox::Apply);
	if (applyButton) {
		connect(applyButton, &QPushButton::clicked, this, &SettingsDialog::applyButtonClicked);
	}
}

void SettingsDialog::connectObsSignalHandlers()
{
	// Source Signals
	signal_handler_connect(obs_get_signal_handler(), "source_create", obsSourceCreated, m_ui);

	signal_handler_connect(obs_get_signal_handler(), "source_destroy", obsSourceDeleted, m_ui);

	signal_handler_connect(obs_get_signal_handler(), "source_rename", obsSourceRenamed, m_ui);
}

void SettingsDialog::setTitle()
{
	QString dialogTitle = QString("Tracker %1").arg(m_trackerData->getTrackerId());
	this->setWindowTitle(dialogTitle);
}

void SettingsDialog::setupDialogUI(QSharedPointer<TrackerData> settingsDialogData)
{
	// ------------------------------------------------ General Tab ------------------------------------------------

	m_ui->generalGroupBox->setTitle(obs_module_text("DialogGeneralGroupBoxTitle"));
	m_ui->connectionGroupBox->setTitle(obs_module_text("DialogConnectionGroupBoxTitle"));

	m_ui->trackerIdLabel->setText(obs_module_text("DialogIdLabel"));

	m_ui->imageSourceDropdownList->setToolTip(obs_module_text("SourceDropdownToolTip"));
	m_ui->imageSourceDropdownList->addItem("");
	m_ui->imageSourceNameLabel->setText(obs_module_text("DialogSourceLabel"));

	m_ui->settingsTabWidget->setTabText(0, obs_module_text("DialogGeneralTabName"));
	m_ui->settingsTabWidget->setTabText(1, obs_module_text("DialogTrackingTabName"));

	m_ui->destIpAddressLabel->setText(obs_module_text("DialogDestIpAddressLabel"));
	m_ui->destIpAddressLineEdit->setValidator(new QRegularExpressionValidator(
		QRegularExpression(R"(^((25[0-5]|2[0-4]\d|[01]?\d?\d)\.){3}(25[0-5]|2[0-4]\d|[01]?\d?\d)$)"), this));

	m_ui->portLabel->setText(obs_module_text("DialogPortAddress"));
	m_ui->portSpinBox->setRange(0, 65535);

	m_ui->ipAddressLabel->setText(obs_module_text("DialogIpAddressLabel"));
	m_ui->ipAddressViewLabel->setText(NetworkTracking::getIpAddresses());

	m_ui->destPortLabel->setText(obs_module_text("DialogDestPortAddressLabel"));
	m_ui->destPortSpinBox->setRange(0, 65535);

	m_ui->byLabel->setText(obs_module_text("DialogInfoByLabel"));
	m_ui->contributorsLabel->setText(obs_module_text("DialogInfoConstributorsLabel"));
	m_ui->versionLabel->setText(obs_module_text("DialogInfoVersionLabel"));
	m_ui->versionTextLabel->setText(PLUGIN_VERSION);

	m_ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	m_ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setText(obs_module_text("DialogButtonApplyLabel"));
	m_ui->dialogButtonBox->button(QDialogButtonBox::Ok)->setText(obs_module_text("DialogButtonOkLabel"));
	m_ui->dialogButtonBox->button(QDialogButtonBox::Cancel)->setText(obs_module_text("DialogButtonCancelLabel"));

	// ----------------------------------------------- Tracking Tab -----------------------------------------------

	m_ui->poseVerticalLayout->addWidget(m_poseListWidget);
	m_ui->poseVerticalLayout->addWidget(m_imageFilesWidget);
	m_ui->settingsVerticalLayout->addWidget(m_faceSettingsWidget);
	m_ui->settingsVerticalLayout->addWidget(m_blendshapeRulesWidget);

	m_ui->centerOnImagesToolButton->setToolTip(obs_module_text("DialogCenterViewOnImagesToolTip"));
	m_ui->moveImageUpLevelToolButton->setToolTip(obs_module_text("DialogMoveImageUpLevelToolTip"));
	m_ui->moveImageDownLevelToolButton->setToolTip(obs_module_text("DialogMoveImageDownLevelToolTip"));
	m_ui->centerOnImagesToolButton->setToolTip(obs_module_text("DialogCenterViewOnImagesToolTip"));
	m_ui->zoomOutToolButton->setToolTip(obs_module_text("DialogZoomOutToolTip"));
	m_ui->zoomInToolButton->setToolTip(obs_module_text("DialogZoomInToolTip"));

	updateStyledUIComponents();

	getOBSSourceList();

	// Set form based on tracker data
	setFormDetails(settingsDialogData);
}

void SettingsDialog::applyFormChanges()
{
	m_isError = false;
	if (m_trackerData != nullptr) {
		QVector<Result> validationResults;

		validationResults.append(validateTrackerID());
		validationResults.append(validateDestIPAddress());

		QString errorMessages = "";

		for (const Result &res : validationResults) {
			if (!res.success) {
				m_isError = true;
				errorMessages += "-" + res.errorMessage + "\n\n";
			}
		}

		if (m_isError) {
			QMessageBox::warning(this, obs_module_text("DialogErrorPopupTitle"), errorMessages);
			return;
		}

		QSharedPointer<TrackerData> newData = QSharedPointer<TrackerData>::create();

		newData->setTrackerId(m_ui->trackerIdLineEdit->text());
		newData->setSelectedImageSource(m_ui->imageSourceDropdownList->currentText());
		newData->setPort(m_ui->portSpinBox->text().toInt());
		newData->setDestinationIpAddress(m_ui->destIpAddressLineEdit->text());
		newData->setDestinationPort(m_ui->destPortSpinBox->text().toInt());

		newData->copyListToPoseList(m_settingsPoseList);

		m_ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
		emit settingsUpdated(newData);
	} else {
		obs_log(LOG_WARNING, "No tracker data found!");
	}
}

Result SettingsDialog::validateTrackerID()
{
	Result updateIdResult = {true, ""};
	QLineEdit *idLineEdit = m_ui->trackerIdLineEdit;
	QString setTrackerId = idLineEdit->text();
	if ((setTrackerId != m_trackerData->getTrackerId() && m_mainWidget)) {
		updateIdResult = m_mainWidget->validateNewTrackerID(setTrackerId);
		if (updateIdResult.success == true) {
			idLineEdit->setStyleSheet("");
			setTitle();
		} else {
			idLineEdit->setStyleSheet(m_formErrorStyling);
			obs_log(LOG_WARNING, updateIdResult.errorMessage.toStdString().c_str());
		}
	}
	return updateIdResult;
}

Result SettingsDialog::validateDestIPAddress()
{
	Result destinationIdValidation = {true, ""};
	QLineEdit *lineEdit = m_ui->destIpAddressLineEdit;

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
		lineEdit->setStyleSheet(m_formErrorStyling);
		destinationIdValidation = {false, obs_module_text("DialogTrackerDestIpValidationError")};
		obs_log(LOG_WARNING, destinationIdValidation.errorMessage.toStdString().c_str());
	}

	return destinationIdValidation;
}

void SettingsDialog::getOBSSourceList()
{
	// Get All Image Sources
	obs_enum_sources(getImageSources, m_ui->imageSourceDropdownList);
}

void SettingsDialog::obsSourceCreated(void *param, calldata_t *calldata)
{
	auto m_ui = static_cast<Ui::FaceTrackerDialog *>(param);
	obs_source_t *source;
	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	bool isImageSourceType = checkIfImageSourceType(source);
	// If not sourceType we need;
	if (!isImageSourceType)
		return;

	const char *name = obs_source_get_name(source);
	m_ui->imageSourceDropdownList->addItem(name);
};

void SettingsDialog::obsSourceDeleted(void *param, calldata_t *calldata)
{
	auto m_ui = static_cast<Ui::FaceTrackerDialog *>(param);

	obs_source_t *source;

	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	bool isImageSourceType = checkIfImageSourceType(source);
	// If not sourceType we need;
	if (!isImageSourceType)
		return;

	const char *name = obs_source_get_name(source);

	int textIndexToRemove = m_ui->imageSourceDropdownList->findText(name);
	m_ui->imageSourceDropdownList->removeItem(textIndexToRemove);
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
	auto m_ui = static_cast<Ui::FaceTrackerDialog *>(param);

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

	int textListIndex = m_ui->imageSourceDropdownList->findText(oldName);
	if (textListIndex == -1)
		return;
	m_ui->imageSourceDropdownList->setItemText(textListIndex, newName);
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
			 &SettingsDialog::handleImageMove);

	centerSceneOnItems();
}

void SettingsDialog::clearScene()
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

void SettingsDialog::clearCurrentPoseConfig()
{
	m_imageFilesWidget->clearSelection();
	m_faceSettingsWidget->clearSelection();
	m_blendshapeRulesWidget->clearAll();
	clearScene();
}

void SettingsDialog::loadSelectedPoseConfig()
{
	int selectedRow = getSelectedRow();
	if (selectedRow == -1)
		return;

	if (selectedRow == m_previouslySelectedPoseIndex)
		return;

	m_previouslySelectedPoseIndex = selectedRow;

	QSharedPointer<Pose> selectedPose = m_settingsPoseList[selectedRow];
	clearCurrentPoseConfig();
	clearScene();
	m_imageFilesWidget->toggleVisible(true);
	m_faceSettingsWidget->setData(selectedPose);
	m_faceSettingsWidget->toggleVisible(true);
	m_blendshapeRulesWidget->setData(selectedPose);
	m_blendshapeRulesWidget->toggleVisible(true);

	for (size_t i = 0; i < static_cast<size_t>(PoseImage::COUNT); ++i) {
		auto poseEnum = static_cast<PoseImage>(i);
		QMap<PoseImage, QLineEdit *> poseImageLineEdits = m_imageFilesWidget->getposeLineEditsMap();
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
	return m_poseListWidget->getSelectedRow();
}

void SettingsDialog::resetPoseUITab()
{
	m_previouslySelectedPoseIndex = -1;
	clearCurrentPoseConfig();
	m_poseListWidget->clearSelection();
	m_imageFilesWidget->toggleVisible(false);
	m_faceSettingsWidget->toggleVisible(false);
	m_blendshapeRulesWidget->toggleVisible(false);
}

void SettingsDialog::centerSceneOnItems()
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

//  ------------------------------------------------ Protected ------------------------------------------------

void SettingsDialog::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);

	// Reset stylings
	m_ui->trackerIdLineEdit->setStyleSheet("");
	m_ui->destIpAddressLineEdit->setStyleSheet("");

	m_ui->ipAddressViewLabel->setText(NetworkTracking::getIpAddresses());

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
	m_ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void SettingsDialog::applyButtonClicked()
{
	applyFormChanges();
}

void SettingsDialog::cancelButtonClicked()
{
	setFormDetails(m_trackerData);
	resetPoseUITab();
	this->reject();
}

void SettingsDialog::okButtonClicked()
{
	applyFormChanges();
	resetPoseUITab();
	if (!m_isError)
		this->reject();
}

void SettingsDialog::syncPoseListToModel()
{
	// Disconnect setting row update signal during sync with tracker data
	QObject::disconnect(m_poseListWidget, &PoseListWidget::rowsInserted, this, &SettingsDialog::onPoseRowsInserted);

	m_poseListWidget->clearList();

	m_settingsPoseList.clear();
	m_previouslySelectedPoseIndex = -1;

	for (int i = 0; i < m_trackerData->getPoseList().size(); ++i) {
		QSharedPointer<Pose> originalPosePtr = m_trackerData->getPoseList()[i];
		if (!originalPosePtr) {
			obs_log(LOG_WARNING, QString("Pose pointer at index %1 is null.").arg(i).toStdString().c_str());
			continue; // Skip null pointers
		}

		// Clone the Pose to ensure an independent copy
		QSharedPointer<Pose> clonedPosePtr = originalPosePtr->clone();
		m_settingsPoseList.append(clonedPosePtr);

		m_poseListWidget->addRow(clonedPosePtr->getPoseId());
	}
	// Reenable signal
	QObject::connect(m_poseListWidget, &PoseListWidget::rowsInserted, this, &SettingsDialog::onPoseRowsInserted);
}

void SettingsDialog::handleSetImageUrl(PoseImage poseEnum, QString fileName)
{
	int selectedRow = getSelectedRow();
	if (selectedRow == -1)
		return;

	QSharedPointer<Pose> selectedPose = m_settingsPoseList[selectedRow];

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

	QSharedPointer<Pose> selectedPose = m_settingsPoseList[selectedRow];
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
	if (m_isMovingPoseListRows)
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
		if (row < 0 || row > m_settingsPoseList.size()) {
			obs_log(LOG_WARNING, "Invalid pose at row: %d", row);
			continue;
		}
		obs_log(LOG_INFO, "Adding row: %d", row);
		QSharedPointer<Pose> newPose = QSharedPointer<Pose>::create();
		newPose->setPoseId(rowMap.value(row));
		m_settingsPoseList.insert(row, newPose);
	}

	loadSelectedPoseConfig();
	formChangeDetected();
}

void SettingsDialog::onPoseRowsRemoved(QList<int> rowsList)
{
	if (m_isMovingPoseListRows)
		return;

	// Sort from greatest index to lowest to ensure no issues
	// deleting indices that dont already exist
	QList<int> sortedRows = rowsList;
	std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());

	for (const int &row : sortedRows) {
		if (row >= 0 && row < m_settingsPoseList.size()) {
			m_settingsPoseList.removeAt(row);
			obs_log(LOG_INFO, "Pose removed: %d", row);
		} else {
			obs_log(LOG_WARNING, "Invalid row index: %d", row);
		}
	}
	m_previouslySelectedPoseIndex = -1;
	loadSelectedPoseConfig();
	formChangeDetected();
}

void SettingsDialog::onPoseDataChanged(QMap<int, QString> rowMap)
{
	for (auto i = rowMap.cbegin(), end = rowMap.cend(); i != end; i++) {
		int row = i.key();
		if (row >= 0 && row < m_settingsPoseList.size()) {
			obs_log(LOG_INFO, "Pose data changed: %d", row);
			m_settingsPoseList[row]->setPoseId(i.value());
		}
	}

	formChangeDetected();
}

void SettingsDialog::onPoseRowMoved(int sourceRow, int targetRow)
{
	obs_log(LOG_INFO, "Swapping from row: %d to row: %d", sourceRow, targetRow);

	obs_log(LOG_INFO, "[SettingsDialog] Before swapping:");
	for (int i = 0; i < m_settingsPoseList.size(); ++i) {
		obs_log(LOG_INFO, "Row %d : %s", i, m_settingsPoseList[i]->getPoseId().toStdString().c_str());
	}

	m_settingsPoseList.swapItemsAt(sourceRow, targetRow);
	m_previouslySelectedPoseIndex = targetRow;

	obs_log(LOG_INFO, "[SettingsDialog] After swapping:");
	for (int i = 0; i < m_settingsPoseList.size(); ++i) {
		obs_log(LOG_INFO, "Row %d : %s", i, m_settingsPoseList[i]->getPoseId().toStdString().c_str());
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

void SettingsDialog::handleMoveImageDownClick()
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

void SettingsDialog::handleImageZoomClick(bool isZoomOut)
{
	double zoomScaleFactor = 1.15;

	if (isZoomOut) {
		m_ui->avatarGraphicsView->scale(1.0 / zoomScaleFactor, 1.0 / zoomScaleFactor);
	} else {
		m_ui->avatarGraphicsView->scale(zoomScaleFactor, zoomScaleFactor);
	}
}

void SettingsDialog::handleImageMove(qreal x, qreal y, qreal z, PoseImage pImageType)
{
	int selectedRow = getSelectedRow();

	obs_log(LOG_INFO, QString("Updated Pose %1: x=%2, y=%3, z=%4").arg(x).arg(y).arg(z).toStdString().c_str());

	if (selectedRow != -1) {
		QSharedPointer<Pose> selectedPose = m_settingsPoseList[selectedRow];
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

void SettingsDialog::handleBlendshapelimitChange(PoseImage poseEnum, double value)
{
	obs_log(LOG_INFO, "Pose: %s, value: %f", poseImageToString(poseEnum).toStdString().c_str(), value);

	int selectedRow = getSelectedRow();
	if (selectedRow == -1)
		return;

	QSharedPointer<Pose> selectedPose = m_settingsPoseList[selectedRow];

	switch (poseEnum) {
	case PoseImage::EYESHALFOPEN:
		selectedPose->setEyesHalfOpenLimit(value);
		formChangeDetected();
		break;
	case PoseImage::EYESOPEN:
		selectedPose->setEyesOpenLimit(value);
		formChangeDetected();
		break;
	case PoseImage::MOUTHOPEN:
		selectedPose->setMouthOpenLimit(value);
		formChangeDetected();
		break;
	case PoseImage::TONGUEOUT:
		selectedPose->setTongueOutLimit(value);
		formChangeDetected();
		break;

	default:
		break;
	}
}
