#ifndef POSE_H
#define POSE_H

#include "vector3.h"
#include <array>
#include <vector>
#include <QJsonObject>
#include <QJsonArray>
#include <QSharedPointer>
#include <obs.h>

#include "pose-image.h"
#include "pose-image-data.h"
#include "pose-image-data.h"
#include "../blendshapes/blendshape-key.h"
#include "../blendshapes/blendshape-rule.h"
#include "../blendshapes/blendshape.h"

// Forward declarations

class Pose {
public:
	Pose();
	Pose(const Pose &other);
	Pose &operator=(const Pose &other);

	~Pose() = default;

	QSharedPointer<Pose> clone() const;
	bool shouldUsePose(const QMap<BlendshapeKey, Blendshape> &blendShapes) const;

	QString getPoseId();
	size_t getPoseImageListSize();
	Vector3 getBodyPosition();
	Vector3 getEyesPosition();
	Vector3 getMouthPosition();

	void setBodyPosition(Vector3 newPosition);
	void setEyesPosition(Vector3 newPosition);
	void setMouthPosition(Vector3 newPosition);

	void setPoseId(QString newId);

	void addBlendShapeRule(BlendshapeRule rule);
	void removeBlendShapeRule(int index);

	PoseImageData *getPoseImageAt(int index);
	PoseImageData *getPoseImageData(PoseImage pose);
	QJsonObject toJson() const;
	static Pose fromJson(const QJsonObject &obj);

private:
	QString m_poseId;
	Vector3 m_bodyPosition;
	Vector3 m_eyesPosition;
	Vector3 m_mouthPosition;
	std::vector<BlendshapeRule> m_blendShapesRuleList;

	std::array<PoseImageData, static_cast<size_t>(PoseImage::COUNT)> m_poseImages;
};

#endif // POSE_H
