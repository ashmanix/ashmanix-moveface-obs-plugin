#include "pose.h"

Pose::Pose() : m_poseImages() // Default-initialized
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
	  m_eyesHalfOpenLimit(other.m_eyesHalfOpenLimit),
	  m_eyesOpenLimit(other.m_eyesOpenLimit),
	  m_mouthOpenLimit(other.m_mouthOpenLimit),
	  m_smileLimit(other.m_smileLimit),
	  m_tongueOutLimit(other.m_tongueOutLimit),
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
	m_eyesHalfOpenLimit = other.m_eyesHalfOpenLimit;
	m_eyesOpenLimit = other.m_eyesOpenLimit;
	m_mouthOpenLimit = other.m_mouthOpenLimit;
	m_smileLimit = other.m_smileLimit;
	m_tongueOutLimit = other.m_tongueOutLimit;
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

QString Pose::getPoseId() const
{
	return m_poseId;
}

size_t Pose::getPoseImageListSize() const
{
	return m_poseImages.size();
}

double Pose::getEyesHalfOpenLimit() const
{
	return m_eyesHalfOpenLimit;
}

double Pose::getEyesOpenLimit() const
{
	return m_eyesOpenLimit;
}

double Pose::getMouthOpenLimit() const
{
	return m_mouthOpenLimit;
}

double Pose::getSmileLimit() const
{
	return m_smileLimit;
}

double Pose::getTongueOutLimit() const
{
	return m_tongueOutLimit;
}

Vector3 Pose::getBodyPosition() const
{
	return m_bodyPosition;
}

Vector3 Pose::getEyesPosition() const
{
	return m_eyesPosition;
}

Vector3 Pose::getMouthPosition() const
{
	return m_mouthPosition;
}

QMap<QString, QSharedPointer<BlendshapeRule>> *Pose::getBlendshapeList()
{
	return &m_blendShapesRuleList;
}

QSharedPointer<BlendshapeRule> Pose::getBlendshapeRule(QString id)
{
	if (auto it = m_blendShapesRuleList.find(id); it != m_blendShapesRuleList.end()) {
		return it.value();
	}
	return nullptr;
}

void Pose::setEyesHalfOpenLimit(double newLimit)
{
	m_eyesHalfOpenLimit = newLimit;
}

void Pose::setEyesOpenLimit(double newLimit)
{
	m_eyesOpenLimit = newLimit;
}

void Pose::setMouthOpenLimit(double newLimit)
{
	m_mouthOpenLimit = newLimit;
}

void Pose::setSmileLimit(double newLimit)
{
	m_smileLimit = newLimit;
}

void Pose::setTongueOutLimit(double newLimit)
{
	m_tongueOutLimit = newLimit;
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

bool Pose::shouldUsePose(const QMap<BlendshapeKey, Blendshape> &blendshapes) const
{
	// If there are no rules then we automatically return true
	// to be used by the main (resting) pose
	if (m_blendShapesRuleList.isEmpty())
		return true;

	for (auto i = m_blendShapesRuleList.begin(), end = m_blendShapesRuleList.end(); i != end; i++) {
		auto rule = i.value();
		if (blendshapes.contains(rule->getKey())) {
			const Blendshape &bs = blendshapes.value(rule->getKey());
			if (rule->evaluate(bs))
				return true;
		}
	}
	return false;
}

void Pose::addBlendShapeRule(QSharedPointer<BlendshapeRule> rule)
{
	m_blendShapesRuleList.insert(rule->getID(), rule);
}

void Pose::removeBlendShapeRule(QString id)
{
	m_blendShapesRuleList.remove(id);
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

	obj["eyesHalfOpenLimit"] = m_eyesHalfOpenLimit;
	obj["eyesOpenLimit"] = m_eyesOpenLimit;
	obj["mouthOpenLimit"] = m_mouthOpenLimit;
	obj["smileLimit"] = m_smileLimit;
	obj["tongueOutLimit"] = m_tongueOutLimit;

	obj["bodyPosition"] = m_bodyPosition.toJson();
	obj["eyesPosition"] = m_eyesPosition.toJson();
	obj["mouthPosition"] = m_mouthPosition.toJson();

	// Convert blendShapesRuleList to a QJsonArray
	QJsonArray rulesArray;
	for (const auto &rule : m_blendShapesRuleList) {
		QJsonObject ruleObj;
		ruleObj["id"] = rule->getID();                     // or store as string if you prefer
		ruleObj["key"] = static_cast<int>(rule->getKey()); // or store as string if you prefer
		ruleObj["compareType"] = static_cast<int>(rule->getComparisonType());
		ruleObj["compareValue"] = rule->getCompareValue();
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
		auto poseEnum = static_cast<PoseImage>(i);
		QString key = QString::number(static_cast<int>(poseEnum));
		if (imagesObj.contains(key)) {
			QJsonObject imageData = imagesObj[key].toObject();
			pose.m_poseImages[i].setImageUrl(imageData["imageUrl"].toString());
		}
	}

	double eyesHalfOpenLimitObj = obj["eyesHalfOpenLimit"].toDouble();
	pose.m_eyesHalfOpenLimit = eyesHalfOpenLimitObj;

	double eyesOpenLimitObj = obj["eyesOpenLimit"].toDouble();
	pose.m_eyesOpenLimit = eyesOpenLimitObj;

	double mouthOpenLimitObj = obj["mouthOpenLimit"].toDouble();
	pose.m_mouthOpenLimit = mouthOpenLimitObj;

	double smileLimitObj = obj["smileLimit"].toDouble();
	pose.m_smileLimit = smileLimitObj;

	double tongueOutLimitObj = obj["tongueOutLimit"].toDouble();
	pose.m_tongueOutLimit = tongueOutLimitObj;

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
			QSharedPointer<BlendshapeRule> rule = QSharedPointer<BlendshapeRule>::create(
				ruleObj["id"].toString(), static_cast<BlendshapeKey>(ruleObj["key"].toInt()),
				static_cast<ComparisonType>(ruleObj["compareType"].toInt()),
				ruleObj["compareValue"].toDouble());
			pose.addBlendShapeRule(rule);
		}
	}
	return pose;
}
