#ifndef BLENDSHAPE_H
#define BLENDSHAPE_H

#include "blendshape-key.h"
#include <QJsonObject>

class Blendshape {
public:
	BlendshapeKey key;
	double value;

	QJsonObject toJson() const;
	static Blendshape fromJson(const QJsonObject &obj);
};

#endif // BLENDSHAPE_H
