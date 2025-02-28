#ifndef SINGLEBLENDSHAPERULE_H
#define SINGLEBLENDSHAPERULE_H

#include <QWidget>
#include <QFileInfo>
#include <QDir>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSharedPointer>
#include <QString>
#include <QCompleter>

#include <obs-frontend-api.h>

#include "plugin-support.h"
#include "../../utils/utils.h"
#include "../../classes/blendshapes/blendshape-key.h"
#include "../../classes/blendshapes/blendshape-rule.h"
#include "../../ui/settings-dialog/ui_SingleBlendshapeRuleWidget.h"

class SingleBlendshapeRuleWidget : public QWidget {
	Q_OBJECT
public:
	explicit SingleBlendshapeRuleWidget(QWidget *parent, QSharedPointer<BlendshapeRule> bsRule);
	~SingleBlendshapeRuleWidget() override;

	void clearSelection();
	void setData(QSharedPointer<BlendshapeRule> bsRule = nullptr);
	void updateStyledUIComponents();

	QString getID() const;
	QSharedPointer<BlendshapeRule> getBlendshapeRule() const;

	void toggleTrackerVisible(bool shouldShowTracking);
	void updateTrackingValue(double value);

private:
	Ui::SingleBlendshapeRuleUI *m_ui = nullptr;
	QSharedPointer<BlendshapeRule> m_blendshapeRule = nullptr;
	bool m_shouldShowTracking = false;
	QStringList m_blendshapeList;
	QCompleter *m_blendshapeListCompleter;

	void connectUISignalHandlers();
	void setupWidgetUI();
	void toggleBlockAllUISignals(bool shouldBlock);

signals:
	void removeBlendshapeRule(QString id);
	void change();

private slots:
	void handleBlendshapeSelection(int bsKeyIndex);
	void handleComparisonTypeSelection(int compareKeyIndex);
	void handleValueChanged(double value);
};

#endif // SINGLEBLENDSHAPERULE_H
