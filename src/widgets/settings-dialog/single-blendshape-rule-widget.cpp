#include "single-blendshape-rule-widget.h"
#include <obs-module.h>

SingleBlendshapeRuleWidget::SingleBlendshapeRuleWidget(QWidget *parent, QSharedPointer<BlendshapeRule> bsRule,
						       int index)
	: QWidget(parent),
	  ui(new Ui::SingleBlendshapeRuleUI),
	  m_index(index)
{
	ui->setupUi(this);

	setupWidgetUI();

	connectUISignalHandlers();

	if (bsRule)
		setData(bsRule);
}

SingleBlendshapeRuleWidget::~SingleBlendshapeRuleWidget() {}

void SingleBlendshapeRuleWidget::clearSelection()
{
	ui->blendshapeNameComboBox->setCurrentIndex(-1);
	ui->comparisonTypeComboBox->setCurrentIndex(-1);
	ui->blendshapeValueSpinBox->setValue(0.0);
}

void SingleBlendshapeRuleWidget::setData(QSharedPointer<BlendshapeRule> bsRule)
{
	toggleBlockAllUISignals(true);

	ui->blendshapeNameComboBox->setCurrentIndex(static_cast<int>(bsRule->key));
	ui->comparisonTypeComboBox->setCurrentIndex(static_cast<int>(bsRule->compareType));
	ui->blendshapeValueSpinBox->setValue(bsRule->compareValue);

	toggleBlockAllUISignals(false);
}

void SingleBlendshapeRuleWidget::updateStyledUIComponents()
{
	QString baseUrl = obs_frontend_is_theme_dark() ? getDataFolderPath() + "/icons/dark/"
						       : getDataFolderPath() + "/icons/light/";
	QString trashIcon = QDir::fromNativeSeparators(baseUrl + "trash.svg");
	if (QFileInfo::exists(trashIcon)) {
		QIcon searchIcon(trashIcon);
		ui->deleteToolButton->setIcon(searchIcon);
	}
}

//  ------------------------------------------------- Private --------------------------------------------------

void SingleBlendshapeRuleWidget::connectUISignalHandlers()
{
	QObject::connect(ui->blendshapeNameComboBox, &QComboBox::currentIndexChanged, this,
			 &SingleBlendshapeRuleWidget::handleBlendshapeSelection);
	QObject::connect(ui->comparisonTypeComboBox, &QComboBox::currentIndexChanged, this,
			 &SingleBlendshapeRuleWidget::handleComparisonTypeSelection);
	QObject::connect(ui->blendshapeValueSpinBox, &QDoubleSpinBox::valueChanged, this,
			 [this](double value) { emit blendshapeRuleValueChanged(value, m_index); });
	QObject::connect(ui->deleteToolButton, &QToolButton::clicked, this,
			 [this]() { emit removeBlendshapeRule(m_index); });
}

void SingleBlendshapeRuleWidget::setupWidgetUI()
{
	ui->deleteToolButton->setToolTip(obs_module_text("DialogDeleteBlendshapeRuleTooltip"));

	for (auto i = 0; i < static_cast<int>(BlendshapeKey::COUNT) - 1; ++i) {
		ui->blendshapeNameComboBox->addItem(blendshapeKeyToString(static_cast<BlendshapeKey>(i)));
	}

	for (auto i = 0; i < static_cast<int>(ComparisonType::COUNT); ++i) {
		ui->comparisonTypeComboBox->addItem(
			BlendshapeRule::comparisonTypeToString(static_cast<ComparisonType>(i)));
	}

	updateStyledUIComponents();
}

void SingleBlendshapeRuleWidget::toggleBlockAllUISignals(bool shouldBlock)
{
	ui->blendshapeNameComboBox->blockSignals(shouldBlock);
	ui->comparisonTypeComboBox->blockSignals(shouldBlock);
	ui->blendshapeValueSpinBox->blockSignals(shouldBlock);
}

//  ---------------------------------------------- Private Slots -----------------------------------------------

void SingleBlendshapeRuleWidget::handleBlendshapeSelection(int bsKeyIndex)
{
	BlendshapeKey bsKey = static_cast<BlendshapeKey>(bsKeyIndex);
	emit blendshapeKeyChanged(bsKey, m_index);
}

void SingleBlendshapeRuleWidget::handleComparisonTypeSelection(int compareKeyIndex)
{
	ComparisonType compareKey = static_cast<ComparisonType>(compareKeyIndex);
	emit blendshapeComparisonTypeChanged(compareKey, m_index);
}
