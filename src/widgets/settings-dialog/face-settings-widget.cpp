#include "face-settings-widget.h"
#include <obs-module.h>

FaceSettingsWidget::FaceSettingsWidget(QWidget *parent, QSharedPointer<Pose> poseData)
	: QWidget(parent),
	  m_ui(new Ui::FaceSettingsWidget)
{
	UNUSED_PARAMETER(poseData);
	m_ui->setupUi(this);

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
		m_ui->faceConfigWidget->setVisible(true);
		m_ui->noConfigLabel->setVisible(false);
	} else {
		m_ui->faceConfigWidget->setVisible(false);
		m_ui->noConfigLabel->setVisible(true);
	}
}

void FaceSettingsWidget::setData(QSharedPointer<Pose> poseData)
{
	toggleBlockAllUISignals(true);

	m_ui->eyesHalfOpenSpinBox->setValue(poseData->getEyesHalfOpenLimit());
	m_ui->eyesHalfOpenSlider->setValue(poseData->getEyesHalfOpenLimit() * 1000);

	m_ui->eyesOpenSpinBox->setValue(poseData->getEyesOpenLimit());
	m_ui->eyesOpenSlider->setValue(poseData->getEyesOpenLimit() * 1000);

	m_ui->mouthOpenSpinBox->setValue(poseData->getMouthOpenLimit());
	m_ui->mouthOpenSlider->setValue(poseData->getMouthOpenLimit() * 1000);

	m_ui->tongueOutSpinBox->setValue(poseData->getTongueOutLimit());
	m_ui->tongueOutSlider->setValue(poseData->getTongueOutLimit() * 1000);

	toggleBlockAllUISignals(false);
}

void FaceSettingsWidget::updateStyledUIComponents()
{
	m_ui->faceSettingsScrollArea->setStyleSheet("QScrollArea {"
						    "background-color: transparent;"
						    "}"
						    "#trackingScrollAreaWidgetContents {"
						    "background-color: transparent;"
						    "}");
	m_ui->noConfigLabel->setStyleSheet("font-size: 20pt; padding-bottom: 40px;");
}

//  ------------------------------------------------- Private --------------------------------------------------

void FaceSettingsWidget::connectUISignalHandlers()
{
	QObject::connect(m_ui->eyesHalfOpenSlider, &QSlider::valueChanged, this,
			 [this](double value) { handleSliderMovement(PoseImage::EYESHALFOPEN, value); });
	QObject::connect(m_ui->eyesOpenSlider, &QSlider::valueChanged, this,
			 [this](double value) { handleSliderMovement(PoseImage::EYESOPEN, value); });
	QObject::connect(m_ui->mouthOpenSlider, &QSlider::valueChanged, this,
			 [this](double value) { handleSliderMovement(PoseImage::MOUTHOPEN, value); });
	QObject::connect(m_ui->tongueOutSlider, &QSlider::valueChanged, this,
			 [this](double value) { handleSliderMovement(PoseImage::TONGUEOUT, value); });

	QObject::connect(m_ui->eyesHalfOpenSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { handleSpinBoxChange(PoseImage::EYESHALFOPEN, value); });
	QObject::connect(m_ui->eyesOpenSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { handleSpinBoxChange(PoseImage::EYESOPEN, value); });
	QObject::connect(m_ui->mouthOpenSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { handleSpinBoxChange(PoseImage::MOUTHOPEN, value); });
	QObject::connect(m_ui->tongueOutSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { handleSpinBoxChange(PoseImage::TONGUEOUT, value); });
}

void FaceSettingsWidget::setupWidgetUI()
{
	m_ui->faceSettingsLabel->setText(obs_module_text("DialogFaceSettingsLabel"));

	m_ui->eyesClosedControlLabel->setText(obs_module_text("DialogEyesClosedLabel"));
	m_ui->eyesHalfOpenControlLabel->setText(obs_module_text("DialogEyesHalfOpenLabel"));
	m_ui->eyesHalfOpen2ControlLabel->setText(obs_module_text("DialogEyesHalfOpenLabel"));
	m_ui->eyesOpenControlLabel->setText(obs_module_text("DialogEyesOpenLabel"));
	m_ui->mouthClosedControlLabel->setText(obs_module_text("DialogMouthClosedLabel"));
	m_ui->mouthOpenControlLabel->setText(obs_module_text("DialogMouthOpenLabel"));
	m_ui->tongueOutControlLabel->setText(obs_module_text("DialogTongueInLabel"));
	m_ui->tongueInControlLabel->setText(obs_module_text("DialogTongueOutLabel"));

	m_ui->eyesClosedControlLabel->setToolTip(obs_module_text("DialogEyesClosedLabel"));
	m_ui->eyesHalfOpenControlLabel->setToolTip(obs_module_text("DialogEyesHalfOpenLabel"));
	m_ui->eyesHalfOpen2ControlLabel->setToolTip(obs_module_text("DialogEyesHalfOpenLabel"));
	m_ui->eyesOpenControlLabel->setToolTip(obs_module_text("DialogEyesOpenLabel"));
	m_ui->mouthClosedControlLabel->setToolTip(obs_module_text("DialogMouthClosedLabel"));
	m_ui->mouthOpenControlLabel->setToolTip(obs_module_text("DialogMouthOpenLabel"));
	m_ui->tongueOutControlLabel->setToolTip(obs_module_text("DialogTongueInLabel"));
	m_ui->tongueInControlLabel->setToolTip(obs_module_text("DialogTongueOutLabel"));

	m_ui->noConfigLabel->setText(obs_module_text("DialogNoConfigMessage"));

	// Initialize the mapping between PoseImage enums and QLineEdit pointers
	faceConfigDoubleSpinBoxes[PoseImage::EYESHALFOPEN] = m_ui->eyesHalfOpenSpinBox;
	faceConfigDoubleSpinBoxes[PoseImage::EYESOPEN] = m_ui->eyesOpenSpinBox;
	faceConfigDoubleSpinBoxes[PoseImage::MOUTHOPEN] = m_ui->mouthOpenSpinBox;
	faceConfigDoubleSpinBoxes[PoseImage::TONGUEOUT] = m_ui->tongueOutSpinBox;

	faceConfigSliders[PoseImage::EYESHALFOPEN] = m_ui->eyesHalfOpenSlider;
	faceConfigSliders[PoseImage::EYESOPEN] = m_ui->eyesOpenSlider;
	faceConfigSliders[PoseImage::MOUTHOPEN] = m_ui->mouthOpenSlider;
	faceConfigSliders[PoseImage::TONGUEOUT] = m_ui->tongueOutSlider;

	toggleVisible(false);

	updateStyledUIComponents();
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
		if (poseEnum == PoseImage::EYESHALFOPEN && (m_ui->eyesOpenSlider->value() < spinnerValue)) {
			m_ui->eyesOpenSlider->blockSignals(true);
			m_ui->eyesOpenSpinBox->blockSignals(true);

			m_ui->eyesOpenSlider->setValue(spinnerValue);
			m_ui->eyesOpenSpinBox->setValue(blendshapeLimit);

			m_ui->eyesOpenSlider->blockSignals(false);
			m_ui->eyesOpenSpinBox->blockSignals(false);
			emit blendshapeLimitChanged(PoseImage::EYESOPEN, blendshapeLimit);
		} else if (poseEnum == PoseImage::EYESOPEN && (m_ui->eyesHalfOpenSlider->value() > spinnerValue)) {
			m_ui->eyesHalfOpenSlider->blockSignals(true);
			m_ui->eyesHalfOpenSpinBox->blockSignals(true);

			m_ui->eyesHalfOpenSlider->setValue(spinnerValue);
			m_ui->eyesHalfOpenSpinBox->setValue(blendshapeLimit);

			m_ui->eyesHalfOpenSlider->blockSignals(false);
			m_ui->eyesHalfOpenSpinBox->blockSignals(false);
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
		if (poseEnum == PoseImage::EYESHALFOPEN && (m_ui->eyesOpenSlider->value() < spinBoxValue)) {
			m_ui->eyesOpenSlider->blockSignals(true);
			m_ui->eyesOpenSpinBox->blockSignals(true);

			m_ui->eyesOpenSlider->setValue(sliderValue);
			m_ui->eyesOpenSpinBox->setValue(spinBoxValue);

			m_ui->eyesOpenSlider->blockSignals(false);
			m_ui->eyesOpenSpinBox->blockSignals(false);
			emit blendshapeLimitChanged(PoseImage::EYESOPEN, spinBoxValue);
		} else if (poseEnum == PoseImage::EYESOPEN && (m_ui->eyesHalfOpenSlider->value() > spinBoxValue)) {
			m_ui->eyesHalfOpenSlider->blockSignals(true);
			m_ui->eyesHalfOpenSpinBox->blockSignals(true);

			m_ui->eyesHalfOpenSlider->setValue(sliderValue);
			m_ui->eyesHalfOpenSpinBox->setValue(spinBoxValue);

			m_ui->eyesHalfOpenSlider->blockSignals(false);
			m_ui->eyesHalfOpenSpinBox->blockSignals(false);
			emit blendshapeLimitChanged(PoseImage::EYESHALFOPEN, spinBoxValue);
		}
		slider->blockSignals(false);
	}
}
