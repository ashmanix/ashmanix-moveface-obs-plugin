#include "vector3.h"

Vector3::Vector3(double x, double y, double z) : m_x(x), m_y(y), m_z(z) {}

double Vector3::getX() const
{
	return m_x;
}
void Vector3::setX(double x)
{
	m_x = x;
}

double Vector3::getY() const
{
	return m_y;
}
void Vector3::setY(double y)
{
	m_y = y;
}

double Vector3::getZ() const
{
	return m_z;
}
void Vector3::setZ(double z)
{
	m_z = z;
}

QJsonObject Vector3::toJson() const
{
	QJsonObject obj;
	obj["x"] = m_x;
	obj["y"] = m_y;
	obj["z"] = m_z;
	return obj;
}

Vector3 Vector3::fromJson(const QJsonObject &obj)
{
	Vector3 vec;
	vec.m_x = obj["x"].toDouble();
	vec.m_y = obj["y"].toDouble();
	vec.m_z = obj["z"].toDouble();
	return vec;
}
