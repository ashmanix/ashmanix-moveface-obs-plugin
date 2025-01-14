#ifndef IMAGEFILESWIDGET_H
#define IMAGEFILESWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QList>
#include <QLineEdit>

#include <obs-frontend-api.h>

#include "plugin-support.h"
#include "settings-widget-interface.h"
#include "../../utils/utils.h"
#include "../../classes/poses/pose.h"
#include "../../classes/poses/pose-image.h"
#include "../../classes/poses/pose-image-data.h"
#include "../../classes/tracking/tracker-data.h"
#include "../../ui/settings-dialog/ui_ImageFilesWidget.h"

class ImageFilesWidget : public QWidget, public SettingsWidgetInterface {
	Q_OBJECT
public:
	explicit ImageFilesWidget(QWidget *parent = nullptr, QSharedPointer<Pose> pose = nullptr);
	~ImageFilesWidget() override;

	void clearSelection() override;
	void toggleVisible(bool isVisible) override;
	QMap<PoseImage, QLineEdit *> getposeLineEditsMap() const;
	void updateStyledUIComponents() override;
	void setData(QSharedPointer<Pose> in_pose = nullptr) override;

private:
	Ui::ImageFilesWidget *m_ui = nullptr;
	QSharedPointer<Pose> m_pose = nullptr;
	QMap<PoseImage, QLineEdit *> m_poseImageLineEdits;

	void setupWidgetUI();
	void connectUISignalHandlers();

signals:
	void imageUrlSet(PoseImage poseEnum, QString filename);
	void imageUrlCleared(int imageIndex);
	void raiseWindow();

private slots:
	void handleImageUrlButtonClicked(PoseImage image);
	void handleClearImageUrlButtonClicked(PoseImage image);
};

#endif // IMAGEFILESWIDGET_H
