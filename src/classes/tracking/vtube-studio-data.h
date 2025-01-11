#ifndef VTUBESTUDIODATA_H
#define VTUBESTUDIODATA_H

#include <array>
#include <vector>
#include <QJsonObject>
#include <QJsonArray>
#include <QSharedPointer>
#include <QMap>

#include "../poses/vector3.h"
#include "../blendshapes/blendshape.h"

class VTubeStudioData {
public:
	VTubeStudioData();
	~VTubeStudioData();

	qint64 getTimeStamp();
	int getHotkey() const;
	bool getFaceFound() const;
	Vector3 getRotation() const;
	Vector3 getPosition() const;
	Vector3 getVNyanPos() const;
	Vector3 getEyeLeft() const;
	Vector3 getEyeRight() const;
	QMap<BlendshapeKey, Blendshape> getBlendshapes() const;

	void setTimeStamp(qint64 newTimestamp);
	void setHotkey(int newHotkey);
	void setFaceFound(bool faceFound);
	void setRotation(Vector3 newRotation);
	void setPosition(Vector3 newPosition);
	void setVNyanPos(Vector3 newVNyanPos);
	void setEyeLeft(Vector3 newPos);
	void setEyeRight(Vector3 newPos);
	void setBlendshapes() const;

	void addBlendshape(BlendshapeKey key, Blendshape blendshape);

	QJsonObject toJson() const;
	static VTubeStudioData fromJson(const QJsonObject &obj);

private:
	qint64 m_timestamp;
	int m_hotkey;
	bool m_faceFound;
	Vector3 m_rotation;
	Vector3 m_position;
	Vector3 m_vNyanPos;
	Vector3 m_eyeLeft;
	Vector3 m_eyeRight;
	QMap<BlendshapeKey, Blendshape> m_blendshapes;
};

#endif // VTUBESTUDIODATA_H
