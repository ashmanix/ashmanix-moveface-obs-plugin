#include "face-settings-widget.h"
#include <obs-module.h>

FaceSettingsWidget::FaceSettingsWidget(QWidget *parent, QSharedPointer<Pose> poseData)
	: QWidget(parent),
	  ui(new Ui::FaceSettingsWidget)
{
	UNUSED_PARAMETER(poseData);
	ui->setupUi(this);

	setupWidgetUI();

	connectUISignalHandlers();

	if (poseData)
		setData(poseData);
}

FaceSettingsWidget::~FaceSettingsWidget() = default;

void FaceSettingsWidget::clearSelection()
{
	blockSignals(true);

	for (auto [key, value] : faceConfigDoubleSpinBoxes.asKeyValueRange()) {
		value->setValue(0);
	}

	for (auto [key, value] : faceConfigSliders.asKeyValueRange()) {
		value->setValue(0);
	}

	blockSignals(false);
}

void FaceSettingsWidget::toggleVisible(bool isVisible)
{
	if (isVisible) {
		ui->faceConfigWidget->setVisible(true);
		ui->noConfigLabel->setVisible(false);
	} else {
		ui->faceConfigWidget->setVisible(false);
		ui->noConfigLabel->setVisible(true);
	}
}

void FaceSettingsWidget::setData(QSharedPointer<Pose> poseData)
{
	toggleBlockAllUISignals(true);

	ui->eyesHalfOpenSpinBox->setValue(poseData->getEyesHalfOpenLimit());
	ui->eyesHalfOpenSlider->setValue(poseData->getEyesHalfOpenLimit() * 1000);

	ui->eyesOpenSpinBox->setValue(poseData->getEyesOpenLimit());
	ui->eyesOpenSlider->setValue(poseData->getEyesOpenLimit() * 1000);

	ui->mouthOpenSpinBox->setValue(poseData->getMouthOpenLimit());
	ui->mouthOpenSlider->setValue(poseData->getMouthOpenLimit() * 1000);

	ui->tongueOutSpinBox->setValue(poseData->getTongueOutLimit());
	ui->tongueOutSlider->setValue(poseData->getTongueOutLimit() * 1000);

	toggleBlockAllUISignals(false);
}

//  ------------------------------------------------- Private --------------------------------------------------

void FaceSettingsWidget::connectUISignalHandlers()
{
	QObject::connect(ui->eyesHalfOpenSlider, &QSlider::valueChanged, this,
			 [this](double value) { handleSliderMovement(PoseImage::EYESHALFOPEN, value); });
	QObject::connect(ui->eyesOpenSlider, &QSlider::valueChanged, this,
			 [this](double value) { handleSliderMovement(PoseImage::EYESOPEN, value); });
	QObject::connect(ui->mouthOpenSlider, &QSlider::valueChanged, this,
			 [this](double value) { handleSliderMovement(PoseImage::MOUTHOPEN, value); });
	QObject::connect(ui->tongueOutSlider, &QSlider::valueChanged, this,
			 [this](double value) { handleSliderMovement(PoseImage::TONGUEOUT, value); });

	QObject::connect(ui->eyesHalfOpenSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { handleSpinBoxChange(PoseImage::EYESHALFOPEN, value); });
	QObject::connect(ui->eyesOpenSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { handleSpinBoxChange(PoseImage::EYESOPEN, value); });
	QObject::connect(ui->mouthOpenSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { handleSpinBoxChange(PoseImage::MOUTHOPEN, value); });
	QObject::connect(ui->tongueOutSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { handleSpinBoxChange(PoseImage::TONGUEOUT, value); });
}

void FaceSettingsWidget::setupWidgetUI()
{
	ui->faceSettingsLabel->setText(obs_module_text("DialogFaceSettingsLabel"));

	ui->eyesClosedControlLabel->setText(obs_module_text("DialogEyesClosedLabel"));
	ui->eyesHalfOpenControlLabel->setText(obs_module_text("DialogEyesHalfOpenLabel"));
	ui->eyesHalfOpen2ControlLabel->setText(obs_module_text("DialogEyesHalfOpenLabel"));
	ui->eyesOpenControlLabel->setText(obs_module_text("DialogEyesOpenLabel"));
	ui->mouthClosedControlLabel->setText(obs_module_text("DialogMouthClosedLabel"));
	ui->mouthOpenControlLabel->setText(obs_module_text("DialogMouthOpenLabel"));
	ui->tongueOutControlLabel->setText(obs_module_text("DialogTongueInLabel"));
	ui->tongueInControlLabel->setText(obs_module_text("DialogTongueOutLabel"));

	ui->eyesClosedControlLabel->setToolTip(obs_module_text("DialogEyesClosedLabel"));
	ui->eyesHalfOpenControlLabel->setToolTip(obs_module_text("DialogEyesHalfOpenLabel"));
	ui->eyesHalfOpen2ControlLabel->setToolTip(obs_module_text("DialogEyesHalfOpenLabel"));
	ui->eyesOpenControlLabel->setToolTip(obs_module_text("DialogEyesOpenLabel"));
	ui->mouthClosedControlLabel->setToolTip(obs_module_text("DialogMouthClosedLabel"));
	ui->mouthOpenControlLabel->setToolTip(obs_module_text("DialogMouthOpenLabel"));
	ui->tongueOutControlLabel->setToolTip(obs_module_text("DialogTongueInLabel"));
	ui->tongueInControlLabel->setToolTip(obs_module_text("DialogTongueOutLabel"));

	// Initialize the mapping between PoseImage enums and QLineEdit pointers
	faceConfigDoubleSpinBoxes[PoseImage::EYESHALFOPEN] = ui->eyesHalfOpenSpinBox;
	faceConfigDoubleSpinBoxes[PoseImage::EYESOPEN] = ui->eyesOpenSpinBox;
	faceConfigDoubleSpinBoxes[PoseImage::MOUTHOPEN] = ui->mouthOpenSpinBox;
	faceConfigDoubleSpinBoxes[PoseImage::TONGUEOUT] = ui->tongueOutSpinBox;

	faceConfigSliders[PoseImage::EYESHALFOPEN] = ui->eyesHalfOpenSlider;
	faceConfigSliders[PoseImage::EYESOPEN] = ui->eyesOpenSlider;
	faceConfigSliders[PoseImage::MOUTHOPEN] = ui->mouthOpenSlider;
	faceConfigSliders[PoseImage::TONGUEOUT] = ui->tongueOutSlider;

	toggleVisible(false);

	updateStyledUIComponents();
}

void FaceSettingsWidget::updateStyledUIComponents()
{
	ui->faceSettingsScrollArea->setStyleSheet("QScrollArea {"
						  "background-color: transparent;"
						  "}"
						  "#trackingScrollAreaWidgetContents {"
						  "background-color: transparent;"
						  "}");
	ui->noConfigLabel->setStyleSheet("font-size: 20pt; padding-bottom: 40px;");
	ui->noConfigLabel->setText(obs_module_text("DialogNoConfigMessage"));
}

void FaceSettingsWidget::toggleBlockAllUISignals(bool shouldBlock)
{
	for (auto [key, value] : faceConfigDoubleSpinBoxes.asKeyValueRange()) {
		value->blockSignals(shouldBlock);
	}

	for (auto [key, value] : faceConfigSliders.asKeyValueRange()) {
		value->blockSignals(shouldBlock);
	}
}

//  ---------------------------------------------- Private Slots -----------------------------------------------

void FaceSettingsWidget::handleSliderMovement(PoseImage poseEnum, double spinnerValue)
{
	// QString poseEnumString = poseImageToString(poseEnum);
	// obs_log(LOG_INFO, "Slider: %s value: %s", poseEnumString.toStdString().c_str(), std::to_string(value).c_str());

	// Slider value runs between 0 - 1000 so spinbox value is spinnerValue/1000
	double blendshapeLimit = spinnerValue / 1000;

	if (auto it = faceConfigDoubleSpinBoxes.find(poseEnum); it != faceConfigDoubleSpinBoxes.end()) {
		QDoubleSpinBox *dSpinBox = it.value(); // For QMap
		dSpinBox->blockSignals(true);
		if (dSpinBox) {
			dSpinBox->setValue(spinnerValue / 1000);
			emit blendshapeLimitChanged(poseEnum, blendshapeLimit);
		}
		// Eyes open value cannot be less than eyes half open value
		if (poseEnum == PoseImage::EYESHALFOPEN && (ui->eyesOpenSlider->value() < spinnerValue)) {
			ui->eyesOpenSlider->blockSignals(true);
			ui->eyesOpenSpinBox->blockSignals(true);

			ui->eyesOpenSlider->setValue(spinnerValue);
			ui->eyesOpenSpinBox->setValue(blendshapeLimit);

			ui->eyesOpenSlider->blockSignals(false);
			ui->eyesOpenSpinBox->blockSignals(false);
			emit blendshapeLimitChanged(PoseImage::EYESOPEN, blendshapeLimit);
		} else if (poseEnum == PoseImage::EYESOPEN && (ui->eyesHalfOpenSlider->value() > spinnerValue)) {
			ui->eyesHalfOpenSlider->blockSignals(true);
			ui->eyesHalfOpenSpinBox->blockSignals(true);

			ui->eyesHalfOpenSlider->setValue(spinnerValue);
			ui->eyesHalfOpenSpinBox->setValue(blendshapeLimit);

			ui->eyesHalfOpenSlider->blockSignals(false);
			ui->eyesHalfOpenSpinBox->blockSignals(false);
			emit blendshapeLimitChanged(PoseImage::EYESHALFOPEN, blendshapeLimit);
		}
		dSpinBox->blockSignals(false);
	}
}

void FaceSettingsWidget::handleSpinBoxChange(PoseImage poseEnum, double spinBoxValue)
{
	// QString poseEnumString = poseImageToString(poseEnum);
	// obs_log(LOG_INFO, "SpinBox: %s value: %f", poseEnumString.toStdString().c_str(), value);

	// Slider value runs between 0 - 1000 so slider value is spinbox value * 1000
	double sliderValue = spinBoxValue * 1000;

	if (auto it = faceConfigSliders.find(poseEnum); it != faceConfigSliders.end()) {
		QSlider *slider = it.value(); // For QMap
		slider->blockSignals(true);
		if (slider) {
			slider->setValue(sliderValue);
			emit blendshapeLimitChanged(poseEnum, spinBoxValue);
		}
		// Eyes open value cannot be less than eyes half open value
		if (poseEnum == PoseImage::EYESHALFOPEN && (ui->eyesOpenSlider->value() < spinBoxValue)) {
			ui->eyesOpenSlider->blockSignals(true);
			ui->eyesOpenSpinBox->blockSignals(true);

			ui->eyesOpenSlider->setValue(sliderValue);
			ui->eyesOpenSpinBox->setValue(spinBoxValue);

			ui->eyesOpenSlider->blockSignals(false);
			ui->eyesOpenSpinBox->blockSignals(false);
			emit blendshapeLimitChanged(PoseImage::EYESOPEN, spinBoxValue);
		} else if (poseEnum == PoseImage::EYESOPEN && (ui->eyesHalfOpenSlider->value() > spinBoxValue)) {
			ui->eyesHalfOpenSlider->blockSignals(true);
			ui->eyesHalfOpenSpinBox->blockSignals(true);

			ui->eyesHalfOpenSlider->setValue(sliderValue);
			ui->eyesHalfOpenSpinBox->setValue(spinBoxValue);

			ui->eyesHalfOpenSlider->blockSignals(false);
			ui->eyesHalfOpenSpinBox->blockSignals(false);
			emit blendshapeLimitChanged(PoseImage::EYESHALFOPEN, spinBoxValue);
		}
		slider->blockSignals(false);
	}
}
