#include "avatar-graphics-view.hpp"

void AvatarGraphicsView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MiddleButton) {
		// Start panning
		setCursor(Qt::ClosedHandCursor);
		m_panning = true;
		m_lastPanPoint = event->pos();
		event->accept();
	} else {
		// Let the default handling occur, which may select/move items
		QGraphicsView::mousePressEvent(event);
	}
}

void AvatarGraphicsView::wheelEvent(QWheelEvent *event)
{
	// Check the scroll direction
	if (event->angleDelta().y() > 0) {
		// Zoom in
		scale(zoomScaleFactor, zoomScaleFactor);
	} else {
		// Zoom out
		scale(1.0 / zoomScaleFactor, 1.0 / zoomScaleFactor);
	}
	event->accept();
}

void AvatarGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
	if (m_panning) {
		// Current mouse position - last mouse position
		QPointF delta = mapToScene(event->pos()) - mapToScene(m_lastPanPoint);

		// Adjust the scrollbars by negative delta
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
		verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());

		// Update the last pan point
		m_lastPanPoint = event->pos();
		event->accept();
	} else {
		for (QGraphicsItem *item : items()) {
			QRectF sceneRect = this->sceneRect();
			QRectF itemRect = item->sceneBoundingRect();

			// Check and adjust the item's position
			if (!sceneRect.contains(itemRect)) {
				QPointF newPos = item->pos();
				if (itemRect.left() < sceneRect.left())
					newPos.setX(sceneRect.left());
				if (itemRect.top() < sceneRect.top())
					newPos.setY(sceneRect.top());
				if (itemRect.right() > sceneRect.right())
					newPos.setX(sceneRect.right() - itemRect.width());
				if (itemRect.bottom() > sceneRect.bottom())
					newPos.setY(sceneRect.bottom() - itemRect.height());

				item->setPos(newPos);
			}
		}
		QGraphicsView::mouseMoveEvent(event);
	}
}

void AvatarGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MiddleButton) {
		setCursor(Qt::ArrowCursor);
		m_panning = false;
		event->accept();
	} else {
		QGraphicsView::mouseReleaseEvent(event);
	}
}
