#ifndef BLENDSHAPESLIDER_H
#define BLENDSHAPESLIDER_H

#include <QSlider>

class BlendshapeSlider : public QSlider {
	Q_OBJECT

public:
	explicit BlendshapeSlider(QWidget *parent = nullptr);

	void setTrackingValue(int trackingValue);
	int trackingValue();

	void setShowTracking(bool shouldShowTracking);
	bool showTracking();

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	int m_trackingValue = 0;
	bool m_showTracking = false;
};

#endif // BLENDSHAPESLIDER_H
