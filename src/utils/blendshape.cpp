#include "blendshape.hpp"

QJsonObject Blendshape::toJson() const
{
	QJsonObject obj;
	obj["k"] = blendshapeKeyToString(key);
	obj["v"] = value;
	return obj;
}

Blendshape Blendshape::fromJson(const QJsonObject &obj)
{
	Blendshape bs;
	bs.key = blendshapeKeyFromString(obj["k"].toString());
	if (bs.key == BlendshapeKey::UNKNOWN)
		return {};

	bs.value = obj["v"].toDouble();
	return bs;
}
