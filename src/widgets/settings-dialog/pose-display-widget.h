#ifndef POSEDISPLAYWIDGET_H
#define POSEDISPLAYWIDGET_H

#include <QWidget>
#include <QWidget>
#include <QDir>
#include <QSharedPointer>

#include <obs-frontend-api.h>

#include "plugin-support.h"
#include "settings-widget-interface.h"
#include "../../classes/poses/pose-image.h"
#include "../../classes/poses/pose.h"
#include "../../ui/settings-dialog/ui_PoseImageView.h"

class PoseDisplayWidget : public QWidget, public SettingsWidgetInterface {
	Q_OBJECT
public:
	explicit PoseDisplayWidget(QWidget *parent = nullptr, QSharedPointer<Pose> pose = nullptr);
	~PoseDisplayWidget() override;

	void clearSelection() override;
	void toggleVisible(bool isVisible) final { UNUSED_PARAMETER(isVisible); };
	void setData(QSharedPointer<Pose> poseData = nullptr) override;
	void updateStyledUIComponents() override;
	void addImageToScene(PoseImageData *imageData, bool useImagePos = false, bool clearScene = false);

private:
	Ui::PoseImageDisplayWidget *m_ui;
	QSharedPointer<QGraphicsScene> m_avatarPreviewScene = nullptr;
	QSharedPointer<Pose> m_pose;
	const double zoomScaleFactor = 1.15;

	void connectUISignalHandlers();
	void setupWidgetUI();
	void centerSceneOnItems();

signals:
	void poseImagesChanged();

public slots:
	void handleSetImageUrl(PoseImage poseEnum, QString fileName);
	void handleClearImageUrl(int imageIndex);

private slots:
	void setImagesFromPose(PoseImage poseEnum, QString fileName);
	void clearAllImages(PoseImage poseEnum);
	void handleCenterViewButtonClick();
	void handleMoveImageUpClick();
	void handleMoveImageDownClick();
	void handleImageZoomClick(bool isZoomOut);
	void handleImageMove(qreal x, qreal y, qreal z, PoseImage pImageType);
};

#endif // POSEDISPLAYWIDGET_H
