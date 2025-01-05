#ifndef POSEIMAGEDATA_H
#define POSEIMAGEDATA_H

#include <QString>

class MovablePixmapItem; // Forward declaration

class PoseImageData {
public:
	PoseImageData(const QString &url = "", MovablePixmapItem *item = nullptr);
	~PoseImageData();

	QString getImageUrl() const;
	MovablePixmapItem *getPixmapItem() const;

	void setImageUrl(QString newUrl);
	void setPixmapItem(MovablePixmapItem *newItem);
	void setImagePosition(qreal x, qreal y);

	void clearPixmapItem();

private:
	QString m_imageUrl;
	MovablePixmapItem *m_pixmapItem;
};

#endif // POSEIMAGEDATA_H
