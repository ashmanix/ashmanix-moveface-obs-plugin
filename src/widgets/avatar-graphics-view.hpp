
#ifndef AVATARGRAPHICSVIEW_H
#define AVATARGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QScrollBar>
#include <QGraphicsScene>
#include <QGraphicsItem>

#include <obs.h>
#include "plugin-support.h"

class AvatarGraphicsView : public QGraphicsView {
	Q_OBJECT

public:
	explicit AvatarGraphicsView(QWidget *parent = nullptr) : QGraphicsView(parent)
	{
		setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		setRenderHint(QPainter::Antialiasing);
		setDragMode(QGraphicsView::RubberBandDrag);
	}

protected:
	void wheelEvent(QWheelEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

private:
	bool m_panning = false;
	QPoint m_lastPanPoint;
	double zoomScaleFactor = 1.15;
};

#endif // AVATARGRAPHICSVIEW_H
