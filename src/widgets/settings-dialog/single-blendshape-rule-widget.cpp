#include "single-blendshape-rule-widget.h"
#include <obs-module.h>

SingleBlendshapeRuleWidget::SingleBlendshapeRuleWidget(QWidget *parent, QSharedPointer<BlendshapeRule> bsRule)
	: QWidget(parent),
	  m_ui(new Ui::SingleBlendshapeRuleUI)
{
	m_ui->setupUi(this);
	m_blendshapeRule = bsRule;

	setupWidgetUI();

	if (m_blendshapeRule) {
		setData();
	} else {
		obs_log(LOG_ERROR, "No rule data found when creating single rule widget!");
	}

	connectUISignalHandlers();
}

SingleBlendshapeRuleWidget::~SingleBlendshapeRuleWidget() {}

QString SingleBlendshapeRuleWidget::getID() const
{
	return m_blendshapeRule->getID();
}

QSharedPointer<BlendshapeRule> SingleBlendshapeRuleWidget::getBlendshapeRule() const
{
	return m_blendshapeRule;
}

void SingleBlendshapeRuleWidget::clearSelection()
{
	m_ui->blendshapeNameComboBox->setCurrentIndex(0);
	m_ui->comparisonTypeComboBox->setCurrentIndex(0);
	m_ui->blendshapeValueSpinBox->setValue(0.0);
}

void SingleBlendshapeRuleWidget::setData(QSharedPointer<BlendshapeRule> bsRule)
{
	if (bsRule)
		m_blendshapeRule = bsRule;

	toggleBlockAllUISignals(true);

	m_ui->blendshapeNameComboBox->setCurrentIndex(static_cast<int>(m_blendshapeRule->getKey()));
	m_ui->comparisonTypeComboBox->setCurrentIndex(static_cast<int>(m_blendshapeRule->getComparisonType()));
	m_ui->blendshapeValueSpinBox->setValue(m_blendshapeRule->getCompareValue());

	toggleBlockAllUISignals(false);
}

void SingleBlendshapeRuleWidget::updateStyledUIComponents()
{
	QString baseUrl = obs_frontend_is_theme_dark() ? getDataFolderPath() + "/icons/dark/"
						       : getDataFolderPath() + "/icons/light/";
	QString trashIcon = QDir::fromNativeSeparators(baseUrl + "trash.svg");
	if (QFileInfo::exists(trashIcon)) {
		QIcon searchIcon(trashIcon);
		m_ui->deleteToolButton->setIcon(searchIcon);
	}
}

void SingleBlendshapeRuleWidget::toggleTrackerVisible(bool shouldShowTracking)
{
	m_ui->trackingValueLabel->setVisible(shouldShowTracking);
	m_shouldShowTracking = shouldShowTracking;
}

void SingleBlendshapeRuleWidget::updateTrackingValue(double value)
{
	if (!m_shouldShowTracking)
		toggleTrackerVisible(true);

	double clampedValue = qBound(0.0, value, 1.0);
	m_ui->trackingValueLabel->setText(QString("[%1]").arg(clampedValue, 0, 'f', 3));
}

//  ------------------------------------------------- Private --------------------------------------------------

void SingleBlendshapeRuleWidget::connectUISignalHandlers()
{
	QObject::connect(m_ui->blendshapeNameComboBox, &QComboBox::currentIndexChanged, this,
			 &SingleBlendshapeRuleWidget::handleBlendshapeSelection);
	QObject::connect(m_ui->comparisonTypeComboBox, &QComboBox::currentIndexChanged, this,
			 &SingleBlendshapeRuleWidget::handleComparisonTypeSelection);
	QObject::connect(m_ui->blendshapeValueSpinBox, &QDoubleSpinBox::valueChanged, this,
			 &SingleBlendshapeRuleWidget::handleValueChanged);
	QObject::connect(m_ui->deleteToolButton, &QToolButton::clicked, this,
			 [this]() { emit removeBlendshapeRule(m_blendshapeRule->getID()); });
}

void SingleBlendshapeRuleWidget::setupWidgetUI()
{
	m_ui->deleteToolButton->setToolTip(obs_module_text("DialogDeleteBlendshapeRuleTooltip"));

	for (auto i = 0; i < static_cast<int>(BlendshapeKey::COUNT) - 1; ++i) {
		m_ui->blendshapeNameComboBox->addItem(blendshapeKeyToString(static_cast<BlendshapeKey>(i)));
	}

	for (auto i = 0; i < static_cast<int>(ComparisonType::COUNT); ++i) {
		m_ui->comparisonTypeComboBox->addItem(
			BlendshapeRule::comparisonTypeToString(static_cast<ComparisonType>(i)));
	}

	toggleTrackerVisible(false);

	updateStyledUIComponents();
}

void SingleBlendshapeRuleWidget::toggleBlockAllUISignals(bool shouldBlock)
{
	m_ui->blendshapeNameComboBox->blockSignals(shouldBlock);
	m_ui->comparisonTypeComboBox->blockSignals(shouldBlock);
	m_ui->blendshapeValueSpinBox->blockSignals(shouldBlock);
}

//  ---------------------------------------------- Private Slots -----------------------------------------------

void SingleBlendshapeRuleWidget::handleBlendshapeSelection(int bsKeyIndex)
{
	BlendshapeKey bsKey = static_cast<BlendshapeKey>(bsKeyIndex);
	m_blendshapeRule->setKey(bsKey);
	emit change();
}

void SingleBlendshapeRuleWidget::handleComparisonTypeSelection(int compareKeyIndex)
{
	ComparisonType compareKey = static_cast<ComparisonType>(compareKeyIndex);
	m_blendshapeRule->setComparisonType(compareKey);
	emit change();
}

void SingleBlendshapeRuleWidget::handleValueChanged(double value)
{
	m_blendshapeRule->setCompareValue(value);
	emit change();
}
