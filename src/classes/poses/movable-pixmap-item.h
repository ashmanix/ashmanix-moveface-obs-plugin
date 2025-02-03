#ifndef MOVABLEPIXMAPITEM_HPP
#define MOVABLEPIXMAPITEM_HPP

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QSharedPointer>
#include "pose-image.h"

class MovablePixmapItem : public QObject, public QGraphicsPixmapItem {
	Q_OBJECT
public:
	explicit MovablePixmapItem(const QPixmap &pixmap, QObject *parent = nullptr,
				   PoseImage pImageType = PoseImage::BODY)
		: QObject(parent),
		  QGraphicsPixmapItem(pixmap),
		  m_poseImageType(pImageType)
	{
		// Enable the item to be movable and selectable
		setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
		setTransformOriginPoint(pixmap.width() / 2, pixmap.height() / 2);
	}

	// New overloaded constructor
	explicit MovablePixmapItem(PoseImage pImageType = PoseImage::BODY, QObject *parent = nullptr)
		: QObject(parent),
		  QGraphicsPixmapItem(), // Initialize with an empty QPixmap
		  m_poseImageType(pImageType)
	{
		// Enable theœ item to be movable and selectable
		setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
	}

	~MovablePixmapItem() override = default;

	// Get clone of MovablePixmapItem
	QSharedPointer<MovablePixmapItem> clone() const
	{
		auto newItem =
			QSharedPointer<MovablePixmapItem>::create(this->pixmap(), nullptr, this->m_poseImageType);

		newItem->setTransformOriginPoint(this->transformOriginPoint());
		newItem->setPos(this->pos());
		newItem->setZValue(this->zValue());

		return QSharedPointer<MovablePixmapItem>(newItem);
	}

private:
	PoseImage m_poseImageType;

signals:
	void positionChanged(qreal x, qreal y, qreal z, PoseImage pImageType);

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) override
	{
		if (change == ItemPositionChange || change == ItemZValueChange) {
			emit positionChanged(pos().x(), pos().y(), zValue(), m_poseImageType);
		}
		return QGraphicsPixmapItem::itemChange(change, value);
	}
};

#endif // MOVABLEPIXMAPITEM_HPP
