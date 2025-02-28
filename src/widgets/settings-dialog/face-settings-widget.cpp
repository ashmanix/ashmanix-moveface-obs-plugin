#include "face-settings-widget.h"
#include <obs-module.h>

FaceSettingsWidget::FaceSettingsWidget(QWidget *parent, QSharedPointer<Pose> poseData)
	: QWidget(parent),
	  m_ui(new Ui::FaceSettingsWidget),
	  m_pose(poseData)
{
	m_ui->setupUi(this);

	setupWidgetUI();

	connectUISignalHandlers();

	if (m_pose)
		setData();
}

FaceSettingsWidget::~FaceSettingsWidget() = default;

void FaceSettingsWidget::clearSelection()
{
	toggleBlockAllUISignals(true);

	for (auto [key, value] : m_faceConfigDoubleSpinBoxes.asKeyValueRange()) {
		value->setValue(0);
	}

	for (auto [key, value] : m_faceConfigSliders.asKeyValueRange()) {
		value->setValue(0);
	}

	toggleBlockAllUISignals(false);
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

void FaceSettingsWidget::setData(QSharedPointer<Pose> in_pose)
{
	clearSelection();

	if (in_pose)
		m_pose = in_pose;

	if (!m_pose)
		return obs_log(LOG_WARNING, "No pose data found when loading face settings!");

	toggleBlockAllUISignals(true);

	m_ui->eyesHalfOpenSpinBox->setValue(m_pose->getEyesHalfOpenLimit());
	m_ui->eyesHalfOpenSlider->setValue(m_pose->getEyesHalfOpenLimit() * 1000);

	m_ui->eyesOpenSpinBox->setValue(m_pose->getEyesOpenLimit());
	m_ui->eyesOpenSlider->setValue(m_pose->getEyesOpenLimit() * 1000);

	m_ui->mouthOpenSpinBox->setValue(m_pose->getMouthOpenLimit());
	m_ui->mouthOpenSlider->setValue(m_pose->getMouthOpenLimit() * 1000);

	m_ui->smileSpinBox->setValue(m_pose->getSmileLimit());
	m_ui->smileSlider->setValue(m_pose->getSmileLimit() * 1000);

	m_ui->tongueOutSpinBox->setValue(m_pose->getTongueOutLimit());
	m_ui->tongueOutSlider->setValue(m_pose->getTongueOutLimit() * 1000);

	toggleBlockAllUISignals(false);
}

void FaceSettingsWidget::updateStyledUIComponents()
{
	QString baseUrl = obs_frontend_is_theme_dark() ? getDataFolderPath() + "/icons/dark/"
						       : getDataFolderPath() + "/icons/light/";

	QString eyeClosedIconPath = QDir::fromNativeSeparators(baseUrl + "eye-closed.svg");
	if (QFileInfo::exists(eyeClosedIconPath)) {
		QIcon eyeClosedIcon(eyeClosedIconPath);
		QPixmap eyeClosedIconPixmap = eyeClosedIcon.pixmap(16, 16);
		m_ui->eyesClosedControlLabel->setPixmap(eyeClosedIconPixmap);
	}

	QString eyeHalfOpenIconPath = QDir::fromNativeSeparators(baseUrl + "eye-half-open.svg");
	if (QFileInfo::exists(eyeHalfOpenIconPath)) {
		QIcon eyeHalfOpenIcon(eyeHalfOpenIconPath);
		QPixmap eyeHalfOpenIconPixmap = eyeHalfOpenIcon.pixmap(16, 16);
		m_ui->eyesHalfOpenControlLabel->setPixmap(eyeHalfOpenIconPixmap);
		m_ui->eyesHalfOpen2ControlLabel->setPixmap(eyeHalfOpenIconPixmap);
	}

	QString eyeOpenIconPath = QDir::fromNativeSeparators(baseUrl + "eye-open.svg");
	if (QFileInfo::exists(eyeOpenIconPath)) {
		QIcon eyeOpenIcon(eyeOpenIconPath);
		QPixmap eyeOpenIconPixmap = eyeOpenIcon.pixmap(16, 16);
		m_ui->eyesOpenControlLabel->setPixmap(eyeOpenIconPixmap);
	}

	QString mouthOpenIconPath = QDir::fromNativeSeparators(baseUrl + "mouth-open.svg");
	if (QFileInfo::exists(mouthOpenIconPath)) {
		QIcon mouthOpenIcon(mouthOpenIconPath);
		QPixmap mouthOpenIconPixmap = mouthOpenIcon.pixmap(16, 16);
		m_ui->mouthOpenControlLabel->setPixmap(mouthOpenIconPixmap);
	}

	QString mouthClosedIconPath = QDir::fromNativeSeparators(baseUrl + "mouth-closed.svg");
	if (QFileInfo::exists(mouthClosedIconPath)) {
		QIcon mouthClosedIcon(mouthClosedIconPath);
		QPixmap mouthClosedIconPixmap = mouthClosedIcon.pixmap(16, 16);
		m_ui->mouthClosedControlLabel->setPixmap(mouthClosedIconPixmap);
		m_ui->noSmileControlLabel->setPixmap(mouthClosedIconPixmap);
		m_ui->tongueInControlLabel->setPixmap(mouthClosedIconPixmap);
	}

	QString smileIconPath = QDir::fromNativeSeparators(baseUrl + "smile.svg");
	if (QFileInfo::exists(smileIconPath)) {
		QIcon smileIcon(smileIconPath);
		QPixmap smileIconPixmap = smileIcon.pixmap(16, 16);
		m_ui->smileControlLabel->setPixmap(smileIconPixmap);
	}

	QString tongueOutIconPath = QDir::fromNativeSeparators(baseUrl + "tongue-out.svg");
	if (QFileInfo::exists(tongueOutIconPath)) {
		QIcon tongueOutIcon(tongueOutIconPath);
		QPixmap tongueOutIconPixmap = tongueOutIcon.pixmap(16, 16);
		m_ui->tongueOutControlLabel->setPixmap(tongueOutIconPixmap);
	}
}

void FaceSettingsWidget::toggleShowTracking(bool shouldShowTracking)
{
	m_ui->eyesHalfOpenSlider->setShowTracking(shouldShowTracking);
	m_ui->eyesOpenSlider->setShowTracking(shouldShowTracking);
	m_ui->mouthOpenSlider->setShowTracking(shouldShowTracking);
	m_ui->smileSlider->setShowTracking(shouldShowTracking);
	m_ui->tongueOutSlider->setShowTracking(shouldShowTracking);

	m_ui->eyesHalfOpenTrackingLabel->setVisible(shouldShowTracking);
	m_ui->eyesOpenTrackingLabel->setVisible(shouldShowTracking);
	m_ui->mouthOpenTrackingLabel->setVisible(shouldShowTracking);
	m_ui->smileTrackingLabel->setVisible(shouldShowTracking);
	m_ui->tongueOutTrackingLabel->setVisible(shouldShowTracking);

	m_shouldShowTracking = shouldShowTracking;
}

void FaceSettingsWidget::handleTrackingValueChange(VTubeStudioData data)
{
	if (!m_shouldShowTracking)
		toggleShowTracking(true);

	QMap<BlendshapeKey, Blendshape> blendshapeMap = data.getBlendshapes();
	double eyeBlink = blendshapeMap.value(BlendshapeKey::EYEBLINK_L).m_value;
	double mouthOpen = blendshapeMap.value(BlendshapeKey::JAWOPEN).m_value;
	double mouthSmile = blendshapeMap.value(BlendshapeKey::MOUTHSMILE_L).m_value;
	double tongueOut = blendshapeMap.value(BlendshapeKey::TONGUEOUT).m_value;

	m_ui->eyesHalfOpenTrackingLabel->setText(formatTrackingValueLabel(eyeBlink));
	m_ui->eyesOpenTrackingLabel->setText(formatTrackingValueLabel(eyeBlink));
	m_ui->mouthOpenTrackingLabel->setText(formatTrackingValueLabel(mouthOpen));
	m_ui->smileTrackingLabel->setText(formatTrackingValueLabel(mouthSmile));
	m_ui->tongueOutTrackingLabel->setText(formatTrackingValueLabel(tongueOut));

	m_ui->eyesHalfOpenSlider->setTrackingValue(formatTrackingValue(eyeBlink));
	m_ui->eyesOpenSlider->setTrackingValue(formatTrackingValue(eyeBlink));
	m_ui->mouthOpenSlider->setTrackingValue(formatTrackingValue(mouthOpen));
	m_ui->smileSlider->setTrackingValue(formatTrackingValue(mouthSmile));
	m_ui->tongueOutSlider->setTrackingValue(formatTrackingValue(tongueOut));
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
	QObject::connect(m_ui->smileSlider, &QSlider::valueChanged, this,
			 [this](double value) { handleSliderMovement(PoseImage::MOUTHSMILE, value); });
	QObject::connect(m_ui->tongueOutSlider, &QSlider::valueChanged, this,
			 [this](double value) { handleSliderMovement(PoseImage::TONGUEOUT, value); });

	QObject::connect(m_ui->eyesHalfOpenSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { handleSpinBoxChange(PoseImage::EYESHALFOPEN, value); });
	QObject::connect(m_ui->eyesOpenSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { handleSpinBoxChange(PoseImage::EYESOPEN, value); });
	QObject::connect(m_ui->mouthOpenSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { handleSpinBoxChange(PoseImage::MOUTHOPEN, value); });
	QObject::connect(m_ui->smileSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { handleSpinBoxChange(PoseImage::MOUTHSMILE, value); });
	QObject::connect(m_ui->tongueOutSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { handleSpinBoxChange(PoseImage::TONGUEOUT, value); });
}

void FaceSettingsWidget::setupWidgetUI()
{
	m_ui->titleLabel->setText(obs_module_text("DialogFaceSettingsLabel"));
	m_ui->titleLabel->setToolTip(obs_module_text("DialogFaceSettingsLabelToolTip"));

	m_ui->eyesClosedControlLabel->setText(obs_module_text("DialogEyesClosedLabel"));
	m_ui->eyesHalfOpenControlLabel->setText(obs_module_text("DialogEyesHalfOpenLabel"));
	m_ui->eyesHalfOpen2ControlLabel->setText(obs_module_text("DialogEyesHalfOpenLabel"));
	m_ui->eyesOpenControlLabel->setText(obs_module_text("DialogEyesOpenLabel"));
	m_ui->mouthClosedControlLabel->setText(obs_module_text("DialogMouthClosedLabel"));
	m_ui->mouthOpenControlLabel->setText(obs_module_text("DialogMouthOpenLabel"));
	m_ui->noSmileControlLabel->setText(obs_module_text("DialogNoSmileLabel"));
	m_ui->smileControlLabel->setText(obs_module_text("DialogSmileLabel"));
	m_ui->tongueOutControlLabel->setText(obs_module_text("DialogTongueInLabel"));
	m_ui->tongueInControlLabel->setText(obs_module_text("DialogTongueOutLabel"));

	m_ui->eyesClosedControlLabel->setToolTip(obs_module_text("DialogEyesClosedLabel"));
	m_ui->eyesHalfOpenControlLabel->setToolTip(obs_module_text("DialogEyesHalfOpenLabel"));
	m_ui->eyesHalfOpen2ControlLabel->setToolTip(obs_module_text("DialogEyesHalfOpenLabel"));
	m_ui->eyesOpenControlLabel->setToolTip(obs_module_text("DialogEyesOpenLabel"));
	m_ui->mouthClosedControlLabel->setToolTip(obs_module_text("DialogMouthClosedLabel"));
	m_ui->mouthOpenControlLabel->setToolTip(obs_module_text("DialogMouthOpenLabel"));
	m_ui->noSmileControlLabel->setToolTip(obs_module_text("DialogNoSmileLabel"));
	m_ui->smileControlLabel->setToolTip(obs_module_text("DialogSmileLabel"));
	m_ui->tongueOutControlLabel->setToolTip(obs_module_text("DialogTongueOutLabel"));
	m_ui->tongueInControlLabel->setToolTip(obs_module_text("DialogTongueInLabel"));

	m_ui->noConfigLabel->setText(obs_module_text("DialogNoConfigMessage"));

	// Initialize the mapping between PoseImage enums and QLineEdit pointers
	m_faceConfigDoubleSpinBoxes[PoseImage::EYESHALFOPEN] = m_ui->eyesHalfOpenSpinBox;
	m_faceConfigDoubleSpinBoxes[PoseImage::EYESOPEN] = m_ui->eyesOpenSpinBox;
	m_faceConfigDoubleSpinBoxes[PoseImage::MOUTHOPEN] = m_ui->mouthOpenSpinBox;
	m_faceConfigDoubleSpinBoxes[PoseImage::MOUTHSMILE] = m_ui->smileSpinBox;
	m_faceConfigDoubleSpinBoxes[PoseImage::TONGUEOUT] = m_ui->tongueOutSpinBox;

	m_faceConfigSliders[PoseImage::EYESHALFOPEN] = m_ui->eyesHalfOpenSlider;
	m_faceConfigSliders[PoseImage::EYESOPEN] = m_ui->eyesOpenSlider;
	m_faceConfigSliders[PoseImage::MOUTHOPEN] = m_ui->mouthOpenSlider;
	m_faceConfigSliders[PoseImage::MOUTHSMILE] = m_ui->smileSlider;
	m_faceConfigSliders[PoseImage::TONGUEOUT] = m_ui->tongueOutSlider;

	toggleVisible(false);

	toggleShowTracking(m_shouldShowTracking);

	updateStyledUIComponents();
}

void FaceSettingsWidget::toggleBlockAllUISignals(bool shouldBlock)
{
	for (auto [key, value] : m_faceConfigDoubleSpinBoxes.asKeyValueRange()) {
		value->blockSignals(shouldBlock);
	}

	for (auto [key, value] : m_faceConfigSliders.asKeyValueRange()) {
		value->blockSignals(shouldBlock);
	}
}

void FaceSettingsWidget::handleBlendshapelimitChange(PoseImage poseEnum, double value)
{
	if (!m_pose)
		return;

	switch (poseEnum) {
	case PoseImage::EYESHALFOPEN:
		m_pose->setEyesHalfOpenLimit(value);
		emit blendshapeLimitChanged();
		break;
	case PoseImage::EYESOPEN:
		m_pose->setEyesOpenLimit(value);
		emit blendshapeLimitChanged();
		break;
	case PoseImage::MOUTHOPEN:
		m_pose->setMouthOpenLimit(value);
		emit blendshapeLimitChanged();
		break;
	case PoseImage::MOUTHSMILE:
		m_pose->setSmileLimit(value);
		emit blendshapeLimitChanged();
		break;
	case PoseImage::TONGUEOUT:
		m_pose->setTongueOutLimit(value);
		emit blendshapeLimitChanged();
		break;

	default:
		break;
	}
}

QString FaceSettingsWidget::formatTrackingValueLabel(double value = 0.0)
{
	double clampedValue = qBound(0.0, value, 1.0);
	return QString("[%1]").arg(clampedValue, 0, 'f', 3);
}

int FaceSettingsWidget::formatTrackingValue(double value = 0.0)
{
	return qBound(0, qRound(value * 1000), 1000);
}

//  ---------------------------------------------- Private Slots -----------------------------------------------

void FaceSettingsWidget::handleSliderMovement(PoseImage poseEnum, double spinnerValue)
{
	// Slider value runs between 0 - 1000 so spinbox value is spinnerValue/1000
	double blendshapeLimit = spinnerValue / 1000;

	if (auto it = m_faceConfigDoubleSpinBoxes.find(poseEnum); it != m_faceConfigDoubleSpinBoxes.end()) {
		QDoubleSpinBox *dSpinBox = it.value(); // For QMap
		dSpinBox->blockSignals(true);
		if (dSpinBox) {
			dSpinBox->setValue(spinnerValue / 1000);
			handleBlendshapelimitChange(poseEnum, blendshapeLimit);
		}
		// Eyes open value cannot be less than eyes half open value
		if (poseEnum == PoseImage::EYESHALFOPEN && (m_ui->eyesOpenSlider->value() < spinnerValue)) {
			m_ui->eyesOpenSlider->blockSignals(true);
			m_ui->eyesOpenSpinBox->blockSignals(true);

			m_ui->eyesOpenSlider->setValue(spinnerValue);
			m_ui->eyesOpenSpinBox->setValue(blendshapeLimit);

			m_ui->eyesOpenSlider->blockSignals(false);
			m_ui->eyesOpenSpinBox->blockSignals(false);
			handleBlendshapelimitChange(PoseImage::EYESOPEN, blendshapeLimit);
		} else if (poseEnum == PoseImage::EYESOPEN && (m_ui->eyesHalfOpenSlider->value() > spinnerValue)) {
			m_ui->eyesHalfOpenSlider->blockSignals(true);
			m_ui->eyesHalfOpenSpinBox->blockSignals(true);

			m_ui->eyesHalfOpenSlider->setValue(spinnerValue);
			m_ui->eyesHalfOpenSpinBox->setValue(blendshapeLimit);

			m_ui->eyesHalfOpenSlider->blockSignals(false);
			m_ui->eyesHalfOpenSpinBox->blockSignals(false);
			handleBlendshapelimitChange(PoseImage::EYESHALFOPEN, blendshapeLimit);
		}
		dSpinBox->blockSignals(false);
	}
}

void FaceSettingsWidget::handleSpinBoxChange(PoseImage poseEnum, double spinBoxValue)
{
	// Slider value runs between 0 - 1000 so slider value is spinbox value * 1000
	double sliderValue = spinBoxValue * 1000;

	if (auto it = m_faceConfigSliders.find(poseEnum); it != m_faceConfigSliders.end()) {
		QSlider *slider = it.value(); // For QMap
		slider->blockSignals(true);
		if (slider) {
			slider->setValue(sliderValue);
			handleBlendshapelimitChange(poseEnum, spinBoxValue);
		}
		// Eyes open value cannot be less than eyes half open value
		if (poseEnum == PoseImage::EYESHALFOPEN && (m_ui->eyesOpenSpinBox->value() < spinBoxValue)) {
			m_ui->eyesOpenSlider->blockSignals(true);
			m_ui->eyesOpenSpinBox->blockSignals(true);

			m_ui->eyesOpenSlider->setValue(sliderValue);
			m_ui->eyesOpenSpinBox->setValue(spinBoxValue);

			m_ui->eyesOpenSlider->blockSignals(false);
			m_ui->eyesOpenSpinBox->blockSignals(false);
			handleBlendshapelimitChange(PoseImage::EYESOPEN, spinBoxValue);
		} else if (poseEnum == PoseImage::EYESOPEN && (m_ui->eyesHalfOpenSpinBox->value() > spinBoxValue)) {
			m_ui->eyesHalfOpenSlider->blockSignals(true);
			m_ui->eyesHalfOpenSpinBox->blockSignals(true);

			m_ui->eyesHalfOpenSlider->setValue(sliderValue);
			m_ui->eyesHalfOpenSpinBox->setValue(spinBoxValue);

			m_ui->eyesHalfOpenSlider->blockSignals(false);
			m_ui->eyesHalfOpenSpinBox->blockSignals(false);
			handleBlendshapelimitChange(PoseImage::EYESHALFOPEN, spinBoxValue);
		}
		slider->blockSignals(false);
	}
}
