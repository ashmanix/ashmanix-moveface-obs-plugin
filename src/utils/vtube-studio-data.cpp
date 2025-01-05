#include "vtube-studio-data.hpp"

VTubeStudioData::VTubeStudioData() {}

VTubeStudioData::~VTubeStudioData() {}

qint64 VTubeStudioData::getTimeStamp()
{
	return m_timestamp;
}

int VTubeStudioData::getHotkey()
{
	return m_hotkey;
}

bool VTubeStudioData::getFaceFound()
{
	return m_faceFound;
}

Vector3 VTubeStudioData::getRotation()
{
	return m_rotation;
}

Vector3 VTubeStudioData::getPosition()
{
	return m_position;
}

Vector3 VTubeStudioData::getVNyanPos()
{
	return m_vNyanPos;
}

Vector3 VTubeStudioData::getEyeLeft()
{
	return m_eyeLeft;
}

Vector3 VTubeStudioData::getEyeRight()
{
	return m_eyeRight;
}

QMap<BlendshapeKey, Blendshape> VTubeStudioData::getBlendshapes()
{
	return m_blendshapes;
}

void VTubeStudioData::setTimeStamp(qint64 newTimestamp)
{
	m_timestamp = newTimestamp;
}

void VTubeStudioData::setHotkey(int newHotkey)
{
	m_hotkey = newHotkey;
}

void VTubeStudioData::setFaceFound(bool faceFound)
{
	m_faceFound = faceFound;
}

void VTubeStudioData::setRotation(Vector3 newRotation)
{
	m_rotation = newRotation;
}

void VTubeStudioData::setPosition(Vector3 newPosition)
{
	m_position = newPosition;
}

void VTubeStudioData::setVNyanPos(Vector3 newVNyanPos)
{
	m_vNyanPos = newVNyanPos;
}

void VTubeStudioData::setEyeLeft(Vector3 newPos)
{
	m_eyeLeft = newPos;
}

void VTubeStudioData::setEyeRight(Vector3 newPos)
{
	m_eyeRight = newPos;
}

void VTubeStudioData::setBlendshapes() {}

void VTubeStudioData::addBlendshape(BlendshapeKey key, Blendshape blendshape)
{
	m_blendshapes.insert(key, blendshape);
}

QJsonObject VTubeStudioData::toJson() const
{
	QJsonObject obj;
	obj["Timestamp"] = static_cast<qint64>(m_timestamp);
	obj["Hotkey"] = m_hotkey;
	obj["FaceFound"] = m_faceFound;
	obj["Rotation"] = m_rotation.toJson();
	obj["Position"] = m_position.toJson();
	obj["VNyanPos"] = m_vNyanPos.toJson();
	obj["EyeLeft"] = m_eyeLeft.toJson();
	obj["EyeRight"] = m_eyeRight.toJson();

	QJsonArray blendshapesArray;
	for (const Blendshape &bs : m_blendshapes) {
		QJsonObject newBs = bs.toJson();
		if ((newBs["k"]) != "")
			blendshapesArray.append(bs.toJson());
	}
	obj["BlendShapes"] = blendshapesArray;

	return obj;
}

VTubeStudioData VTubeStudioData::fromJson(const QJsonObject &obj)
{
	VTubeStudioData trackingData;
	trackingData.setTimeStamp(obj["Timestamp"].toVariant().toLongLong());
	trackingData.setHotkey(obj["Hotkey"].toInt());
	trackingData.setFaceFound(obj["FaceFound"].toBool());
	trackingData.setRotation(Vector3::fromJson(obj["Rotation"].toObject()));
	trackingData.setPosition(Vector3::fromJson(obj["Position"].toObject()));
	trackingData.setVNyanPos(Vector3::fromJson(obj["VNyanPos"].toObject()));
	trackingData.setEyeLeft(Vector3::fromJson(obj["EyeLeft"].toObject()));
	trackingData.setEyeRight(Vector3::fromJson(obj["EyeRight"].toObject()));

	QJsonArray blendShapesArray = obj["BlendShapes"].toArray();
	for (const auto item : blendShapesArray) {
		if (item.isObject()) {
			Blendshape bsToAdd = Blendshape::fromJson(item.toObject());
			trackingData.addBlendshape(bsToAdd.key, bsToAdd);
		}
	}

	return trackingData;
}
