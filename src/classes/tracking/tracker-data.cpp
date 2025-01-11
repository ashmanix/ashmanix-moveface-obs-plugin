#include "tracker-data.h"
#include "../poses/pose.h"

TrackerData::TrackerData(QString trackerId, QString selectedImageSource, QString destIpAddress)
	: m_trackerId(trackerId),
	  m_selectedImageSource(selectedImageSource),
	  m_destIpAddress(destIpAddress)
{
}

QString TrackerData::getTrackerId()
{
	return m_trackerId;
}

QString TrackerData::getSelectedImageSource()
{
	return m_selectedImageSource;
}

QString TrackerData::getDestinationIpAddress()
{
	return m_destIpAddress;
}

int TrackerData::getDestinationPort()
{
	return m_destPort;
}

int TrackerData::getPort()
{
	return m_port;
}

bool TrackerData::getIsEnabled()
{
	return m_isEnabled;
}

QSharedPointer<Pose> TrackerData::getPoseAt(int index)
{
	return m_poseList[index];
}

QList<QSharedPointer<Pose>> TrackerData::getPoseList()
{
	return m_poseList;
}

void TrackerData::setTrackerId(QString newId)
{
	m_trackerId = newId;
}

void TrackerData::setSelectedImageSource(QString newImageSource)
{
	m_selectedImageSource = newImageSource;
}

void TrackerData::setDestinationIpAddress(QString newDestIpAddress)
{
	m_destIpAddress = newDestIpAddress;
}
void TrackerData::setDestinationPort(int newDestPort)
{
	m_destPort = newDestPort;
}

void TrackerData::setPort(int newPort)
{
	m_port = newPort;
}

void TrackerData::setIsEnabled(bool isEnabled)
{
	obs_log(LOG_INFO, "is enabled: %s", isEnabled ? "true" : "false");
	m_isEnabled = isEnabled;
}

void TrackerData::setPoseAt(int index, QSharedPointer<Pose> newPose)
{
	m_poseList[index] = newPose->clone();
}

void TrackerData::copyListToPoseList(QList<QSharedPointer<Pose>> newList)
{
	m_poseList.clear();
	for (const QSharedPointer<Pose> &posePtr : newList) {
		if (posePtr) {
			m_poseList.append(posePtr->clone());
		}
	}
}

QString TrackerData::poseListToJsonString() const
{

	QJsonObject obj;

	// Convert blendShapesRuleList to a QJsonArray
	QJsonArray poseArray;
	for (const auto &posePtr : m_poseList) {
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

void TrackerData::jsonStringToPoseList(const QString &jsonString)
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
	m_poseList.clear();

	for (auto value : poseArray) {
		if (value.isObject()) {
			QJsonObject poseObj = value.toObject();

			Pose p = Pose::fromJson(poseObj);
			QSharedPointer<Pose> posePtr = QSharedPointer<Pose>::create();
			*posePtr = p; // Assign the Pose data to the shared pointer

			m_poseList.push_back(posePtr);
		}
	}
}
