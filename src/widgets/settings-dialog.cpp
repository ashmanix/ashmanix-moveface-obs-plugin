#include "settings-dialog.hpp"

SettingsDialog::SettingsDialog(QWidget *parent, QSharedPointer<TrackerData> tData, MainWidgetDock *mWidget)
	: QDialog(parent),
	  ui(new Ui::FaceTrackerDialog)
{
	ui->setupUi(this);
	trackerData = tData;
	mainWidget = mWidget;
	setTitle();

	setupDialogUI(trackerData);

	connectUISignalHandlers();
	connectObsSignalHandlers();
}

SettingsDialog::~SettingsDialog()
{
	// Disconnect OBS signals before the dialog is destroyed
	signal_handler_disconnect(obs_get_signal_handler(), "source_create", obsSourceCreated, ui);
	signal_handler_disconnect(obs_get_signal_handler(), "source_destroy", obsSourceDeleted, ui);
	signal_handler_disconnect(obs_get_signal_handler(), "source_rename", obsSourceRenamed, ui);

	delete ui;
	settingsPoseList.clear();

	this->deleteLater();
}

void SettingsDialog::updateStyledUIComponents()
{
	QString baseUrl = obs_frontend_is_theme_dark() ? GetDataFolderPath() + "/icons/dark/"
						       : GetDataFolderPath() + "/icons/light/";
	QString searchIconPath = QDir::fromNativeSeparators(baseUrl + "search.svg");
	if (QFileInfo::exists(searchIconPath)) {
		QIcon searchIcon(searchIconPath);

		ui->bodyUrlBrowseToolButton->setIcon(searchIcon);
		ui->eyesOpenUrlBrowseToolButton->setIcon(searchIcon);
		ui->eyesHalfOpenUrlBrowseToolButton->setIcon(searchIcon);
		ui->eyesClosedUrlBrowseToolButton->setIcon(searchIcon);
		ui->mouthClosedUrlBrowseToolButton->setIcon(searchIcon);
		ui->mouthOpenUrlBrowseToolButton->setIcon(searchIcon);
		ui->mouthSmileUrlBrowseToolButton->setIcon(searchIcon);
		ui->tongueOutUrlBrowseToolButton->setIcon(searchIcon);
	}

	QString plusIconPath = QDir::fromNativeSeparators(baseUrl + "plus.svg");
	if (QFileInfo::exists(plusIconPath)) {
		QIcon plusIcon(plusIconPath);

		ui->addPoseToolButton->setIcon(plusIcon);
		ui->zoomInToolButton->setIcon(plusIcon);
	}

	QString minusIconPath = QDir::fromNativeSeparators(baseUrl + "minus.svg");
	if (QFileInfo::exists(minusIconPath)) {
		QIcon minusIcon(minusIconPath);

		ui->zoomOutToolButton->setIcon(minusIcon);
	}

	QString trashIconPath = QDir::fromNativeSeparators(baseUrl + "trash.svg");
	if (QFileInfo::exists(trashIconPath)) {
		QIcon trashIcon(trashIconPath);

		ui->deletePoseToolButton->setIcon(trashIcon);
	}

	QString entryClearIconPath = QDir::fromNativeSeparators(baseUrl + "entry-clear.svg");
	if (QFileInfo::exists(entryClearIconPath)) {
		QIcon entryClearIcon(entryClearIconPath);

		ui->bodyUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->eyesOpenUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->eyesHalfOpenUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->eyesClosedUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->mouthClosedUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->mouthOpenUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->mouthSmileUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->tongueOutUrlDeleteToolButton->setIcon(entryClearIcon);
	}

	QString upIconPath = QDir::fromNativeSeparators(baseUrl + "up.svg");
	if (QFileInfo::exists(upIconPath)) {
		QIcon upIcon(upIconPath);
		ui->moveImageUpLevelToolButton->setIcon(upIcon);
		ui->movePoseUpToolButton->setIcon(upIcon);
	}

	QString downIconPath = QDir::fromNativeSeparators(baseUrl + "down.svg");
	if (QFileInfo::exists(downIconPath)) {
		QIcon downIcon(downIconPath);
		ui->moveImageDownLevelToolButton->setIcon(downIcon);
		ui->movePoseDownToolButton->setIcon(downIcon);
	}

	QString centerIconPath = QDir::fromNativeSeparators(baseUrl + "center.svg");
	if (QFileInfo::exists(centerIconPath)) {
		QIcon centerIcon(centerIconPath);
		ui->centerOnImagesToolButton->setIcon(centerIcon);
	}

	ui->trackingDialogScrollArea->setStyleSheet("QScrollArea {"
						    "background-color: transparent;"
						    "}"
						    "#trackingScrollAreaWidgetContents {"
						    "background-color: transparent;"
						    "}");
	ui->noConfigLabel->setStyleSheet("font-size: 20pt; padding-bottom: 40px;");
	ui->noConfigLabel->setText(obs_module_text("DialogNoConfigMessage"));
	ui->avatarGraphicsView->setStyleSheet("background-color: rgb(0, 0, 0);");
	ui->poseListView->setStyleSheet("QListView::item {"
					"   padding-top: 5px;"
					"   padding-bottom: 5px;"
					"}");
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

	QObject::connect(ui->addPoseToolButton, &QToolButton::clicked, this, &SettingsDialog::addPose);
	QObject::connect(ui->deletePoseToolButton, &QToolButton::clicked, this, &SettingsDialog::deletePose);
	QObject::connect(ui->movePoseUpToolButton, &QToolButton::clicked, this, [this]() { handleMovePose(true); });
	QObject::connect(ui->movePoseDownToolButton, &QToolButton::clicked, this, [this]() { handleMovePose(); });

	QObject::connect(ui->dialogButtonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::okButtonClicked);

	QObject::connect(ui->dialogButtonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::cancelButtonClicked);

	QObject::connect(ui->bodyUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::BODY); });
	QObject::connect(ui->eyesOpenUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::EYESOPEN); });
	QObject::connect(ui->eyesHalfOpenUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::EYESHALFOPEN); });
	QObject::connect(ui->eyesClosedUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::EYESCLOSED); });
	QObject::connect(ui->mouthClosedUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::MOUTHCLOSED); });
	QObject::connect(ui->mouthOpenUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::MOUTHOPEN); });
	QObject::connect(ui->mouthSmileUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::MOUTHSMILE); });
	QObject::connect(ui->tongueOutUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::TONGUEOUT); });

	QObject::connect(ui->bodyUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrl(PoseImage::BODY); });
	QObject::connect(ui->eyesOpenUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrl(PoseImage::EYESOPEN); });
	QObject::connect(ui->eyesHalfOpenUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrl(PoseImage::EYESHALFOPEN); });
	QObject::connect(ui->eyesClosedUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrl(PoseImage::EYESCLOSED); });
	QObject::connect(ui->mouthClosedUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrl(PoseImage::MOUTHCLOSED); });
	QObject::connect(ui->mouthOpenUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrl(PoseImage::MOUTHOPEN); });
	QObject::connect(ui->mouthSmileUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrl(PoseImage::MOUTHSMILE); });
	QObject::connect(ui->tongueOutUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrl(PoseImage::TONGUEOUT); });

	QObject::connect(ui->poseListView, &QListView::clicked, this, &SettingsDialog::handlePoseListClick);

	QObject::connect(ui->centerOnImagesToolButton, &QToolButton::clicked, this,
			 &SettingsDialog::handleCenterViewButtonClick);
	QObject::connect(ui->moveImageUpLevelToolButton, &QToolButton::clicked, this,
			 &SettingsDialog::handleMoveImageUpClick);
	QObject::connect(ui->moveImageDownLevelToolButton, &QToolButton::clicked, this,
			 &SettingsDialog::handleMoveImageDownClick);

	QObject::connect(ui->zoomInToolButton, &QToolButton::clicked, this, [this]() { handleImageZoomClick(false); });
	QObject::connect(ui->zoomOutToolButton, &QToolButton::clicked, this, [this]() { handleImageZoomClick(true); });

	QObject::connect(poseListModel, &QAbstractItemModel::rowsInserted, this, &SettingsDialog::onPoseRowsInserted);
	QObject::connect(poseListModel, &QAbstractItemModel::rowsRemoved, this, &SettingsDialog::onPoseRowsRemoved);
	QObject::connect(poseListModel, &QAbstractItemModel::dataChanged, this, &SettingsDialog::onPoseDataChanged);

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

	ui->imageGroupBox->setTitle(obs_module_text("DialogAvatarGroupBoxTitle"));

	ui->addPoseToolButton->setToolTip(obs_module_text("DialogAddPoseToolTip"));
	ui->deletePoseToolButton->setToolTip(obs_module_text("DialogDeletePoseToolTip"));
	ui->movePoseUpToolButton->setToolTip(obs_module_text("DialogMovePoseUpToolTip"));
	ui->movePoseDownToolButton->setToolTip(obs_module_text("DialogMovePoseDownToolTip"));

	ui->poseListLabel->setText(obs_module_text("DialogPostListLabel"));
	ui->poseImageLabel->setText(obs_module_text("DialogPoseImageLabel"));
	poseListModel = new QStandardItemModel(this);
	ui->poseListView->setModel(poseListModel);
	ui->poseListView->setDragEnabled(false);
	ui->poseListView->setAcceptDrops(false);
	ui->poseListView->setDragDropOverwriteMode(false);

	ui->bodyUrlLabel->setText(obs_module_text("DialogBodyUrlLabel"));
	ui->eyesOpenUrlLabel->setText(obs_module_text("DialogEyesOpenUrlLabel"));
	ui->eyesHalfOpenUrLabel->setText(obs_module_text("DialogEyesHalfOpenUrlLabel"));
	ui->eyesClosedUrlLabel->setText(obs_module_text("DialogEyesClosedUrlLabel"));
	ui->mouthClosedUrlLabel->setText(obs_module_text("DialogMouthClosedUrlLabel"));
	ui->mouthOpenUrlLabel->setText(obs_module_text("DialogMouthOpenUrlLabel"));
	ui->mouthSmileUrlLabel->setText(obs_module_text("DialogMouthSmileUrlLabel"));
	ui->tongueOutUrlLabel->setText(obs_module_text("DialogTongueOutUrlLabel"));

	ui->bodyUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->eyesOpenUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->eyesHalfOpenUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->eyesClosedUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->mouthClosedUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->mouthOpenUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->mouthSmileUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->tongueOutUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));

	ui->bodyUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->eyesOpenUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->eyesHalfOpenUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->eyesClosedUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->mouthClosedUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->mouthOpenUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->mouthSmileUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->tongueOutUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));

	ui->centerOnImagesToolButton->setToolTip(obs_module_text("DialogCenterViewOnImagesToolTip"));
	ui->moveImageUpLevelToolButton->setToolTip(obs_module_text("DialogMoveImageUpLevelToolTip"));
	ui->moveImageDownLevelToolButton->setToolTip(obs_module_text("DialogMoveImageDownLevelToolTip"));
	ui->centerOnImagesToolButton->setToolTip(obs_module_text("DialogCenterViewOnImagesToolTip"));
	ui->zoomOutToolButton->setToolTip(obs_module_text("DialogZoomOutToolTip"));
	ui->zoomInToolButton->setToolTip(obs_module_text("DialogZoomInToolTip"));

	// Initialize the mapping between PoseImage enums and QLineEdit pointers
	poseImageLineEdits[PoseImage::BODY] = ui->bodyUrlLineEdit;
	poseImageLineEdits[PoseImage::EYESOPEN] = ui->eyesOpenUrlLineEdit;
	poseImageLineEdits[PoseImage::EYESHALFOPEN] = ui->eyesHalfOpenUrlEdit;
	poseImageLineEdits[PoseImage::EYESCLOSED] = ui->eyesClosedUrlLineEdit;
	poseImageLineEdits[PoseImage::MOUTHCLOSED] = ui->mouthClosedUrlLineEdit;
	poseImageLineEdits[PoseImage::MOUTHOPEN] = ui->mouthOpenUrlLineEdit;
	poseImageLineEdits[PoseImage::MOUTHSMILE] = ui->mouthSmileUrlLineEdit;
	poseImageLineEdits[PoseImage::TONGUEOUT] = ui->tongueOutUrlLineEdit;

	ui->imageConfigWidget->setVisible(false);
	ui->noConfigLabel->setVisible(true);

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

		// newData->poseList.clear();
		// for (const QSharedPointer<Pose> &posePtr : settingsPoseList) {
		// 	if (posePtr) {
		// 		newData->poseList.append(posePtr->clone());
		// 	}
		// }

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

		// Create a pen for the boundary rectangle
		QPen boundaryPen(Qt::red);
		boundaryPen.setWidth(3);

		// Create a rectangle to show the scene area
		QGraphicsRectItem *boundaryRect = new QGraphicsRectItem(avatarPreviewScene->sceneRect());
		boundaryRect->setPen(boundaryPen);
		boundaryRect->setBrush(Qt::NoBrush); // No fill color
		boundaryRect->setZValue(1000);

		avatarPreviewScene->addItem(boundaryRect);
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

		QGraphicsPixmapItem *imagePixmap =
			static_cast<QGraphicsPixmapItem *>(imageData->getPixmapItem().data());

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
	for (size_t i = 0; i < static_cast<size_t>(PoseImage::COUNT); ++i) {
		PoseImage poseEnum = static_cast<PoseImage>(i);
		auto it = poseImageLineEdits.find(poseEnum);
		if (it != poseImageLineEdits.end()) {
			QLineEdit *lineEdit = it.value(); // For QMap
			// If using std::map, use it->second
			if (lineEdit) {
				lineEdit->clear();
			}
		}
	}
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

	for (size_t i = 0; i < static_cast<size_t>(PoseImage::COUNT); ++i) {
		PoseImage poseEnum = static_cast<PoseImage>(i);
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
	QModelIndex modelIndex = ui->poseListView->currentIndex();
	return modelIndex.row();
}

void SettingsDialog::resetPoseUITab()
{
	ui->poseListView->selectionModel()->clearSelection();
	previouslySelectedPoseIndex = -1;
	clearCurrentPoseConfig();
	ui->imageConfigWidget->setVisible(false);
	ui->noConfigLabel->setVisible(true);
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
	QObject::disconnect(poseListModel, &QAbstractItemModel::rowsInserted, this,
			    &SettingsDialog::onPoseRowsInserted);

	poseListModel->clear();

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

		// const Pose &p = *clonedPosePtr;
		QStandardItem *item = new QStandardItem(clonedPosePtr->getPoseId());

		poseListModel->appendRow(item);
	}
	// Reenable signal
	QObject::connect(poseListModel, &QAbstractItemModel::rowsInserted, this, &SettingsDialog::onPoseRowsInserted);
}

void SettingsDialog::addPose()
{
	if (!poseListModel)
		return obs_log(LOG_ERROR, "No list model found!");

	int count = poseListModel->rowCount();
	QStandardItem *poseItem = new QStandardItem(QString("New Pose %1").arg(count));
	poseListModel->appendRow(poseItem);
}

void SettingsDialog::deletePose()
{
	QModelIndex modelIndex = ui->poseListView->currentIndex();
	int rowIndex = modelIndex.row();

	obs_log(LOG_INFO, "Row To Delete: %d", rowIndex);

	if (rowIndex != -1) {
		poseListModel->takeRow(rowIndex);
	}
}

void SettingsDialog::handleMovePose(bool isDirectionUp)
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

	settingsPoseList.swapItemsAt(selectedRow, targetRow);

	ui->poseListView->setCurrentIndex(poseListModel->index(targetRow, 0));
	previouslySelectedPoseIndex = targetRow;
	loadSelectedPoseConfig();
	formChangeDetected();
}

void SettingsDialog::handleImageUrlButtonClicked(PoseImage poseEnum)
{
	QString fileName = QFileDialog::getOpenFileName(this, obs_module_text("ImageUrlFileDialogWindowTitle"),
							QString("%HOME%/Images"),
							tr("Images (*.png *.jpg *.bmp *.jpeg)"));

	this->raise();
	this->activateWindow();

	if (fileName.isEmpty()) {
		return;
	}

	int selectedRow = getSelectedRow();
	if (selectedRow == -1)
		return;

	QSharedPointer<Pose> selectedPose = settingsPoseList[selectedRow];

	int imageIndex = static_cast<int>(poseEnum);

	if (!(imageIndex >= 0 && static_cast<size_t>(imageIndex) < selectedPose->getPoseImageListSize())) {
		obs_log(LOG_WARNING, "Invalid PoseImage enum value.");
		return;
	}

	formChangeDetected();

	if (!FileExists(fileName)) {
		obs_log(LOG_WARNING, QString("Image file: %1 not found!").arg(fileName).toStdString().c_str());
		return;
	}

	QLineEdit *lineEdit = poseImageLineEdits.value(poseEnum, nullptr);
	if (lineEdit) {
		lineEdit->setText(fileName);
	} else {
		obs_log(LOG_WARNING, QString("QLineEdit not found for PoseImage enum value: %1")
					     .arg(static_cast<int>(poseEnum))
					     .toStdString()
					     .c_str());
		return;
	}

	selectedPose->getPoseImageAt(imageIndex)->getImageUrl() = fileName;

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

void SettingsDialog::handleClearImageUrl(PoseImage poseEnum)
{
	int selectedRow = getSelectedRow();
	if (selectedRow == -1)
		return;

	formChangeDetected();

	QSharedPointer<Pose> selectedPose = settingsPoseList[selectedRow];
	int imageIndex = static_cast<int>(poseEnum);

	if (imageIndex < 0)
		return;

	if (imageIndex >= 0 && static_cast<size_t>(imageIndex) < selectedPose->getPoseImageListSize()) {
		PoseImageData *poseData = selectedPose->getPoseImageAt(imageIndex);
		poseImageLineEdits[poseEnum]->setText(QString());

		poseData->setImageUrl(QString());
		if (poseData->getPixmapItem()) {
			poseData->clearPixmapItem();
		}
	} else {
		obs_log(LOG_WARNING, "Invalid PoseImage enum value.");
	}
}

void SettingsDialog::handlePoseListClick(QModelIndex modelIndex)
{
	obs_log(LOG_INFO, "Row: %d", modelIndex.row());
	loadSelectedPoseConfig();
	ui->imageConfigWidget->setVisible(true);
	ui->noConfigLabel->setVisible(false);
}

void SettingsDialog::onPoseRowsInserted(const QModelIndex &parent, int first, int last)
{
	Q_UNUSED(parent);

	if (isMovingPoseListRows)
		return;

	// For each newly inserted row, create a default Pose or read from the model
	// and insert into settingsPoseList at the correct position.
	for (int row = first; row <= last; ++row) {
		// E.g. read the poseId from the model
		QModelIndex idx = poseListModel->index(row, 0);
		QString poseId = idx.data(Qt::DisplayRole).toString();

		QSharedPointer<Pose> newPose = QSharedPointer<Pose>::create();
		newPose->setPoseId(poseId);

		settingsPoseList.insert(row, newPose);
	}
	loadSelectedPoseConfig();
	formChangeDetected();
}

void SettingsDialog::onPoseRowsRemoved(const QModelIndex &parent, int first, int last)
{
	Q_UNUSED(parent);

	if (isMovingPoseListRows)
		return;

	// Remove the corresponding poses from settingsPoseList
	for (int row = last; row >= first; --row) {
		if (row >= 0 && row < settingsPoseList.size()) {
			settingsPoseList.removeAt(row);
		}
	}
	loadSelectedPoseConfig();
	formChangeDetected();
}

void SettingsDialog::onPoseDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
				       const QList<int> &roles)
{
	Q_UNUSED(&roles);
	for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
		// Read changed data from the model and update the Pose
		QModelIndex idx = poseListModel->index(row, 0);
		QString newPoseId = idx.data(Qt::DisplayRole).toString();
		if (row >= 0 && row < settingsPoseList.size()) {
			settingsPoseList[row]->setPoseId(newPoseId);
		}
	}
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
