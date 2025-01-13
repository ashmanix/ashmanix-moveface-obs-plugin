#include "blendshape.h"

QJsonObject Blendshape::toJson() const
{
	QJsonObject obj;
	obj["k"] = blendshapeKeyToString(m_key);
	obj["v"] = m_value;
	return obj;
}

Blendshape Blendshape::fromJson(const QJsonObject &obj)
{
	Blendshape bs;
	bs.m_key = blendshapeKeyFromString(obj["k"].toString());
	if (bs.m_key == BlendshapeKey::UNKNOWN)
		return {};

	bs.m_value = obj["v"].toDouble();
	return bs;
}
