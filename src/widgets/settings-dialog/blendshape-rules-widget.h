#ifndef BLENDSHAPERULESWIDGET_H
#define BLENDSHAPERULESWIDGET_H

#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QList>
#include <QSharedPointer>
#include <QToolButton>
#include <QFileInfo>
#include <QDir>

#include <obs-frontend-api.h>

#include "plugin-support.h"
#include "settings-widget-interface.h"
#include "single-blendshape-rule-widget.h"
#include "../../utils/utils.h"
#include "../../classes/poses/pose.h"
#include "../../classes/tracking/tracker-data.h"
#include "../../ui/settings-dialog/ui_BlendshapeRulesWidget.h"

class BlendshapeRulesWidget : public QWidget, public SettingsWidgetInterface {
	Q_OBJECT
public:
	explicit BlendshapeRulesWidget(QWidget *parent = nullptr, QSharedPointer<Pose> pose = nullptr);
	~BlendshapeRulesWidget();

	void clearSelection() override;
	void toggleVisible(bool isVisible) override;
	void setData(QSharedPointer<Pose> in_pose) override;
	void updateStyledUIComponents();

private:
	Ui::BlendshapeRulesWidget *m_ui;
	QMap<QString, SingleBlendshapeRuleWidget *> m_blendshapeRulesWidgetMap;
	QSharedPointer<Pose> m_pose;

	void connectUISignalHandlers();
	void connectBlendshapeRuleSignalHandlers(SingleBlendshapeRuleWidget *bsRuleWidget);
	void setupWidgetUI();
	void addBlendshapeRule(QSharedPointer<BlendshapeRule> in_bsRule);
	void addBlendshapeRule();

signals:
	void blendshapeRuleChanged();

private slots:
	void handleAddButtonClicked();
	void handleBlendshapeRuleDeleteButtonClicked(QString id);
};

#endif // BLENDSHAPERULESWIDGET_H
