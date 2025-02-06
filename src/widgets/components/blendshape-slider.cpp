#include "blendshape-slider.h"
#include <QPainter>
#include <QStyleOptionSlider>
#include <QStyle>

BlendshapeSlider::BlendshapeSlider(QWidget *parent) : QSlider(parent)
{
	setRange(0, 100);
}

void BlendshapeSlider::setTrackingValue(int trackingValue)
{
	trackingValue = qBound(minimum(), trackingValue, maximum());
	if (m_trackingValue != trackingValue) {
		m_trackingValue = trackingValue;
		update(); // Redraw marker
	}
}

int BlendshapeSlider::trackingValue()
{
	return m_trackingValue;
}

void BlendshapeSlider::setShowTracking(bool shouldShowTracking)
{
	if (m_showTracking != shouldShowTracking) {
		m_showTracking = shouldShowTracking;
		update();
	}
}

bool BlendshapeSlider::showTracking()
{
	return m_showTracking;
}

void BlendshapeSlider::paintEvent(QPaintEvent *event)
{
	if (!m_showTracking) {
		QSlider::paintEvent(event);
		return;
	}

	QStyleOptionSlider opt;
	initStyleOption(&opt);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	// Draw the slider groove only to allow us to draw the handle separately
	opt.subControls = QStyle::SC_SliderGroove;
	style()->drawComplexControl(QStyle::CC_Slider, &opt, &painter, this);

	QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
	int sliderLength = style()->pixelMetric(QStyle::PM_SliderLength, &opt, this);
	int sliderMin = 0;
	int sliderMax = (orientation() == Qt::Horizontal) ? grooveRect.width() - sliderLength
							  : grooveRect.height() - sliderLength;

	int pos = style()->sliderPositionFromValue(minimum(), maximum(), m_trackingValue, sliderMax, sliderMin);

	pos += sliderMin;

	QColor penColor;
	penColor.setRgb(150, 0, 0);

	QPen pen(penColor, 6);
	painter.setPen(pen);

	if (orientation() == Qt::Horizontal) {
		int x = grooveRect.x() + pos + sliderLength / 2;
		painter.drawLine(x, grooveRect.top(), x, grooveRect.bottom());
	} else {
		int y = grooveRect.y() + (sliderMax - pos) + sliderLength / 2;
		painter.drawLine(grooveRect.right(), y, grooveRect.left(), y);
	}

	// Draw the slider handle last on top of both slider groove and marker
	opt.subControls = QStyle::SC_SliderHandle;
	style()->drawComplexControl(QStyle::CC_Slider, &opt, &painter, this);
}
