#ifndef TRACKERUTILS_H
#define TRACKERUTILS_H

#include <QVector>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDateTime>
#include <QString>
#include <QUuid>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QGraphicsPixmapItem>

#include <obs.h>
#include "pose-image.hpp"
#include "../widgets/pixmap-item.hpp"
#include "plugin-support.h"

#include <array>

#define CONFIG "config.json"

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

enum class BlendShapeKey {
	BROWOUTERUP_L,
	BROWINNERUP_L,
	BROWDOWN_L,
	EYEBLINK_L,
	EYESQUINT_L,
	EYEWIDE_L,
	EYELOOKUP_L,
	EYELOOKOUT_L,
	EYELOOKIN_L,
	EYELOOKDOWN_L,
	NOSESNEER_L,
	MOUTHUPPERUP_L,
	MOUTHSMILE_L,
	MOUTHLEFT,
	MOUTHFROWN_L,
	MOUTHLOWERDOWN_L,
	JAWLEFT,
	CHEEKPUFF,
	MOUTHSHRUGUPPER,
	MOUTHFUNNEL,
	MOUTHROLLLOWER,
	JAWOPEN,
	TONGUEOUT,
	MOUTHPUCKER,
	MOUTHROLLUPPER,
	JAWRIGHT,
	MOUTHLOWERDOWN_R,
	MOUTHFROWN_R,
	MOUTHRIGHT,
	MOUTHSMILE_R,
	MOUTHUPPERUP_R,
	NOSESNEER_R,
	EYELOOKDOWN_R,
	EYELOOKIN_R,
	EYELOOKOUT_R,
	EYELOOKUP_R,
	EYEWIDE_R,
	EYESQUINT_R,
	EYEBLINK_R,
	BROWDOWN_R,
	BROWINNERUP_R,
	BROWOUTERUP_R,
	HEADLEFT,
	HEADRIGHT,
	HEADUP,
	HEADDOWN,
	HEADROLLLEFT,
	HEADROLLRIGHT,
	EYEBLINKLEFT,
	EYEBLINKRIGHT,
	UNKNOWN,
};

inline QString blendShapeKeyToString(BlendShapeKey key)
{
	switch (key) {
	case BlendShapeKey::BROWOUTERUP_L:
		return "browOuterUp_L";
	case BlendShapeKey::BROWINNERUP_L:
		return "browInnerUp_L";
	case BlendShapeKey::BROWDOWN_L:
		return "browDown_L";
	case BlendShapeKey::EYEBLINK_L:
		return "eyeBlink_L";
	case BlendShapeKey::EYESQUINT_L:
		return "eyeSquint_L";
	case BlendShapeKey::EYEWIDE_L:
		return "eyeWide_L";
	case BlendShapeKey::EYELOOKUP_L:
		return "eyeLookUp_L";
	case BlendShapeKey::EYELOOKOUT_L:
		return "eyeLookOut_L";
	case BlendShapeKey::EYELOOKIN_L:
		return "eyeLookIn_L";
	case BlendShapeKey::EYELOOKDOWN_L:
		return "eyeLookDown_L";
	case BlendShapeKey::NOSESNEER_L:
		return "noseSneer_L";
	case BlendShapeKey::MOUTHUPPERUP_L:
		return "mouthUpperUp_L";
	case BlendShapeKey::MOUTHSMILE_L:
		return "mouthSmile_L";
	case BlendShapeKey::MOUTHLEFT:
		return "mouthLeft";
	case BlendShapeKey::MOUTHFROWN_L:
		return "mouthFrown_L";
	case BlendShapeKey::MOUTHLOWERDOWN_L:
		return "mouthLowerDown_L";
	case BlendShapeKey::JAWLEFT:
		return "jawLeft";
	case BlendShapeKey::CHEEKPUFF:
		return "cheekPuff";
	case BlendShapeKey::MOUTHSHRUGUPPER:
		return "mouthShrugUpper";
	case BlendShapeKey::MOUTHFUNNEL:
		return "mouthFunnel";
	case BlendShapeKey::MOUTHROLLLOWER:
		return "mouthRollLower";
	case BlendShapeKey::JAWOPEN:
		return "jawOpen";
	case BlendShapeKey::TONGUEOUT:
		return "tongueOut";
	case BlendShapeKey::MOUTHPUCKER:
		return "mouthPucker";
	case BlendShapeKey::MOUTHROLLUPPER:
		return "mouthRollUpper";
	case BlendShapeKey::JAWRIGHT:
		return "jawRight";
	case BlendShapeKey::MOUTHLOWERDOWN_R:
		return "mouthLowerDown_R";
	case BlendShapeKey::MOUTHFROWN_R:
		return "mouthFrown_R";
	case BlendShapeKey::MOUTHRIGHT:
		return "mouthRight";
	case BlendShapeKey::MOUTHSMILE_R:
		return "mouthSmile_R";
	case BlendShapeKey::MOUTHUPPERUP_R:
		return "mouthUpperUp_R";
	case BlendShapeKey::NOSESNEER_R:
		return "noseSneer_R";
	case BlendShapeKey::EYELOOKDOWN_R:
		return "eyeLookDown_R";
	case BlendShapeKey::EYELOOKIN_R:
		return "eyeLookIn_R";
	case BlendShapeKey::EYELOOKOUT_R:
		return "eyeLookOut_R";
	case BlendShapeKey::EYELOOKUP_R:
		return "eyeLookUp_R";
	case BlendShapeKey::EYEWIDE_R:
		return "eyeWide_R";
	case BlendShapeKey::EYESQUINT_R:
		return "eyeSquint_R";
	case BlendShapeKey::EYEBLINK_R:
		return "eyeBlink_R";
	case BlendShapeKey::BROWDOWN_R:
		return "browDown_R";
	case BlendShapeKey::BROWINNERUP_R:
		return "browInnerUp_R";
	case BlendShapeKey::BROWOUTERUP_R:
		return "browOuterUp_R";
	case BlendShapeKey::HEADLEFT:
		return "headLeft";
	case BlendShapeKey::HEADRIGHT:
		return "headRight";
	case BlendShapeKey::HEADUP:
		return "headUp";
	case BlendShapeKey::HEADDOWN:
		return "headDown";
	case BlendShapeKey::HEADROLLLEFT:
		return "headRollLeft";
	case BlendShapeKey::HEADROLLRIGHT:
		return "headRollRight";
	case BlendShapeKey::EYEBLINKLEFT:
		return "EyeBlinkLeft";
	case BlendShapeKey::EYEBLINKRIGHT:
		return "EyeBlinkRight";
	default:
		return QString();
	}
}

inline BlendShapeKey blendShapeKeyFromString(const QString &str)
{
	if (str == "browOuterUp_L")
		return BlendShapeKey::BROWOUTERUP_L;
	if (str == "browInnerUp_L")
		return BlendShapeKey::BROWINNERUP_L;
	if (str == "browDown_L")
		return BlendShapeKey::BROWDOWN_L;
	if (str == "eyeBlink_L")
		return BlendShapeKey::EYEBLINK_L;
	if (str == "eyeSquint_L")
		return BlendShapeKey::EYESQUINT_L;
	if (str == "eyeWide_L")
		return BlendShapeKey::EYEWIDE_L;
	if (str == "eyeLookUp_L")
		return BlendShapeKey::EYELOOKUP_L;
	if (str == "eyeLookOut_L")
		return BlendShapeKey::EYELOOKOUT_L;
	if (str == "eyeLookIn_L")
		return BlendShapeKey::EYELOOKIN_L;
	if (str == "eyeLookDown_L")
		return BlendShapeKey::EYELOOKDOWN_L;
	if (str == "noseSneer_L")
		return BlendShapeKey::NOSESNEER_L;
	if (str == "mouthUpperUp_L")
		return BlendShapeKey::MOUTHUPPERUP_L;
	if (str == "mouthSmile_L")
		return BlendShapeKey::MOUTHSMILE_L;
	if (str == "mouthLeft")
		return BlendShapeKey::MOUTHLEFT;
	if (str == "mouthFrown_L")
		return BlendShapeKey::MOUTHFROWN_L;
	if (str == "mouthLowerDown_L")
		return BlendShapeKey::MOUTHLOWERDOWN_L;
	if (str == "jawLeft")
		return BlendShapeKey::JAWLEFT;
	if (str == "cheekPuff")
		return BlendShapeKey::CHEEKPUFF;
	if (str == "mouthShrugUpper")
		return BlendShapeKey::MOUTHSHRUGUPPER;
	if (str == "mouthFunnel")
		return BlendShapeKey::MOUTHFUNNEL;
	if (str == "mouthRollLower")
		return BlendShapeKey::MOUTHROLLLOWER;
	if (str == "jawOpen")
		return BlendShapeKey::JAWOPEN;
	if (str == "tongueOut")
		return BlendShapeKey::TONGUEOUT;
	if (str == "mouthPucker")
		return BlendShapeKey::MOUTHPUCKER;
	if (str == "mouthRollUpper")
		return BlendShapeKey::MOUTHROLLUPPER;
	if (str == "jawRight")
		return BlendShapeKey::JAWRIGHT;
	if (str == "mouthLowerDown_R")
		return BlendShapeKey::MOUTHLOWERDOWN_R;
	if (str == "mouthFrown_R")
		return BlendShapeKey::MOUTHFROWN_R;
	if (str == "mouthRight")
		return BlendShapeKey::MOUTHRIGHT;
	if (str == "mouthSmile_R")
		return BlendShapeKey::MOUTHSMILE_R;
	if (str == "mouthUpperUp_R")
		return BlendShapeKey::MOUTHUPPERUP_R;
	if (str == "noseSneer_R")
		return BlendShapeKey::NOSESNEER_R;
	if (str == "eyeLookDown_R")
		return BlendShapeKey::EYELOOKDOWN_R;
	if (str == "eyeLookIn_R")
		return BlendShapeKey::EYELOOKIN_R;
	if (str == "eyeLookOut_R")
		return BlendShapeKey::EYELOOKOUT_R;
	if (str == "eyeLookUp_R")
		return BlendShapeKey::EYELOOKUP_R;
	if (str == "eyeWide_R")
		return BlendShapeKey::EYEWIDE_R;
	if (str == "eyeSquint_R")
		return BlendShapeKey::EYESQUINT_R;
	if (str == "eyeBlink_R")
		return BlendShapeKey::EYEBLINK_R;
	if (str == "browDown_R")
		return BlendShapeKey::BROWDOWN_R;
	if (str == "browInnerUp_R")
		return BlendShapeKey::BROWINNERUP_R;
	if (str == "browOuterUp_R")
		return BlendShapeKey::BROWOUTERUP_R;
	if (str == "headLeft")
		return BlendShapeKey::HEADLEFT;
	if (str == "headRight")
		return BlendShapeKey::HEADRIGHT;
	if (str == "headUp")
		return BlendShapeKey::HEADUP;
	if (str == "headDown")
		return BlendShapeKey::HEADDOWN;
	if (str == "headRollLeft")
		return BlendShapeKey::HEADROLLLEFT;
	if (str == "headRollRight")
		return BlendShapeKey::HEADROLLRIGHT;
	if (str == "EyeBlinkLeft")
		return BlendShapeKey::EYEBLINKLEFT;
	if (str == "EyeBlinkRight")
		return BlendShapeKey::EYEBLINKRIGHT;

	return BlendShapeKey::UNKNOWN;
}

struct BlendShape {
	BlendShapeKey k;
	double v;

	QJsonObject toJson() const
	{
		QJsonObject obj;
		obj["k"] = blendShapeKeyToString(k);
		obj["v"] = v;
		return obj;
	}

	static BlendShape fromJson(const QJsonObject &obj)
	{
		BlendShape bs;
		bs.k = blendShapeKeyFromString(obj["k"].toString());
		if (bs.k == BlendShapeKey::UNKNOWN)
			return {};

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
	QMap<BlendShapeKey, BlendShape> blendShapes;

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
			QJsonObject newBs = bs.toJson();
			if ((newBs["k"]) != "")
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
				BlendShape bsToAdd = BlendShape::fromJson(item.toObject());
				trackingData.blendShapes.insert(bsToAdd.k, bsToAdd);
			}
		}

		return trackingData;
	}
};

enum ComparisonType { EQ, LT, LTEQ, GT, GTEQ };

struct BlendShapeRule {
	BlendShapeKey key;
	ComparisonType compareType;
	double compareValue;

	bool result(BlendShape blendShape)
	{
		if (blendShape.k == BlendShapeKey::UNKNOWN)
			return false;

		double valueToCompare = blendShape.v;

		switch (compareType) {
		case ComparisonType::EQ:
			return valueToCompare == compareValue;
		case ComparisonType::LT:
			return valueToCompare < compareValue;
		case ComparisonType::LTEQ:
			return valueToCompare <= compareValue;
		case ComparisonType::GT:
			return valueToCompare > compareValue;
		case ComparisonType::GTEQ:
			return valueToCompare >= compareValue;

		default:
			return false;
			break;
		}
	}
};

struct PoseImageData {
	QString imageUrl;
	MovablePixmapItem *pixmapItem;

	PoseImageData(const QString &url = "", MovablePixmapItem *item = nullptr) : imageUrl(url), pixmapItem(item) {};
};

// For poses we will concentrate first on
// Head rotation (looking left or right)
// Eye blink
// Mouth type
struct Pose {
	QString poseId;

	// Container mapping PoseImage enums to PoseImageData
	std::array<PoseImageData, static_cast<size_t>(PoseImage::COUNT)> poseImages;

	Vector3 bodyPosition = {0, 0, 0};
	Vector3 eyesPosition = {0, 0, 0};
	Vector3 mouthPosition = {0, 0, 0};

	std::vector<BlendShapeRule> blendShapesRuleList;

	// Constructor
	Pose()
	{
		// Initialize the array with default PoseImageData
		for (size_t i = 0; i < poseImages.size(); ++i) {
			poseImages[i] = PoseImageData();
		}
	}

	PoseImageData *getPoseImageData(PoseImage pose)
	{
		size_t index = static_cast<size_t>(pose);
		if (index < poseImages.size()) {
			return &poseImages[index];
		}
		// Return nullptr if the pose is out of range
		return nullptr;
	}

	QSharedPointer<Pose> clone() const
	{
		return QSharedPointer<Pose>(new Pose(*this)); // Utilizes the copy constructor
	}

	Pose(const Pose &other)
		: poseId(other.poseId),
		  poseImages(other.poseImages),
		  bodyPosition(other.bodyPosition),
		  eyesPosition(other.eyesPosition),
		  mouthPosition(other.mouthPosition),
		  blendShapesRuleList(other.blendShapesRuleList)
	{
	}

	Pose &operator=(const Pose &other)
	{
		if (this == &other) {
			return *this; // Handle self-assignment
		}

		// Copy each member variable
		poseId = other.poseId;
		poseImages = other.poseImages;
		bodyPosition = other.bodyPosition;
		eyesPosition = other.eyesPosition;
		mouthPosition = other.mouthPosition;
		blendShapesRuleList = other.blendShapesRuleList;

		return *this;
	}

	bool shouldUsePose(QMap<BlendShapeKey, BlendShape> blendShapes)
	{
		for (auto rule : blendShapesRuleList) {
			BlendShape bs = blendShapes.value(rule.key, {BlendShapeKey::UNKNOWN, 0.0});
			if (bs.k != BlendShapeKey::UNKNOWN) {
				bool result = rule.result(bs);
				if (result)
					return true;
			}
		}
		return false;
	}

	// Convert this Pose object to a QJsonObject
	QJsonObject toJson() const
	{
		QJsonObject obj;
		obj["poseId"] = poseId;

		QJsonObject imagesObj;
		for (size_t i = 0; i < poseImages.size(); ++i) {
			const PoseImageData &data = poseImages[i];
			QJsonObject imageData;
			imageData["imageUrl"] = data.imageUrl;

			QString key = QString::number(static_cast<int>(static_cast<PoseImage>(i)));
			imagesObj[key] = imageData; // Assigning imageData to imagesObj
		}
		obj["poseImages"] = imagesObj;

		obj["bodyPosition"] = bodyPosition.toJson();
		obj["eyesPosition"] = eyesPosition.toJson();
		obj["mouthPosition"] = mouthPosition.toJson();

		// Convert blendShapesRuleList to a QJsonArray
		QJsonArray rulesArray;
		for (const auto &rule : blendShapesRuleList) {
			QJsonObject ruleObj;
			ruleObj["key"] = static_cast<int>(rule.key); // or store as string if you prefer
			ruleObj["compareType"] = static_cast<int>(rule.compareType);
			ruleObj["compareValue"] = rule.compareValue;
			rulesArray.append(ruleObj);
		}
		obj["blendShapesRuleList"] = rulesArray;

		return obj;
	}

	// Create a Pose from a QJsonObject
	static Pose fromJson(const QJsonObject &obj)
	{
		Pose pose;
		pose.poseId = obj["poseId"].toString();

		QJsonObject imagesObj = obj["poseImages"].toObject();
		for (size_t i = 0; i < pose.poseImages.size(); ++i) {
			PoseImage poseEnum = static_cast<PoseImage>(i);
			QString key = QString::number(static_cast<int>(poseEnum));
			if (imagesObj.contains(key)) {
				QJsonObject imageData = imagesObj[key].toObject();
				pose.poseImages[i].imageUrl = imageData["imageUrl"].toString();
			}
		}

		QJsonObject bodyPosObj = obj["bodyPosition"].toObject();
		pose.bodyPosition = Vector3::fromJson(bodyPosObj);

		QJsonObject eyePosObj = obj["eyesPosition"].toObject();
		pose.eyesPosition = Vector3::fromJson(eyePosObj);

		QJsonObject mouthPosObj = obj["mouthPosition"].toObject();
		pose.mouthPosition = Vector3::fromJson(mouthPosObj);

		QJsonArray rulesArray = obj["blendShapesRuleList"].toArray();
		for (auto ruleValue : rulesArray) {
			if (ruleValue.isObject()) {
				QJsonObject ruleObj = ruleValue.toObject();
				BlendShapeRule rule;
				rule.key = static_cast<BlendShapeKey>(ruleObj["key"].toInt());
				rule.compareType = static_cast<ComparisonType>(ruleObj["compareType"].toInt());
				rule.compareValue = ruleObj["compareValue"].toDouble();
				pose.blendShapesRuleList.push_back(rule);
			}
		}
		return pose;
	}
};

struct TrackerDataStruct {
	QString trackerId;
	QString selectedImageSource;
	QString destIpAddress = "192.0.0.0";
	int destPort = 21412;
	int port = 21412;
	QList<QSharedPointer<Pose>> poseList;
	bool isEnabled = false;

	QString poseListToJsonString()
	{
		QJsonObject obj;

		// Convert blendShapesRuleList to a QJsonArray
		QJsonArray poseArray;
		for (const auto &posePtr : poseList) {
			if (posePtr) {
				QJsonObject poseObj = posePtr->toJson();
				poseArray.append(poseObj);
			} else {
				obs_log(LOG_WARNING, "Encountered a null Pose pointer during JSON serialization.");
			}
		}

		obj["poseArray"] = poseArray;
		QJsonDocument doc(obj);
		return doc.toJson(QJsonDocument::Compact);
	}

	void jsonStringToPoseList(QString jsonString)
	{
		QByteArray jsonData = jsonString.toUtf8();

		QJsonParseError parseError;
		QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

		if (parseError.error != QJsonParseError::NoError) {
			obs_log(LOG_ERROR, "Failed to parse JSON: %s", parseError.errorString().toStdString().c_str());
			return;
		}

		QJsonObject obj = doc.object();

		QJsonValue poseArrayValue = obj.value("poseArray");
		if (!poseArrayValue.isArray()) {
			obs_log(LOG_ERROR, "JSON does not contain a valid 'poseArray'.");
			return;
		}

		QJsonArray poseArray = poseArrayValue.toArray();
		poseList.clear();

		for (auto value : poseArray) {
			if (value.isObject()) {
				QJsonObject poseObj = value.toObject();

				Pose p = Pose::fromJson(poseObj);
				QSharedPointer<Pose> posePtr = QSharedPointer<Pose>::create();
				*posePtr = p; // Assign the Pose data to the shared pointer

				poseList.push_back(posePtr);
			}
		}
	}
};

QString GenerateUniqueID();
bool FileExists(QString filePath);

#endif // TRACKERUTILS_H
