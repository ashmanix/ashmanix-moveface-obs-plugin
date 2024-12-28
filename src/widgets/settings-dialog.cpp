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
}

SettingsDialog::~SettingsDialog()
{
	this->deleteLater();
}

void SettingsDialog::SetFormDetails(TrackerDataStruct *settingsDialogData)
{
	ui->ipAddressViewLabel->setText(NetworkTracking::GetIpAddresses());

	if (settingsDialogData != nullptr) {
		ui->trackerIdLineEdit->setText(settingsDialogData->trackerId);

		ui->portSpinBox->setValue(settingsDialogData->port);
		ui->destIpAddressLineEdit->setText(settingsDialogData->destIpAddress);
		ui->destPortSpinBox->setValue(settingsDialogData->destPort);

		ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	} else {
		obs_log(LOG_WARNING, "No tracker data found!");
	}
}

void SettingsDialog::ConnectUISignalHandlers()
{
	QObject::connect(ui->trackerIdLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->destIpAddressLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->destPortSpinBox, &QSpinBox::valueChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->portSpinBox, &QSpinBox::valueChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->dialogButtonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::OkButtonClicked);

	QObject::connect(ui->dialogButtonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::CancelButtonClicked);

	QPushButton *applyButton = ui->dialogButtonBox->button(QDialogButtonBox::Apply);
	if (applyButton) {
		connect(applyButton, &QPushButton::clicked, this, &SettingsDialog::ApplyButtonClicked);
	}
}

void SettingsDialog::SetTitle()
{
	QString dialogTitle = QString("Tracker %1").arg(trackerData->trackerId);
	this->setWindowTitle(dialogTitle);
}

void SettingsDialog::SetupDialogUI(TrackerDataStruct *settingsDialogData)
{
	UNUSED_PARAMETER(settingsDialogData);

	ui->trackerIdLabel->setText(obs_module_text("DialogIdLabel"));

	ui->destIpAddressLabel->setText(obs_module_text("DialogDestIpAddressLabel"));
	ui->destIpAddressLineEdit->setValidator(new QRegularExpressionValidator(
		QRegularExpression(R"(^((25[0-5]|2[0-4]\d|[01]?\d?\d)\.){3}(25[0-5]|2[0-4]\d|[01]?\d?\d)$)"), this));

	ui->portLabel->setText(obs_module_text("DialogPortAddress"));
	ui->portSpinBox->setRange(0, 65535);

	ui->ipAddressLabel->setText(obs_module_text("DialogIpAddressLabel"));
	ui->ipAddressViewLabel->setText(NetworkTracking::GetIpAddresses());

	ui->destPortLabel->setText(obs_module_text("DialogDestPortAddress"));
	ui->destPortSpinBox->setRange(0, 65535);

	ui->generalGroupBox->setTitle(obs_module_text("DialogGeneralGroupBoxTitle"));
	ui->connectionGroupBox->setTitle(obs_module_text("DialogConnectionGroupBoxTitle"));

	ui->byLabel->setText(obs_module_text("DialogInfoByLabel"));
	ui->contributorsLabel->setText(obs_module_text("DialogInfoConstributorsLabel"));
	ui->versionLabel->setText(obs_module_text("DialogInfoVersionLabel"));
	ui->versionTextLabel->setText(PLUGIN_VERSION);

	ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setText(obs_module_text("DialogButtonApplyLabel"));
	ui->dialogButtonBox->button(QDialogButtonBox::Ok)->setText(obs_module_text("DialogButtonOkLabel"));
	ui->dialogButtonBox->button(QDialogButtonBox::Cancel)->setText(obs_module_text("DialogButtonCancelLabel"));

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
		newData->port = ui->portSpinBox->text().toInt();
		newData->destIpAddress = ui->destIpAddressLineEdit->text();
		newData->destPort = ui->destPortSpinBox->text().toInt();

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

// ----------------------------------- Protected ------------------------------------

void SettingsDialog::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);

	// Reset stylings
	ui->trackerIdLineEdit->setStyleSheet("");
	ui->destIpAddressLineEdit->setStyleSheet("");

	ui->ipAddressViewLabel->setText(NetworkTracking::GetIpAddresses());
}

// --------------------------------- Private Slots ----------------------------------

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
