#ifndef POSEDISPLAYWIDGET_H
#define POSEDISPLAYWIDGET_H

#include <QWidget>
#include <QSharedPointer>

#include <obs-frontend-api.h>

#include "plugin-support.h"
#include "settings-widget-interface.h"
#include "../../classes/poses/pose-image.h"
#include "../../classes/poses/pose.h"
#include "../../ui/settings-dialog/ui_PoseImageWidgetUI.h"

class PoseDisplayWidget : public QWidget, public SettingsWidgetInterface {
	Q_OBJECT
public:
	explicit PoseDisplayWidget *parent, QSharedPointer<Pose> poseData = nullptr);
	~PoseDisplayWidget override;

	void clearSelection() override;
	void toggleVisible(bool isVisible) override;
	void setData(QSharedPointer<Pose> poseData = nullptr) override;
	void updateStyledUIComponents();

private:
	Ui::FaceSettingsWidget *m_ui;
	QSharedPointer<Pose> m_pose;

	void connectUISignalHandlers();
	void setupWidgetUI();
	void toggleBlockAllUISignals(bool shouldBlock);
	void imageMoved(PoseImage poseEnum, double value);

signals:
	void poseImagesChanged();

private slots:
	void imageMoved();
};

#endif // POSEDISPLAYWIDGET_H
