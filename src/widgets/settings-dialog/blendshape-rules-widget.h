
#ifndef BLENDSHAPERULESWIDGET_H
#define BLENDSHAPERULESWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QList>
#include <QSharedPointer>

#include <obs-frontend-api.h>

#include "plugin-support.h"
#include "../../classes/poses/pose.h"
#include "../../classes/tracking/tracker-data.h"
#include "../../ui/settings-dialog/ui_BlendshapeRulesWidget.h"

class BlendshapeRulesWidget : public QWidget {
	Q_OBJECT
public:
	explicit BlendshapeRulesWidget(QWidget *parent = nullptr, QSharedPointer<Pose> poseData = nullptr);
	~BlendshapeRulesWidget() override;

	void clearSelection();
	void toggleVisible(bool isVisible);
	void setData(QSharedPointer<Pose> poseData);

private:
	Ui::BlendshapeRulesWidget *ui;

	void connectUISignalHandlers();
	void setupWidgetUI();
	void updateStyledUIComponents();
	void toggleBlockAllUISignals(bool shouldBlock);

signals:
	void blendshapeRuleChanged(double value);

private slots:
	void handleAddButtonClicked();
	void handleRemoveButtonClicked(int index);
};

#endif // BLENDSHAPERULESWIDGET_H
