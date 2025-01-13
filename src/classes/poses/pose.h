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
	size_t getPoseImageListSize() const;

	double getEyesHalfOpenLimit() const;
	double getEyesOpenLimit() const;
	double getMouthOpenLimit() const;
	double getTongueOutLimit() const;

	Vector3 getBodyPosition() const;
	Vector3 getEyesPosition() const;
	Vector3 getMouthPosition() const;

	QMap<QString, QSharedPointer<BlendshapeRule>> *getBlendshapeList();
	QSharedPointer<BlendshapeRule> getBlendshapeRule(QString id);

	void setBodyPosition(Vector3 newPosition);
	void setEyesPosition(Vector3 newPosition);
	void setMouthPosition(Vector3 newPosition);

	void setEyesHalfOpenLimit(double newLimit);
	void setEyesOpenLimit(double newLimit);
	void setMouthOpenLimit(double newLimit);
	void setTongueOutLimit(double newLimit);

	void setPoseId(QString newId);

	void addBlendShapeRule(QSharedPointer<BlendshapeRule> rule);
	void removeBlendShapeRule(QString id);

	PoseImageData *getPoseImageAt(size_t index);
	PoseImageData *getPoseImageData(PoseImage pose);
	QJsonObject toJson() const;
	static Pose fromJson(const QJsonObject &obj);

private:
	QString m_poseId = "";
	Vector3 m_bodyPosition = {0.0, 0.0, 0.0};
	Vector3 m_eyesPosition = {0.0, 0.0, 0.0};
	Vector3 m_mouthPosition = {0.0, 0.0, 0.0};

	double m_eyesHalfOpenLimit = 0.4;
	double m_eyesOpenLimit = 0.7;
	double m_mouthOpenLimit = 0.5;
	double m_tongueOutLimit = 0.7;

	QMap<QString, QSharedPointer<BlendshapeRule>> m_blendShapesRuleList;

	std::array<PoseImageData, static_cast<size_t>(PoseImage::COUNT)> m_poseImages = {};
};

#endif // POSE_H
