#ifndef TRACKERUTILS_H
#define TRACKERUTILS_H

#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QString>
#include <QUuid>
#include <QCryptographicHash>

struct TrackerDataStruct {
	QString trackerId;
	QString destIpAddress = "255.255.255.255";
	int destPort = 21412;
	int port = 21412;
	bool isEnabled = false;
};

struct Result {
	bool success;
	QString errorMessage;
};

struct Vector3 {
	double x;
	double y;
	double z;

	QJsonObject toJson() const
	{
		QJsonObject obj;
		obj["x"] = x;
		obj["y"] = y;
		obj["z"] = z;
		return obj;
	}

	static Vector3 fromJson(const QJsonObject &obj)
	{
		Vector3 vec;
		vec.x = obj["x"].toDouble();
		vec.y = obj["y"].toDouble();
		vec.z = obj["z"].toDouble();
		return vec;
	}
};

struct BlendShape {
	QString k;
	double v;

	QJsonObject toJson() const
	{
		QJsonObject obj;
		obj["k"] = k;
		obj["v"] = v;
		return obj;
	}

	static BlendShape fromJson(const QJsonObject &obj)
	{
		BlendShape bs;
		bs.k = obj["k"].toString();
		bs.v = obj["v"].toDouble();
		return bs;
	}
};

struct VTubeStudioTrackingData {
	qint64 timestamp;
	int hotkey;
	bool faceFound;
	Vector3 rotation;
	Vector3 position;
	Vector3 vNyanPos;
	Vector3 eyeLeft;
	Vector3 eyeRight;
	QVector<BlendShape> blendShapes;

	QJsonObject toJson() const
	{
		QJsonObject obj;
		obj["Timestamp"] = static_cast<qint64>(timestamp);
		obj["Hotkey"] = hotkey;
		obj["FaceFound"] = faceFound;
		obj["Rotation"] = rotation.toJson();
		obj["Position"] = position.toJson();
		obj["VNyanPos"] = vNyanPos.toJson();
		obj["EyeLeft"] = eyeLeft.toJson();
		obj["EyeRight"] = eyeRight.toJson();

		QJsonArray blendShapesArray;
		for (const BlendShape &bs : blendShapes) {
			blendShapesArray.append(bs.toJson());
		}
		obj["BlendShapes"] = blendShapesArray;

		return obj;
	}

	static VTubeStudioTrackingData fromJson(const QJsonObject &obj)
	{
		VTubeStudioTrackingData trackingData;
		trackingData.timestamp = obj["Timestamp"].toVariant().toLongLong();
		trackingData.hotkey = obj["Hotkey"].toInt();
		trackingData.faceFound = obj["FaceFound"].toBool();
		trackingData.rotation = Vector3::fromJson(obj["Rotation"].toObject());
		trackingData.position = Vector3::fromJson(obj["Position"].toObject());
		trackingData.vNyanPos = Vector3::fromJson(obj["VNyanPos"].toObject());
		trackingData.eyeLeft = Vector3::fromJson(obj["EyeLeft"].toObject());
		trackingData.eyeRight = Vector3::fromJson(obj["EyeRight"].toObject());

		QJsonArray blendShapesArray = obj["BlendShapes"].toArray();
		for (const auto item : blendShapesArray) {
			if (item.isObject()) {
				trackingData.blendShapes.append(BlendShape::fromJson(item.toObject()));
			}
		}

		return trackingData;
	}
};

QString GenerateUniqueID();

#endif // TRACKERUTILS_H
