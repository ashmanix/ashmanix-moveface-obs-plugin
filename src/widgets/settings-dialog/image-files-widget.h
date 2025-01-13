#ifndef IMAGEFILESWIDGET_H
#define IMAGEFILESWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QList>
#include <QLineEdit>

#include <obs-frontend-api.h>

#include "plugin-support.h"
#include "../../utils/utils.h"
#include "../../classes/poses/pose-image.h"
#include "../../classes/poses/pose-image-data.h"
#include "../../classes/tracking/tracker-data.h"
#include "../../ui/settings-dialog/ui_ImageFilesWidget.h"

class ImageFilesWidget : public QWidget {
	Q_OBJECT
public:
	explicit ImageFilesWidget(QWidget *parent = nullptr, QSharedPointer<TrackerData> tData = nullptr);
	~ImageFilesWidget() override;

	void clearSelection();
	void toggleVisible(bool isVisible);
	QMap<PoseImage, QLineEdit *> getposeLineEditsMap() const;
	void updateStyledUIComponents();

private:
	Ui::ImageFilesWidget *m_ui;
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
