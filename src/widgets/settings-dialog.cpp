#include "settings-dialog.hpp"

SettingsDialog::SettingsDialog(QWidget *parent, TrackerDataStruct *tData, MainWidgetDock *mWidget)
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
	}

	QString trashIconPath = QDir::fromNativeSeparators(baseUrl + "trash.svg");
	if (QFileInfo::exists(trashIconPath)) {
		QIcon trashIcon(trashIconPath);
		ui->deletePoseToolButton->setIcon(trashIcon);

		ui->bodyUrlDeleteToolButton->setIcon(trashIcon);
		ui->eyesOpenUrlDeleteToolButton->setIcon(trashIcon);
		ui->eyesHalfOpenUrlDeleteToolButton->setIcon(trashIcon);
		ui->eyesClosedUrlDeleteToolButton->setIcon(trashIcon);
		ui->mouthClosedUrlDeleteToolButton->setIcon(trashIcon);
		ui->mouthOpenUrlDeleteToolButton->setIcon(trashIcon);
		ui->mouthSmileUrlDeleteToolButton->setIcon(trashIcon);
		ui->tongueOutUrlDeleteToolButton->setIcon(trashIcon);
	}

	obs_log(LOG_INFO, "Is theme dark: %s", obs_frontend_is_theme_dark() ? "true" : "false");
	obs_log(LOG_INFO, "file: %s", searchIconPath.toStdString().c_str());
	obs_log(LOG_INFO, "Exists: %s", FileExists(searchIconPath) ? "true" : "false");
}

void SettingsDialog::SetFormDetails(TrackerDataStruct *settingsDialogData)
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

void SettingsDialog::SetupDialogUI(TrackerDataStruct *settingsDialogData)
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

	ui->poseListLabel->setText(obs_module_text("DialogPostListLabel"));
	ui->poseImageLabel->setText(obs_module_text("DialogPoseImageLabel"));
	poseListModel = new QStandardItemModel(this);
	ui->poseListView->setModel(poseListModel);

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

	ui->imageConfigWidget->setVisible(false);

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

		TrackerDataStruct *newData = new TrackerDataStruct;

		newData->trackerId = ui->trackerIdLineEdit->text();
		newData->selectedImageSource = ui->imageSourceDropdownList->currentText();
		newData->port = ui->portSpinBox->text().toInt();
		newData->destIpAddress = ui->destIpAddressLineEdit->text();
		newData->destPort = ui->destPortSpinBox->text().toInt();
		newData->poseList = settingsPoseList;

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

void SettingsDialog::AddImageToQGraphics(QString fileUrl, bool clearScene)
{
	if (!avatarPreviewScene || clearScene) {
		QGraphicsView *view = ui->avatarGraphicsView;
		avatarPreviewScene = new QGraphicsScene(this);

		avatarPreviewScene->setSceneRect(0, 0, 10000, 10000);
		view->scale(0.5, 0.5);

		view->setScene(avatarPreviewScene);
		view->setRenderHint(QPainter::Antialiasing);
		// Create a QGraphicsView to visualize the scene
		view->setDragMode(QGraphicsView::RubberBandDrag); // optional: for selecting items
								  // view->setDragMode(QGraphicsView::NoDrag);
	}

	if (!FileExists(fileUrl))
		return obs_log(LOG_ERROR, QString("File %1 not found!").arg(fileUrl).toStdString().c_str());

	// Example: Add one image item
	QGraphicsPixmapItem *pixmapItem = new QGraphicsPixmapItem(QPixmap(fileUrl));
	pixmapItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
	avatarPreviewScene->addItem(pixmapItem);
}

void SettingsDialog::ClearCurrentPoseConfig()
{
	ui->bodyUrlLineEdit->setText("");
	ui->eyesOpenUrlLineEdit->setText("");
	ui->eyesHalfOpenUrlEdit->setText("");
	ui->eyesClosedUrlLineEdit->setText("");
	ui->mouthOpenUrlLineEdit->setText("");
	ui->mouthClosedUrlLineEdit->setText("");
	ui->mouthSmileUrlLineEdit->setText("");
	ui->tongueOutUrlLineEdit->setText("");
}

void SettingsDialog::LoadSelectedPoseConfig()
{
	int selectedRow = GetSelectedRow();
	if (selectedRow == -1)
		return;

	Pose *selectedPose = &settingsPoseList[selectedRow];
	ClearCurrentPoseConfig();

	ui->bodyUrlLineEdit->setText(selectedPose->bodyImageUrl);
	ui->eyesOpenUrlLineEdit->setText(selectedPose->eyesOpenImageUrl);
	ui->eyesHalfOpenUrlEdit->setText(selectedPose->eyesHalfOpenImageUrl);
	ui->eyesClosedUrlLineEdit->setText(selectedPose->eyesClosedImageUrl);
	ui->mouthOpenUrlLineEdit->setText(selectedPose->mouthOpenImageUrl);
	ui->mouthClosedUrlLineEdit->setText(selectedPose->mouthClosedImageUrl);
	ui->mouthSmileUrlLineEdit->setText(selectedPose->mouthSmileImageUrl);
	ui->tongueOutUrlLineEdit->setText(selectedPose->mouthTongueOutImageUrl);

	if (!selectedPose->bodyImageUrl.isEmpty())
		AddImageToQGraphics(selectedPose->bodyImageUrl, true);

	if (!selectedPose->eyesOpenImageUrl.isEmpty())
		AddImageToQGraphics(selectedPose->eyesOpenImageUrl);

	if (!selectedPose->mouthClosedImageUrl.isEmpty())
		AddImageToQGraphics(selectedPose->mouthClosedImageUrl);
}

int SettingsDialog::GetSelectedRow()
{
	QModelIndex modelIndex = ui->poseListView->currentIndex();
	return modelIndex.row();
}

//  ------------------------------------------------ Protected ------------------------------------------------

void SettingsDialog::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);

	// Reset stylings
	ui->trackerIdLineEdit->setStyleSheet("");
	ui->destIpAddressLineEdit->setStyleSheet("");

	ui->ipAddressViewLabel->setText(NetworkTracking::GetIpAddresses());
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
	this->reject();
}

void SettingsDialog::OkButtonClicked()
{
	ApplyFormChanges();
	if (!isError)
		this->reject();
}

void SettingsDialog::SyncPoseListToModel()
{
	// Clear the model
	poseListModel->clear();
	settingsPoseList = trackerData->poseList;

	// Refill from the poseList
	for (int i = 0; i < trackerData->poseList.size(); ++i) {
		const Pose &p = trackerData->poseList[i];

		// For a single-column model, store display text as Pose ID
		QStandardItem *item = new QStandardItem(p.poseId);
		// If you need more data, you can store it in user roles
		item->setData(p.bodyImageUrl, Qt::UserRole + 1);
		// ... etc.

		poseListModel->appendRow(item);
	}
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

void SettingsDialog::HandleImageUrlButtonClicked(PoseImage image)
{
	// if (selectedPoseIndex < 0 || selectedPoseIndex >= trackerData->poseList.size())
	// 	return obs_log(LOG_ERROR, "Error finding selected Post at index: %d", selectedPoseIndex);

	// Pose selectedPost = trackerData->poseList[selectedPoseIndex];

	QString fileName = QFileDialog::getOpenFileName(this, obs_module_text("ImageUrlFileDialogWindowTitle"),
							QString(), tr("Images (*.png *.jpg *.bmp *.jpeg)"));
	obs_log(LOG_INFO, "File: %s", fileName.toStdString().c_str());

	this->raise();
	this->activateWindow();

	if (fileName.isEmpty()) {
		return;
	}

	FormChangeDetected();

	int selectedRow = GetSelectedRow();
	if (selectedRow == -1)
		return;

	Pose *selectedPose = &settingsPoseList[selectedRow];

	switch (image) {
	case PoseImage::BODY:
		selectedPose->bodyImageUrl = fileName;
		AddImageToQGraphics(selectedPose->bodyImageUrl);
		ui->bodyUrlLineEdit->setText(selectedPose->bodyImageUrl);
		break;
	case PoseImage::EYESOPEN:
		selectedPose->eyesOpenImageUrl = fileName;
		AddImageToQGraphics(selectedPose->eyesOpenImageUrl);
		ui->eyesOpenUrlLineEdit->setText(selectedPose->eyesOpenImageUrl);
		break;
	case PoseImage::MOUTHCLOSED:
		selectedPose->mouthClosedImageUrl = fileName;
		AddImageToQGraphics(selectedPose->mouthClosedImageUrl);
		ui->mouthClosedUrlLineEdit->setText(selectedPose->mouthClosedImageUrl);
		break;
	case PoseImage::EYESCLOSED:
		selectedPose->eyesClosedImageUrl = fileName;
		ui->eyesClosedUrlLineEdit->setText(selectedPose->eyesClosedImageUrl);
		break;
	case PoseImage::EYESHALFOPEN:
		selectedPose->eyesHalfOpenImageUrl = fileName;
		ui->eyesHalfOpenUrlEdit->setText(selectedPose->eyesHalfOpenImageUrl);
		break;
	case PoseImage::MOUTHOPEN:
		selectedPose->mouthOpenImageUrl = fileName;
		ui->mouthOpenUrlLineEdit->setText(selectedPose->mouthOpenImageUrl);
		break;
	case PoseImage::MOUTHSMILE:
		selectedPose->mouthSmileImageUrl = fileName;
		ui->mouthSmileUrlLineEdit->setText(selectedPose->mouthSmileImageUrl);
		break;
	case PoseImage::TONGUEOUT:
		selectedPose->mouthTongueOutImageUrl = fileName;
		ui->tongueOutUrlLineEdit->setText(selectedPose->mouthTongueOutImageUrl);
		break;

	default:
		break;
	}
}

void SettingsDialog::HandleClearImageUrl(PoseImage image)
{
	int selectedRow = GetSelectedRow();
	if (selectedRow == -1)
		return;

	FormChangeDetected();

	Pose *selectedPose = &settingsPoseList[selectedRow];

	switch (image) {
	case PoseImage::BODY:
		selectedPose->bodyImageUrl = QString();
		ui->bodyUrlLineEdit->setText(QString());
		// Remove Image
		break;
	case PoseImage::EYESOPEN:
		selectedPose->eyesOpenImageUrl = QString();
		ui->eyesOpenUrlLineEdit->setText(QString());
		// Remove Image
		break;
	case PoseImage::MOUTHCLOSED:
		selectedPose->mouthClosedImageUrl = QString();
		ui->mouthClosedUrlLineEdit->setText(QString());
		// Remove Image
		break;
	case PoseImage::EYESCLOSED:
		selectedPose->eyesClosedImageUrl = QString();
		ui->eyesClosedUrlLineEdit->setText(QString());
		break;
	case PoseImage::EYESHALFOPEN:
		selectedPose->eyesHalfOpenImageUrl = QString();
		ui->eyesHalfOpenUrlEdit->setText(QString());
		break;
	case PoseImage::MOUTHOPEN:
		selectedPose->mouthOpenImageUrl = QString();
		ui->mouthOpenUrlLineEdit->setText(QString());
		break;
	case PoseImage::MOUTHSMILE:
		selectedPose->mouthSmileImageUrl = QString();
		ui->mouthSmileUrlLineEdit->setText(QString());
		break;
	case PoseImage::TONGUEOUT:
		selectedPose->mouthTongueOutImageUrl = QString();
		ui->tongueOutUrlLineEdit->setText(QString());
		break;

	default:
		break;
	}
}

void SettingsDialog::HandlePoseListClick(QModelIndex modelIndex)
{
	obs_log(LOG_INFO, "Row: %d", modelIndex.row());
	LoadSelectedPoseConfig();
	ui->imageConfigWidget->setVisible(true);
}

void SettingsDialog::OnPoseRowsInserted(const QModelIndex &parent, int first, int last)
{
	Q_UNUSED(parent);

	// For each newly inserted row, create a default Pose or read from the model
	// and insert into settingsPoseList at the correct position.
	for (int row = first; row <= last; ++row) {
		// E.g. read the poseId from the model
		QModelIndex idx = poseListModel->index(row, 0);
		QString poseId = idx.data(Qt::DisplayRole).toString();

		Pose newPose;
		newPose.poseId = poseId;

		settingsPoseList.insert(row, newPose);
	}
	LoadSelectedPoseConfig();
	FormChangeDetected();
}

void SettingsDialog::OnPoseRowsRemoved(const QModelIndex &parent, int first, int last)
{
	Q_UNUSED(parent);

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
			settingsPoseList[row].poseId = newPoseId;
		}
	}
	FormChangeDetected();
}
