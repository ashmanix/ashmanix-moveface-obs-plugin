#include "settings-dialog.h"

SettingsDialog::SettingsDialog(QWidget *parent, QSharedPointer<TrackerData> tData, MainWidgetDock *mWidget)
	: QDialog(parent),
	  m_ui(new Ui::FaceTrackerDialog)
{
	m_ui->setupUi(this);
	m_trackerData = tData;
	m_mainWidget = mWidget;
	m_poseListWidget = new PoseListWidget(this, m_trackerData);

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
	for (SettingsWidgetInterface *widget : m_trackingWidgetList) {
		widget->updateStyledUIComponents();
	}
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

	QObject::connect(m_imageFilesWidget, &ImageFilesWidget::imageUrlSet, m_poseDisplayWidget,
			 &PoseDisplayWidget::handleSetImageUrl);
	QObject::connect(m_imageFilesWidget, &ImageFilesWidget::imageUrlCleared, m_poseDisplayWidget,
			 &PoseDisplayWidget::handleClearImageUrl);
	QObject::connect(m_imageFilesWidget, &ImageFilesWidget::raiseWindow, this, &SettingsDialog::handleRaisedWindow);

	QObject::connect(m_poseDisplayWidget, &PoseDisplayWidget::poseImagesChanged, this,
			 &SettingsDialog::formChangeDetected);

	QObject::connect(m_poseListWidget, &PoseListWidget::rowsInserted, this, &SettingsDialog::onPoseRowsInserted);
	QObject::connect(m_poseListWidget, &PoseListWidget::rowsRemoved, this, &SettingsDialog::onPoseRowsRemoved);
	QObject::connect(m_poseListWidget, &PoseListWidget::rowsDataChanged, this, &SettingsDialog::onPoseDataChanged);
	QObject::connect(m_poseListWidget, &PoseListWidget::rowsSelected, this, &SettingsDialog::onPoseSelected);
	QObject::connect(m_poseListWidget, &PoseListWidget::rowMoved, this, &SettingsDialog::onPoseRowMoved);

	QObject::connect(m_faceSettingsWidget, &FaceSettingsWidget::blendshapeLimitChanged, this,
			 &SettingsDialog::formChangeDetected);

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
	m_ui->poseDisplayVerticalLayout->addWidget(m_poseDisplayWidget);

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

	auto sourceListUi = static_cast<QComboBox *>(list_property);

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

void SettingsDialog::clearCurrentPoseConfig()
{
	for (SettingsWidgetInterface *widget : m_trackingWidgetList)
		widget->clearSelection();

	// clearScene();
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

	for (SettingsWidgetInterface *widget : m_trackingWidgetList) {
		widget->setData(selectedPose);
		widget->toggleVisible(true);
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

	for (SettingsWidgetInterface *widget : m_trackingWidgetList) {
		widget->toggleVisible(false);
	}
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
