#include "settings-dialog.hpp"

SettingsDialog::SettingsDialog(QWidget *parent, QSharedPointer<TrackerDataStruct> tData, MainWidgetDock *mWidget)
	: QDialog(parent),
	  ui(new Ui::FaceTrackerDialog)
{
	ui->setupUi(this);
	trackerData = tData;
	mainWidget = mWidget;
	SetTitle();

	SetupDialogUI(trackerData);

	ConnectUISignalHandlers();
	ConnectObsSignalHandlers();
}

SettingsDialog::~SettingsDialog()
{
	// Disconnect OBS signals before the dialog is destroyed
	signal_handler_disconnect(obs_get_signal_handler(), "source_create", OBSSourceCreated, ui);
	signal_handler_disconnect(obs_get_signal_handler(), "source_destroy", OBSSourceDeleted, ui);
	signal_handler_disconnect(obs_get_signal_handler(), "source_rename", OBSSourceRenamed, ui);

	delete ui;
	settingsPoseList.clear();

	this->deleteLater();
}

void SettingsDialog::UpdateStyledUIComponents()
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

	ui->noConfigLabel->setStyleSheet("font-size: 20pt; padding-bottom: 40px;");
	ui->noConfigLabel->setText(obs_module_text("DialogNoConfigMessage"));
	ui->avatarGraphicsView->setStyleSheet("background-color: rgb(0, 0, 0);");
	ui->poseListView->setStyleSheet("QListView::item {"
					"   padding-top: 5px;"
					"   padding-bottom: 5px;"
					"}");
}

void SettingsDialog::SetFormDetails(QSharedPointer<TrackerDataStruct> settingsDialogData)
{
	ui->ipAddressViewLabel->setText(NetworkTracking::GetIpAddresses());

	if (settingsDialogData != nullptr) {
		ui->trackerIdLineEdit->setText(settingsDialogData->trackerId);

		int selectedImageSource =
			ui->imageSourceDropdownList->findText(settingsDialogData->selectedImageSource);
		if (selectedImageSource != -1) {
			ui->imageSourceDropdownList->setCurrentIndex(selectedImageSource);
		} else {
			ui->imageSourceDropdownList->setCurrentIndex(0);
		}

		ui->portSpinBox->setValue(settingsDialogData->port);
		ui->destIpAddressLineEdit->setText(settingsDialogData->destIpAddress);
		ui->destPortSpinBox->setValue(settingsDialogData->destPort);

		ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

		SyncPoseListToModel();
	} else {
		obs_log(LOG_WARNING, "No tracker data found!");
	}
}

void SettingsDialog::ConnectUISignalHandlers()
{
	QObject::connect(ui->trackerIdLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->imageSourceDropdownList, &QComboBox::currentTextChanged, this,
			 &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->destIpAddressLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->destPortSpinBox, &QSpinBox::valueChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->portSpinBox, &QSpinBox::valueChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->addPoseToolButton, &QToolButton::clicked, this, &SettingsDialog::AddPose);
	QObject::connect(ui->deletePoseToolButton, &QToolButton::clicked, this, &SettingsDialog::DeletePose);
	QObject::connect(ui->movePoseUpToolButton, &QToolButton::clicked, this, [this]() { HandleMovePose(true); });
	QObject::connect(ui->movePoseDownToolButton, &QToolButton::clicked, this, [this]() { HandleMovePose(); });

	QObject::connect(ui->dialogButtonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::OkButtonClicked);

	QObject::connect(ui->dialogButtonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::CancelButtonClicked);

	QObject::connect(ui->bodyUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::BODY); });
	QObject::connect(ui->eyesOpenUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::EYESOPEN); });
	QObject::connect(ui->eyesHalfOpenUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::EYESHALFOPEN); });
	QObject::connect(ui->eyesClosedUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::EYESCLOSED); });
	QObject::connect(ui->mouthClosedUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::MOUTHCLOSED); });
	QObject::connect(ui->mouthOpenUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::MOUTHOPEN); });
	QObject::connect(ui->mouthSmileUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::MOUTHSMILE); });
	QObject::connect(ui->tongueOutUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::TONGUEOUT); });

	QObject::connect(ui->bodyUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { HandleClearImageUrl(PoseImage::BODY); });
	QObject::connect(ui->eyesOpenUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { HandleClearImageUrl(PoseImage::EYESOPEN); });
	QObject::connect(ui->eyesHalfOpenUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { HandleClearImageUrl(PoseImage::EYESHALFOPEN); });
	QObject::connect(ui->eyesClosedUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { HandleClearImageUrl(PoseImage::EYESCLOSED); });
	QObject::connect(ui->mouthClosedUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { HandleClearImageUrl(PoseImage::MOUTHCLOSED); });
	QObject::connect(ui->mouthOpenUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { HandleClearImageUrl(PoseImage::MOUTHOPEN); });
	QObject::connect(ui->mouthSmileUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { HandleClearImageUrl(PoseImage::MOUTHSMILE); });
	QObject::connect(ui->tongueOutUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { HandleClearImageUrl(PoseImage::TONGUEOUT); });

	QObject::connect(ui->poseListView, &QListView::clicked, this, &SettingsDialog::HandlePoseListClick);

	QObject::connect(ui->centerOnImagesToolButton, &QToolButton::clicked, this,
			 &SettingsDialog::HandleCenterViewButtonClick);
	QObject::connect(ui->moveImageUpLevelToolButton, &QToolButton::clicked, this,
			 &SettingsDialog::HandleMoveImageUpClick);
	QObject::connect(ui->moveImageDownLevelToolButton, &QToolButton::clicked, this,
			 &SettingsDialog::HandleMoveImageDownClick);

	QObject::connect(ui->zoomInToolButton, &QToolButton::clicked, this, [this]() { HandleImageZoomClick(false); });
	QObject::connect(ui->zoomOutToolButton, &QToolButton::clicked, this, [this]() { HandleImageZoomClick(true); });

	QObject::connect(poseListModel, &QAbstractItemModel::rowsInserted, this, &SettingsDialog::OnPoseRowsInserted);
	QObject::connect(poseListModel, &QAbstractItemModel::rowsRemoved, this, &SettingsDialog::OnPoseRowsRemoved);
	QObject::connect(poseListModel, &QAbstractItemModel::dataChanged, this, &SettingsDialog::OnPoseDataChanged);

	QPushButton *applyButton = ui->dialogButtonBox->button(QDialogButtonBox::Apply);
	if (applyButton) {
		connect(applyButton, &QPushButton::clicked, this, &SettingsDialog::ApplyButtonClicked);
	}
}

void SettingsDialog::ConnectObsSignalHandlers()
{
	// Source Signals
	signal_handler_connect(obs_get_signal_handler(), "source_create", OBSSourceCreated, ui);

	signal_handler_connect(obs_get_signal_handler(), "source_destroy", OBSSourceDeleted, ui);

	signal_handler_connect(obs_get_signal_handler(), "source_rename", OBSSourceRenamed, ui);
}

void SettingsDialog::SetTitle()
{
	QString dialogTitle = QString("Tracker %1").arg(trackerData->trackerId);
	this->setWindowTitle(dialogTitle);
}

void SettingsDialog::SetupDialogUI(QSharedPointer<TrackerDataStruct> settingsDialogData)
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
	ui->ipAddressViewLabel->setText(NetworkTracking::GetIpAddresses());

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

	UpdateStyledUIComponents();

	GetOBSSourceList();

	// Set form based on tracker data
	SetFormDetails(settingsDialogData);
}

void SettingsDialog::ApplyFormChanges()
{
	isError = false;
	if (trackerData != nullptr) {
		QVector<Result> validationResults;

		validationResults.append(ValidateTrackerID());
		validationResults.append(ValidateDestIPAddress());

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

		QSharedPointer<TrackerDataStruct> newData = QSharedPointer<TrackerDataStruct>::create();

		newData->trackerId = ui->trackerIdLineEdit->text();
		newData->selectedImageSource = ui->imageSourceDropdownList->currentText();
		newData->port = ui->portSpinBox->text().toInt();
		newData->destIpAddress = ui->destIpAddressLineEdit->text();
		newData->destPort = ui->destPortSpinBox->text().toInt();

		newData->poseList.clear();
		for (const QSharedPointer<Pose> &posePtr : settingsPoseList) {
			if (posePtr) {
				newData->poseList.append(posePtr->clone());
			}
		}

		ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
		emit SettingsUpdated(newData);
	} else {
		obs_log(LOG_WARNING, "No tracker data found!");
	}
}

Result SettingsDialog::ValidateTrackerID()
{
	Result updateIdResult = {true, ""};
	QLineEdit *idLineEdit = ui->trackerIdLineEdit;
	QString setTrackerId = idLineEdit->text();
	if ((setTrackerId != trackerData->trackerId && mainWidget)) {
		updateIdResult = mainWidget->ValidateNewTrackerID(setTrackerId);
		if (updateIdResult.success == true) {
			idLineEdit->setStyleSheet("");
			SetTitle();
		} else {
			idLineEdit->setStyleSheet(formErrorStyling);
			obs_log(LOG_WARNING, updateIdResult.errorMessage.toStdString().c_str());
		}
	}
	return updateIdResult;
}

Result SettingsDialog::ValidateDestIPAddress()
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

void SettingsDialog::GetOBSSourceList()
{
	// Get All Image Sources
	obs_enum_sources(GetImageSources, ui->imageSourceDropdownList);
}

void SettingsDialog::OBSSourceCreated(void *param, calldata_t *calldata)
{
	auto ui = static_cast<Ui::FaceTrackerDialog *>(param);
	obs_source_t *source;
	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	bool isImageSourceType = CheckIfImageSourceType(source);
	// If not sourceType we need;
	if (!isImageSourceType)
		return;

	const char *name = obs_source_get_name(source);
	ui->imageSourceDropdownList->addItem(name);
};

void SettingsDialog::OBSSourceDeleted(void *param, calldata_t *calldata)
{
	auto ui = static_cast<Ui::FaceTrackerDialog *>(param);

	obs_source_t *source;

	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	bool isImageSourceType = CheckIfImageSourceType(source);
	// If not sourceType we need;
	if (!isImageSourceType)
		return;

	const char *name = obs_source_get_name(source);

	int textIndexToRemove = ui->imageSourceDropdownList->findText(name);
	ui->imageSourceDropdownList->removeItem(textIndexToRemove);
};

bool SettingsDialog::GetImageSources(void *list_property, obs_source_t *source)
{
	if (!source)
		return true;
	bool isImageSourceType = CheckIfImageSourceType(source);
	// If not sourceType we need;
	if (!isImageSourceType)
		return true;

	QComboBox *sourceListUi = static_cast<QComboBox *>(list_property);

	const char *name = obs_source_get_name(source);
	sourceListUi->addItem(name);
	return true;
}

void SettingsDialog::OBSSourceRenamed(void *param, calldata_t *calldata)
{
	auto ui = static_cast<Ui::FaceTrackerDialog *>(param);

	obs_source_t *source;
	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	bool isImageSourceType = CheckIfImageSourceType(source);
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

int SettingsDialog::CheckIfImageSourceType(obs_source_t *source)
{
	const char *source_id = obs_source_get_unversioned_id(source);

	if (strcmp(source_id, "image_source") == 0)
		return true;

	return false;
}

void SettingsDialog::AddImageToScene(PoseImageData *imageData, bool clearScene)
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

	if (imageData->pixmapItem == nullptr)
		return obs_log(
			LOG_ERROR,
			QString("Image data for file: %1 not found!").arg(imageData->imageUrl).toStdString().c_str());

	avatarPreviewScene->addItem(imageData->pixmapItem);

	QObject::connect(imageData->pixmapItem, &MovablePixmapItem::positionChanged, this,
			 &SettingsDialog::HandleImageMove);

	CenterSceneOnItems();
}

void SettingsDialog::ClearScene()
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

void SettingsDialog::ClearCurrentPoseConfig()
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
	ClearScene();
}

void SettingsDialog::LoadSelectedPoseConfig()
{
	int selectedRow = GetSelectedRow();
	if (selectedRow == -1)
		return;

	if (selectedRow == previouslySelectedPoseIndex)
		return;

	previouslySelectedPoseIndex = selectedRow;

	QSharedPointer<Pose> selectedPose = settingsPoseList[selectedRow];
	ClearCurrentPoseConfig();
	ClearScene();

	for (size_t i = 0; i < static_cast<size_t>(PoseImage::COUNT); ++i) {
		PoseImage poseEnum = static_cast<PoseImage>(i);
		auto it = poseImageLineEdits.find(poseEnum);
		if (it != poseImageLineEdits.end()) {
			QLineEdit *lineEdit = it.value();
			QString fileName = selectedPose->getPoseImageData(poseEnum)->imageUrl;

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

			switch (poseEnum) {
			case PoseImage::BODY:
			case PoseImage::MOUTHCLOSED:
			case PoseImage::EYESOPEN:
				AddImageToScene(selectedPose->getPoseImageData(poseEnum));
				break;

			default:
				break;
			}
		}
	}
}

int SettingsDialog::GetSelectedRow()
{
	QModelIndex modelIndex = ui->poseListView->currentIndex();
	return modelIndex.row();
}

void SettingsDialog::ResetPoseUITab()
{
	ui->poseListView->selectionModel()->clearSelection();
	previouslySelectedPoseIndex = -1;
	ClearCurrentPoseConfig();
	ui->imageConfigWidget->setVisible(false);
	ui->noConfigLabel->setVisible(true);
}

void SettingsDialog::CenterSceneOnItems()
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

	ui->ipAddressViewLabel->setText(NetworkTracking::GetIpAddresses());

	ClearCurrentPoseConfig();
}

void SettingsDialog::closeEvent(QCloseEvent *)
{
	ResetPoseUITab();
}

//  ---------------------------------------------- Private Slots -----------------------------------------------

void SettingsDialog::FormChangeDetected()
{
	ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void SettingsDialog::ApplyButtonClicked()
{
	ApplyFormChanges();
}

void SettingsDialog::CancelButtonClicked()
{
	SetFormDetails(trackerData);
	ResetPoseUITab();
	this->reject();
}

void SettingsDialog::OkButtonClicked()
{
	ApplyFormChanges();
	ResetPoseUITab();
	if (!isError)
		this->reject();
}

void SettingsDialog::SyncPoseListToModel()
{
	QObject::disconnect(poseListModel, &QAbstractItemModel::rowsInserted, this,
			    &SettingsDialog::OnPoseRowsInserted);

	poseListModel->clear();

	settingsPoseList.clear();
	previouslySelectedPoseIndex = -1;

	for (int i = 0; i < trackerData->poseList.size(); ++i) {
		QSharedPointer<Pose> originalPosePtr = trackerData->poseList[i];
		if (!originalPosePtr) {
			obs_log(LOG_WARNING, QString("Pose pointer at index %1 is null.").arg(i).toStdString().c_str());
			continue; // Skip null pointers
		}

		// Clone the Pose to ensure an independent copy
		QSharedPointer<Pose> clonedPosePtr = originalPosePtr->clone();
		settingsPoseList.append(clonedPosePtr);

		const Pose &p = *clonedPosePtr;
		QStandardItem *item = new QStandardItem(p.poseId);

		poseListModel->appendRow(item);
	}

	QObject::connect(poseListModel, &QAbstractItemModel::rowsInserted, this, &SettingsDialog::OnPoseRowsInserted);
}

void SettingsDialog::AddPose()
{
	if (!poseListModel)
		return obs_log(LOG_ERROR, "No list model found!");

	int count = poseListModel->rowCount();
	QStandardItem *poseItem = new QStandardItem(QString("New Pose %1").arg(count));
	poseListModel->appendRow(poseItem);
}

void SettingsDialog::DeletePose()
{
	QModelIndex modelIndex = ui->poseListView->currentIndex();
	int rowIndex = modelIndex.row();

	obs_log(LOG_INFO, "Row To Delete: %d", rowIndex);

	if (rowIndex != -1) {
		poseListModel->takeRow(rowIndex);
	}
}

void SettingsDialog::HandleMovePose(bool isDirectionUp)
{
	if (!poseListModel)
		return;

	int selectedRow = GetSelectedRow();
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
	LoadSelectedPoseConfig();
	FormChangeDetected();
}

void SettingsDialog::HandleImageUrlButtonClicked(PoseImage poseEnum)
{
	QString fileName = QFileDialog::getOpenFileName(this, obs_module_text("ImageUrlFileDialogWindowTitle"),
							QString("%HOME%/Images"),
							tr("Images (*.png *.jpg *.bmp *.jpeg)"));

	this->raise();
	this->activateWindow();

	if (fileName.isEmpty()) {
		return;
	}

	int selectedRow = GetSelectedRow();
	if (selectedRow == -1)
		return;

	QSharedPointer<Pose> selectedPose = settingsPoseList[selectedRow];

	int imageIndex = static_cast<int>(poseEnum);

	if (!(imageIndex >= 0 && static_cast<size_t>(imageIndex) < selectedPose->poseImages.size())) {
		obs_log(LOG_WARNING, "Invalid PoseImage enum value.");
		return;
	}

	FormChangeDetected();

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

	selectedPose->poseImages[imageIndex].imageUrl = fileName;

	if (selectedPose->poseImages[imageIndex].pixmapItem) {
		delete selectedPose->poseImages[imageIndex].pixmapItem;
		selectedPose->poseImages[imageIndex].pixmapItem = nullptr;
	}

	QPixmap pixmap(fileName);
	if (pixmap.isNull()) {
		obs_log(LOG_WARNING,
			QString("Failed to load pixmap from file: %1").arg(fileName).toStdString().c_str());
		delete selectedPose->poseImages[imageIndex].pixmapItem;
		selectedPose->poseImages[imageIndex].pixmapItem = nullptr;
		return;
	}

	selectedPose->poseImages[imageIndex].pixmapItem = new MovablePixmapItem(pixmap);

	switch (poseEnum) {
	case PoseImage::BODY:
	case PoseImage::MOUTHCLOSED:
	case PoseImage::EYESOPEN:
		AddImageToScene(selectedPose->getPoseImageData(poseEnum));
		break;

	default:
		break;
	}
}

void SettingsDialog::HandleClearImageUrl(PoseImage poseEnum)
{
	int selectedRow = GetSelectedRow();
	if (selectedRow == -1)
		return;

	FormChangeDetected();

	QSharedPointer<Pose> selectedPose = settingsPoseList[selectedRow];
	int imageIndex = static_cast<int>(poseEnum);

	if (imageIndex < 0)
		return;

	if (imageIndex >= 0 && static_cast<size_t>(imageIndex) < selectedPose->poseImages.size()) {
		PoseImageData &poseData = selectedPose->poseImages[imageIndex];
		poseImageLineEdits[poseEnum]->setText(QString());

		poseData.imageUrl = QString();
		if (poseData.pixmapItem) {
			delete poseData.pixmapItem;
			poseData.pixmapItem = nullptr;
		}
	} else {
		obs_log(LOG_WARNING, "Invalid PoseImage enum value.");
	}
}

void SettingsDialog::HandlePoseListClick(QModelIndex modelIndex)
{
	obs_log(LOG_INFO, "Row: %d", modelIndex.row());
	LoadSelectedPoseConfig();
	ui->imageConfigWidget->setVisible(true);
	ui->noConfigLabel->setVisible(false);
}

void SettingsDialog::OnPoseRowsInserted(const QModelIndex &parent, int first, int last)
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
		newPose->poseId = poseId;

		settingsPoseList.insert(row, newPose);
	}
	LoadSelectedPoseConfig();
	FormChangeDetected();
}

void SettingsDialog::OnPoseRowsRemoved(const QModelIndex &parent, int first, int last)
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
	LoadSelectedPoseConfig();
	FormChangeDetected();
}

void SettingsDialog::OnPoseDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
				       const QList<int> &roles)
{
	Q_UNUSED(&roles);
	for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
		// Read changed data from the model and update the Pose
		QModelIndex idx = poseListModel->index(row, 0);
		QString newPoseId = idx.data(Qt::DisplayRole).toString();
		if (row >= 0 && row < settingsPoseList.size()) {
			settingsPoseList[row]->poseId = newPoseId;
		}
	}
	FormChangeDetected();
}

void SettingsDialog::HandleCenterViewButtonClick()
{
	CenterSceneOnItems();
}

void SettingsDialog::HandleMoveImageUpClick()
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

void SettingsDialog::HandleMoveImageDownClick()
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

void SettingsDialog::HandleImageZoomClick(bool isZoomOut)
{
	double zoomScaleFactor = 1.15;

	if (isZoomOut) {
		ui->avatarGraphicsView->scale(1.0 / zoomScaleFactor, 1.0 / zoomScaleFactor);
	} else {
		ui->avatarGraphicsView->scale(zoomScaleFactor, zoomScaleFactor);
	}
}

void SettingsDialog::HandleImageMove(qreal x, qreal y, qreal z, PoseImage pImageType)
{
	int selectedRow = GetSelectedRow();

	obs_log(LOG_INFO, QString("Updated Pose %1: x=%2, y=%3, z=%4")
				  .arg(x)
				  .arg(y)
				  .arg(z)
				  .toStdString()
				  .c_str());

	if (selectedRow != -1) {
		QSharedPointer<Pose> selectedPose = settingsPoseList[selectedRow];
		switch (pImageType) {
		case PoseImage::BODY:
			selectedPose->bodyPosition = {x, y, z};
			/* code */
			break;
		case PoseImage::MOUTHCLOSED:
			selectedPose->mouthPosition = {x, y, z};
			break;
		case PoseImage::EYESOPEN:
			selectedPose->eyesPosition = {x, y, z};
			break;

		default:
			break;
		}
		// Optionally, mark the form as changed
		FormChangeDetected();
		// Log the update
		obs_log(LOG_INFO, QString("Updated Pose %1: x=%2, y=%3, z=%4")
					  .arg(selectedPose->poseId)
					  .arg(x)
					  .arg(y)
					  .arg(z)
					  .toStdString()
					  .c_str());
	}

	FormChangeDetected();
}
