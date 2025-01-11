#include "pose.h"

Pose::Pose()
	: m_poseId(""),
	  m_bodyPosition(0.0, 0.0, 0.0),
	  m_eyesPosition(0.0, 0.0, 0.0),
	  m_mouthPosition(0.0, 0.0, 0.0),
	  m_poseImages() // Default-initialized
{
	for (auto &poseImage : m_poseImages) {
		poseImage = PoseImageData();
	}
}

Pose::Pose(const Pose &other)
	: m_poseId(other.m_poseId),
	  m_poseImages(other.m_poseImages),
	  m_bodyPosition(other.m_bodyPosition),
	  m_eyesPosition(other.m_eyesPosition),
	  m_mouthPosition(other.m_mouthPosition),
	  m_blendShapesRuleList(other.m_blendShapesRuleList)
{
	// Deep copy
}

// Copy Assignment Operato
Pose &Pose::operator=(const Pose &other)
{
	if (this == &other) {
		return *this; // Handle self-assignment
	}

	m_poseId = other.m_poseId;
	m_poseImages = other.m_poseImages;
	m_bodyPosition = other.m_bodyPosition;
	m_eyesPosition = other.m_eyesPosition;
	m_mouthPosition = other.m_mouthPosition;
	m_blendShapesRuleList = other.m_blendShapesRuleList;

	return *this;
}

QSharedPointer<Pose> Pose::clone() const
{
	QSharedPointer<Pose> clonedPose(new Pose(*this)); // Utilizes the copy constructor

	// Deep clone each PoseImageData's MovablePixmapItem
	std::array<PoseImageData, static_cast<size_t>(PoseImage::COUNT)> clonedPoseImages = m_poseImages;
	for (PoseImageData &poseImageData : clonedPoseImages) {
		if (poseImageData.getPixmapItem()) {
			poseImageData.setPixmapItem(poseImageData.getPixmapItem()->clone());
		}
	}
	clonedPose->m_poseImages = clonedPoseImages;

	return clonedPose;
}

QString Pose::getPoseId()
{
	return m_poseId;
}

size_t Pose::getPoseImageListSize()
{
	return m_poseImages.size();
}

Vector3 Pose::getBodyPosition()
{
	return m_bodyPosition;
}
Vector3 Pose::getEyesPosition()
{
	return m_eyesPosition;
}
Vector3 Pose::getMouthPosition()
{
	return m_mouthPosition;
}

void Pose::setBodyPosition(Vector3 newPosition)
{
	m_bodyPosition = newPosition;
}
void Pose::setEyesPosition(Vector3 newPosition)
{
	m_eyesPosition = newPosition;
}
void Pose::setMouthPosition(Vector3 newPosition)
{
	m_mouthPosition = newPosition;
}

void Pose::setPoseId(QString newId)
{
	m_poseId = newId;
}

bool Pose::shouldUsePose(const QMap<BlendshapeKey, Blendshape> &blendShapes) const
{
	for (const auto &rule : m_blendShapesRuleList) {
		if (blendShapes.contains(rule.key)) {
			const Blendshape &bs = blendShapes.value(rule.key);
			if (rule.evaluate(bs)) {
				return true;
			}
		}
	}
	return false;
}

void Pose::addBlendShapeRule(BlendshapeRule rule)
{
	m_blendShapesRuleList.push_back(rule);
}

void Pose::removeBlendShapeRule(int index)
{
	UNUSED_PARAMETER(index);
}

PoseImageData *Pose::getPoseImageData(PoseImage pose)
{
	if (auto index = static_cast<size_t>(pose); index < m_poseImages.size()) {
		return &m_poseImages[index];
	}
	// Return nullptr if the pose is out of range
	return nullptr;
}

PoseImageData *Pose::getPoseImageAt(size_t index)
{
	return &m_poseImages.at(index);
}

QJsonObject Pose::toJson() const
{
	QJsonObject obj;
	obj["poseId"] = m_poseId;

	QJsonObject imagesObj;
	for (size_t i = 0; i < m_poseImages.size(); ++i) {
		const PoseImageData &data = m_poseImages[i];
		QJsonObject imageData;
		imageData["imageUrl"] = data.getImageUrl();

		QString key = QString::number(static_cast<int>(static_cast<PoseImage>(i)));
		imagesObj[key] = imageData; // Assigning imageData to imagesObj
	}
	obj["poseImages"] = imagesObj;

	obj["bodyPosition"] = m_bodyPosition.toJson();
	obj["eyesPosition"] = m_eyesPosition.toJson();
	obj["mouthPosition"] = m_mouthPosition.toJson();

	// Convert blendShapesRuleList to a QJsonArray
	QJsonArray rulesArray;
	for (const auto &rule : m_blendShapesRuleList) {
		QJsonObject ruleObj;
		ruleObj["key"] = static_cast<int>(rule.key); // or store as string if you prefer
		ruleObj["compareType"] = static_cast<int>(rule.compareType);
		ruleObj["compareValue"] = rule.compareValue;
		rulesArray.append(ruleObj);
	}
	obj["blendShapesRuleList"] = rulesArray;

	return obj;
}

Pose Pose::fromJson(const QJsonObject &obj)
{
	Pose pose;
	pose.setPoseId(obj["poseId"].toString());

	QJsonObject imagesObj = obj["poseImages"].toObject();
	for (size_t i = 0; i < pose.m_poseImages.size(); ++i) {
		PoseImage poseEnum = static_cast<PoseImage>(i);
		QString key = QString::number(static_cast<int>(poseEnum));
		if (imagesObj.contains(key)) {
			QJsonObject imageData = imagesObj[key].toObject();
			pose.m_poseImages[i].setImageUrl(imageData["imageUrl"].toString());
		}
	}

	QJsonObject bodyPosObj = obj["bodyPosition"].toObject();
	pose.setBodyPosition(Vector3::fromJson(bodyPosObj));

	QJsonObject eyesPosObj = obj["eyesPosition"].toObject();
	pose.setEyesPosition(Vector3::fromJson(eyesPosObj));

	QJsonObject mouthPosObj = obj["mouthPosition"].toObject();
	pose.setMouthPosition(Vector3::fromJson(mouthPosObj));

	QJsonArray rulesArray = obj["blendShapesRuleList"].toArray();
	for (auto ruleValue : rulesArray) {
		if (ruleValue.isObject()) {
			QJsonObject ruleObj = ruleValue.toObject();
			BlendshapeRule rule;
			rule.key = static_cast<BlendshapeKey>(ruleObj["key"].toInt());
			rule.compareType = static_cast<ComparisonType>(ruleObj["compareType"].toInt());
			rule.compareValue = ruleObj["compareValue"].toDouble();
			pose.addBlendShapeRule(rule);
		}
	}
	return pose;
}
