#ifndef VECTOR3_H
#define VECTOR3_H

#include <QJsonObject>

class Vector3 {
public:
	Vector3(double x = 0.0, double y = 0.0, double z = 0.0);

	double getX() const;
	void setX(double x);

	double getY() const;
	void setY(double y);

	double getZ() const;
	void setZ(double z);

	QJsonObject toJson() const;
	static Vector3 fromJson(const QJsonObject &obj);

private:
	double m_x;
	double m_y;
	double m_z;
};

#endif // VECTOR3_H
