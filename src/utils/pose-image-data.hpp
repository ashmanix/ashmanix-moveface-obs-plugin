#ifndef POSEIMAGEDATA_H
#define POSEIMAGEDATA_H

#include <QString>
#include "../widgets/pixmap-item.hpp"

class PoseImageData {
public:
	PoseImageData(const QString &url = "", QSharedPointer<MovablePixmapItem> item = nullptr);

	// Copy Constructor for deep copying
	PoseImageData(const PoseImageData &other);

	// Copy Assignment Operator for deep copying
	PoseImageData &operator=(const PoseImageData &other);

	~PoseImageData();

	QString getImageUrl() const;
	QSharedPointer<MovablePixmapItem> getPixmapItem() const;

	void setImageUrl(QString newUrl);
	void setPixmapItem(QSharedPointer<MovablePixmapItem> newItem);
	void setImagePosition(qreal x, qreal y);

	void clearPixmapItem();

private:
	QString m_imageUrl;
	QSharedPointer<MovablePixmapItem> m_pixmapItem;
};

#endif // POSEIMAGEDATA_H
