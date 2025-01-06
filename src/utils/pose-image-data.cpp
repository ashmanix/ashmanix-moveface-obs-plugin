#include "pose-image-data.hpp"
#include "../widgets/pixmap-item.hpp"

PoseImageData::PoseImageData(const QString &url, QSharedPointer<MovablePixmapItem> item)
	: m_imageUrl(url),
	  m_pixmapItem(item)
{
}

PoseImageData::PoseImageData(const PoseImageData &other)
	: m_imageUrl(other.m_imageUrl),
	  m_pixmapItem(other.getPixmapItem() ? other.m_pixmapItem->clone() : nullptr)
{
}

PoseImageData::~PoseImageData() {}

PoseImageData &PoseImageData::operator=(const PoseImageData &other)
{
	if (this == &other)
		return *this;

	m_imageUrl = other.m_imageUrl;

	if (other.m_pixmapItem) {
		m_pixmapItem = other.m_pixmapItem->clone();
	} else {
		m_pixmapItem = nullptr;
	}

	return *this;
}

QString PoseImageData::getImageUrl() const
{
	return m_imageUrl;
}

QSharedPointer<MovablePixmapItem> PoseImageData::getPixmapItem() const
{
	return m_pixmapItem;
}

void PoseImageData::setImageUrl(QString newUrl)
{
	m_imageUrl = newUrl;
}

void PoseImageData::setPixmapItem(QSharedPointer<MovablePixmapItem> newItem)
{
	m_pixmapItem = newItem;
}

void PoseImageData::setImagePosition(qreal x, qreal y)
{
	m_pixmapItem->setPos(x, y);
}

void PoseImageData::clearPixmapItem()
{
	m_pixmapItem.clear();
}
