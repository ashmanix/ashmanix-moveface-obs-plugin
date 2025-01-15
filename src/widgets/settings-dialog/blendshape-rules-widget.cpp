#include "blendshape-rules-widget.h"
#include <obs-module.h>

BlendshapeRulesWidget::BlendshapeRulesWidget(QWidget *parent, QSharedPointer<Pose> pose)
	: QWidget(parent),
	  m_ui(new Ui::BlendshapeRulesWidget),
	  m_pose(pose)
{
	m_ui->setupUi(this);

	if (m_pose)
		setData(m_pose);

	setupWidgetUI();

	connectUISignalHandlers();
}

BlendshapeRulesWidget::~BlendshapeRulesWidget()
{
	delete m_ui;
}

void BlendshapeRulesWidget::clearSelection()
{
	blockSignals(true);
	// Clear widget list
	for (auto i = m_blendshapeRulesWidgetMap.begin(), end = m_blendshapeRulesWidgetMap.end(); i != end; i++) {
		auto rule = *i;
		rule->deleteLater();
	}

	m_blendshapeRulesWidgetMap.clear();
	blockSignals(false);
}

void BlendshapeRulesWidget::toggleVisible(bool isVisible)
{
	if (isVisible) {
		m_ui->blenshapeRulesConfigWidget->setVisible(true);
		m_ui->noConfigLabel->setVisible(false);
	} else {
		m_ui->blenshapeRulesConfigWidget->setVisible(false);
		m_ui->noConfigLabel->setVisible(true);
	}
}

void BlendshapeRulesWidget::setData(QSharedPointer<Pose> in_pose)
{
	clearSelection();
	m_pose = in_pose;
	if (!m_pose)
		return obs_log(LOG_WARNING, "No pose data found when loading blendshape rules!");

	blockSignals(true);
	QMap<QString, QSharedPointer<BlendshapeRule>> *bsList = m_pose->getBlendshapeList();
	// list rules
	if (!bsList->isEmpty()) {
		for (auto i = bsList->begin(), end = bsList->end(); i != end; i++) {
			auto rule = *i;
			addBlendshapeRule(rule);
		}
	}
	blockSignals(false);
}

void BlendshapeRulesWidget::updateStyledUIComponents()
{
	QString baseUrl = obs_frontend_is_theme_dark() ? getDataFolderPath() + "/icons/dark/"
						       : getDataFolderPath() + "/icons/light/";
	QString plusIconUrl = QDir::fromNativeSeparators(baseUrl + "plus.svg");
	if (QFileInfo::exists(plusIconUrl)) {
		QIcon plusIcon(plusIconUrl);
		m_ui->addBlenshapeRuleToolButton->setIcon(plusIcon);
	}
}

//  ------------------------------------------------- Private --------------------------------------------------

void BlendshapeRulesWidget::connectUISignalHandlers()
{
	QObject::connect(m_ui->addBlenshapeRuleToolButton, &QToolButton::clicked, this,
			 &BlendshapeRulesWidget::handleAddButtonClicked);
}

void BlendshapeRulesWidget::connectBlendshapeRuleSignalHandlers(SingleBlendshapeRuleWidget *bsRuleWidget)
{
	QObject::connect(bsRuleWidget, &SingleBlendshapeRuleWidget::change, this,
			 [this]() { emit blendshapeRuleChanged(); });
	QObject::connect(bsRuleWidget, &SingleBlendshapeRuleWidget::removeBlendshapeRule, this,
			 &BlendshapeRulesWidget::handleBlendshapeRuleDeleteButtonClicked);
}

void BlendshapeRulesWidget::setupWidgetUI()
{
	m_ui->titleLabel->setText(obs_module_text("DialogBlendshapeRulesLabel"));
	m_ui->titleLabel->setToolTip(obs_module_text("DialogBlendshapeRulesLabelToolTip"));

	m_ui->addBlenshapeRuleToolButton->setToolTip(obs_module_text("DialogAddBlendshapeRuleTooltip"));

	m_ui->noConfigLabel->setText(obs_module_text("DialogNoConfigMessage"));

	toggleVisible(false);

	updateStyledUIComponents();
}

void BlendshapeRulesWidget::addBlendshapeRule(QSharedPointer<BlendshapeRule> in_bsRule)
{
	if (in_bsRule) {
		auto newBlendshapeRulesWidget = new SingleBlendshapeRuleWidget(this, in_bsRule);
		m_blendshapeRulesWidgetMap.insert(newBlendshapeRulesWidget->getID(), newBlendshapeRulesWidget);
		connectBlendshapeRuleSignalHandlers(newBlendshapeRulesWidget);

		m_ui->blendshapeRulesVerticalLayout->addWidget(newBlendshapeRulesWidget);
		emit blendshapeRuleChanged();
	}
}

void BlendshapeRulesWidget::addBlendshapeRule()
{
	auto newBsRule = QSharedPointer<BlendshapeRule>::create();
	if (newBsRule) {
		m_pose->addBlendShapeRule(newBsRule);

		auto newBlendshapeRulesWidget = new SingleBlendshapeRuleWidget(this, newBsRule);
		m_blendshapeRulesWidgetMap.insert(newBlendshapeRulesWidget->getID(), newBlendshapeRulesWidget);
		connectBlendshapeRuleSignalHandlers(newBlendshapeRulesWidget);

		m_ui->blendshapeRulesVerticalLayout->addWidget(newBlendshapeRulesWidget);
		emit blendshapeRuleChanged();
	}
}

//  ---------------------------------------------- Private Slots -----------------------------------------------

void BlendshapeRulesWidget::handleAddButtonClicked()
{
	addBlendshapeRule();
}

void BlendshapeRulesWidget::handleBlendshapeRuleDeleteButtonClicked(QString id)
{
	obs_log(LOG_INFO, "Trying to delete rule: %s", id.toStdString().c_str());
	SingleBlendshapeRuleWidget *itemToBeRemoved = m_blendshapeRulesWidgetMap.value(id, nullptr);

	if (itemToBeRemoved) {
		m_blendshapeRulesWidgetMap.remove(id);
		m_pose->removeBlendShapeRule(id);
		itemToBeRemoved->deleteLater();

		emit blendshapeRuleChanged();
		obs_log(LOG_INFO, (QString("Blendshape rule %1 deleted").arg(id)).toStdString().c_str());
	}
}
