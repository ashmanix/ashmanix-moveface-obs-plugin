#ifndef FACESETTINGSWIDGET_H
#define FACESETTINGSWIDGET_H

#include <QWidget>
#include <QMap>
#include <QSharedPointer>
#include <QDoubleSpinBox>
#include <QSlider>

#include <obs-frontend-api.h>

#include "plugin-support.h"
#include "settings-widget-interface.h"
#include "../../classes/poses/pose-image.h"
#include "../../classes/poses/pose.h"
#include "../../classes/tracking/tracker-data.h"
#include "../../ui/settings-dialog/ui_FaceSettingsWidget.h"

class FaceSettingsWidget : public QWidget, public SettingsWidgetInterface {
	Q_OBJECT
public:
	explicit FaceSettingsWidget(QWidget *parent, QSharedPointer<Pose> poseData = nullptr);
	~FaceSettingsWidget() override;

	void clearSelection() override;
	void toggleVisible(bool isVisible) override;
	void setData(QSharedPointer<Pose> poseData = nullptr) override;
	void updateStyledUIComponents() override {};

private:
	Ui::FaceSettingsWidget *m_ui;
	QSharedPointer<Pose> m_pose;
	QMap<PoseImage, QDoubleSpinBox *> m_faceConfigDoubleSpinBoxes;
	QMap<PoseImage, QSlider *> m_faceConfigSliders;
	bool m_isDataLoading = false;

	void connectUISignalHandlers();
	void setupWidgetUI();
	void toggleBlockAllUISignals(bool shouldBlock);
	void handleBlendshapelimitChange(PoseImage poseEnum, double value);

signals:
	void blendshapeLimitChanged();

private slots:
	void handleSliderMovement(PoseImage poseEnum, double value);
	void handleSpinBoxChange(PoseImage poseEnum, double value);
};

#endif // FACESETTINGSWIDGET_H
