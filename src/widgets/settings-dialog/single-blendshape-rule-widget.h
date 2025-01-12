#ifndef SINGLEBLENDSHAPERULE_H
#define SINGLEBLENDSHAPERULE_H

#include <QWidget>
#include <QFileInfo>
#include <QDir>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSharedPointer>

#include <obs-frontend-api.h>

#include "plugin-support.h"
#include "../../utils/utils.h"
#include "../../classes/blendshapes/blendshape-key.h"
#include "../../classes/blendshapes/blendshape-rule.h"
#include "../../ui/settings-dialog/ui_SingleBlendshapeRuleWidget.h"

class SingleBlendshapeRuleWidget : public QWidget {
	Q_OBJECT
public:
	explicit SingleBlendshapeRuleWidget(QWidget *parent = nullptr, QSharedPointer<BlendshapeRule> bsRule = nullptr,
					    int index = 0);
	~SingleBlendshapeRuleWidget() override;

	void clearSelection();
	void setData(QSharedPointer<BlendshapeRule> bsRule);
	void updateStyledUIComponents();

private:
	Ui::SingleBlendshapeRuleUI *ui;
	int m_index = 0;

	void connectUISignalHandlers();
	void setupWidgetUI();
	void toggleBlockAllUISignals(bool shouldBlock);

signals:
	void blendshapeKeyChanged(BlendshapeKey bsKey, int index);
	void blendshapeComparisonTypeChanged(ComparisonType compType, int index);
	void blendshapeRuleValueChanged(double value, int index);
	void removeBlendshapeRule(int index);

private slots:
	void handleBlendshapeSelection(int bsKeyIndex);
	void handleComparisonTypeSelection(int compareKeyIndex);
};

#endif // SINGLEBLENDSHAPERULE_H
