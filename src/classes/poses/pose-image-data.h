#ifndef POSEIMAGEDATA_H
#define POSEIMAGEDATA_H

#include <QString>
#include "pixmap-item.h"

class PoseImageData {
public:
	PoseImageData(const QString &url = "", QSharedPointer<MovablePixmapItem> item = nullptr);

	// Copy Constructor for deep copying
	PoseImageData(const PoseImageData &other);

	// Copy Assignment Operator for deep copying
	PoseImageData &operator=(const PoseImageData &other);

	// Defaulted Move Constructor
	PoseImageData(PoseImageData &&other) noexcept = default;

	// Defaulted Move Assignment Operator
	PoseImageData &operator=(PoseImageData &&other) noexcept = default;

	// Defaulted Destructor
	~PoseImageData() = default;

	QString getImageUrl() const;
	QSharedPointer<MovablePixmapItem> getPixmapItem() const;

	void setImageUrl(QString newUrl);
	void setPixmapItem(QSharedPointer<MovablePixmapItem> newItem);
	void setImagePosition(qreal x, qreal y);

	void clearPixmapItem();

private:
	QString m_imageUrl = "";
	QSharedPointer<MovablePixmapItem> m_pixmapItem = nullptr;
};

#endif // POSEIMAGEDATA_H
