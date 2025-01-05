#include "pose-image-data.hpp"
#include "../widgets/pixmap-item.hpp"

PoseImageData::PoseImageData(const QString &url, MovablePixmapItem *item)
{
	m_imageUrl = url;
	m_pixmapItem = item;
};

PoseImageData::~PoseImageData() {}

QString PoseImageData::getImageUrl() const
{
	return m_imageUrl;
}

MovablePixmapItem *PoseImageData::getPixmapItem() const
{
	return m_pixmapItem;
}

void PoseImageData::setImageUrl(QString newUrl)
{
	m_imageUrl = newUrl;
}

void PoseImageData::setPixmapItem(MovablePixmapItem *newItem)
{
	if (m_pixmapItem) {
		delete m_pixmapItem;
		m_pixmapItem = nullptr;
	}
	m_pixmapItem = newItem;
}

void PoseImageData::setImagePosition(qreal x, qreal y)
{
	m_pixmapItem->setPos(x, y);
}

void PoseImageData::clearPixmapItem()
{
	if (m_pixmapItem) {
		delete m_pixmapItem;
		m_pixmapItem = nullptr;
	}
}
