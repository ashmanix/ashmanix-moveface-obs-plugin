#ifndef FACESETTINGSWIDGET_H
#define FACESETTINGSWIDGET_H

#include <QWidget>
#include <QMap>
#include <QSharedPointer>
#include <QDoubleSpinBox>
#include <QSlider>

#include <obs-frontend-api.h>

#include "plugin-support.h"
#include "../../classes/poses/pose-image.h"
#include "../../classes/poses/pose.h"
#include "../../classes/tracking/tracker-data.h"
#include "../../ui/settings-dialog/ui_FaceSettingsWidget.h"

class FaceSettingsWidget : public QWidget {
	Q_OBJECT
public:
	explicit FaceSettingsWidget(QWidget *parent = nullptr, QSharedPointer<Pose> poseData = nullptr);
	~FaceSettingsWidget() override;

	void clearSelection();
	void toggleVisible(bool isVisible);
	void setData(QSharedPointer<Pose> poseData);
	void updateStyledUIComponents();

private:
	Ui::FaceSettingsWidget *m_ui;
	QMap<PoseImage, QDoubleSpinBox *> faceConfigDoubleSpinBoxes;
	QMap<PoseImage, QSlider *> faceConfigSliders;

	void connectUISignalHandlers();
	void setupWidgetUI();
	void toggleBlockAllUISignals(bool shouldBlock);

signals:
	void blendshapeLimitChanged(PoseImage poseEnum, double value);

private slots:
	void handleSliderMovement(PoseImage poseEnum, double value);
	void handleSpinBoxChange(PoseImage poseEnum, double value);
};

#endif // FACESETTINGSWIDGET_H
