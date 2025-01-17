#include "avatar-graphics-view.h"

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
		scale(m_zoomScaleFactor, m_zoomScaleFactor);
	} else {
		// Zoom out
		scale(1.0 / m_zoomScaleFactor, 1.0 / m_zoomScaleFactor);
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
