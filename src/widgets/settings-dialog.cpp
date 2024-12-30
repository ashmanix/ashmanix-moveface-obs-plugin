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

	QObject::connect(ui->bodyUrlPushButton, &QPushButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::BODY); });
	QObject::connect(ui->eyesOpenUrlPushButton, &QPushButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::EYESOPEN); });
	QObject::connect(ui->eyesHalfOpenUrlPushButton, &QPushButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::EYESHALFOPEN); });
	QObject::connect(ui->eyesClosedUrlPushButton, &QPushButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::EYESCLOSED); });
	QObject::connect(ui->mouthClosedUrlPushButton, &QPushButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::MOUTHCLOSED); });
	QObject::connect(ui->mouthOpenUrlPushButton, &QPushButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::MOUTHOPEN); });
	QObject::connect(ui->mouthSmileUrlPushButton, &QPushButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::MOUTHSMILE); });
	QObject::connect(ui->tongueOutUrlPushButton, &QPushButton::clicked, this,
			 [this]() { HandleImageUrlButtonClicked(PoseImage::TONGUEOUT); });

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

	ui->bodyUrlPushButton->setText(obs_module_text("DialogImageUrlBrowseButtonText"));
	ui->eyesOpenUrlPushButton->setText(obs_module_text("DialogImageUrlBrowseButtonText"));
	ui->eyesHalfOpenUrlPushButton->setText(obs_module_text("DialogImageUrlBrowseButtonText"));
	ui->eyesClosedUrlPushButton->setText(obs_module_text("DialogImageUrlBrowseButtonText"));
	ui->mouthClosedUrlPushButton->setText(obs_module_text("DialogImageUrlBrowseButtonText"));
	ui->mouthOpenUrlPushButton->setText(obs_module_text("DialogImageUrlBrowseButtonText"));
	ui->mouthSmileUrlPushButton->setText(obs_module_text("DialogImageUrlBrowseButtonText"));
	ui->tongueOutUrlPushButton->setText(obs_module_text("DialogImageUrlBrowseButtonText"));

	ui->imageConfigWidget->setVisible(false);

	GetOBSSourceList();

	// Set form based on tracker data
	SetFormDetails(settingsDialogData);

	AddImageToQGraphics();
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

void SettingsDialog::AddImageToQGraphics()
{
	// Create a QGraphicsScene
	QGraphicsScene *scene = new QGraphicsScene(this);

	// Create a QGraphicsView to visualize the scene
	QGraphicsView *view = ui->avatarGraphicsView;
	view->setScene(scene);
	view->setRenderHint(QPainter::Antialiasing);
	view->setDragMode(QGraphicsView::ScrollHandDrag); // optional: for selecting items

	// Example: Add one image item
	QGraphicsPixmapItem *pixmapItem = new QGraphicsPixmapItem(QPixmap("/Users/ash/Images/YouTube/JobbyPic.png"));
	pixmapItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
	scene->addItem(pixmapItem);

	// Optionally, add a second movable image
	QGraphicsPixmapItem *pixmapItem2 = new QGraphicsPixmapItem(QPixmap("/Users/ash/Images/YouTube/JobbyPic.png"));
	pixmapItem2->setPos(100, 50); // set initial position
	pixmapItem2->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
	scene->addItem(pixmapItem2);

	scene->setSceneRect(0, 0, 400, 300);
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

	switch (image) {
	case PoseImage::BODY:

		break;

	default:
		break;
	}
}

void SettingsDialog::HandlePoseListClick(QModelIndex modelIndex)
{
	obs_log(LOG_INFO, "Row: %d", modelIndex.row());
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
