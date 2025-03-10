#include "network-tracking.h"
#include <obs.h>
#include "plugin-support.h"

NetworkTracking::NetworkTracking(QWidget *parent, quint16 in_port, QString in_destIpAddress, quint16 in_destPort)
	: QWidget(parent),
	  port(in_port),
	  destIpAddress(in_destIpAddress),
	  destPort(in_destPort)
{
	udpSocket = QSharedPointer<QUdpSocket>::create(this);
}

NetworkTracking::~NetworkTracking()
{
	if (udpSocket)
		udpSocket->close();
	if (networkTrackingDataRequestTimer)
		networkTrackingDataRequestTimer->stop();
	if (connectionTimer) {
		connectionTimer->stop();
		emit connectionToggle(false);
	}
	obs_log(LOG_INFO, "Closing network connection");
}

bool NetworkTracking::updateConnection(quint16 newPort, std::optional<QString> in_destIpAddress,
				       std::optional<quint16> in_destPort)
{
	port = newPort;

	if (in_destIpAddress.has_value())
		destIpAddress = *in_destIpAddress;

	if (in_destPort.has_value())
		destPort = *in_destPort;

	return startConnection();
}

bool NetworkTracking::sendUDPData(QString d_IpAddress, quint16 d_Port, QByteArray data)
{
	bool result = false;
	if (udpSocket) {
		auto destAddress = QHostAddress(d_IpAddress);
		qint64 bytesSent = udpSocket->writeDatagram(data, destAddress, d_Port);
		if (bytesSent == -1) {
			// An error occurred while sending
			QUdpSocket::SocketError socketError = udpSocket->error();
			QString errorString = udpSocket->errorString();
			obs_log(LOG_ERROR, "Failed to send UDP datagram: %s (Error Code: %d)",
				errorString.toStdString().c_str(), static_cast<int>(socketError));
		} else {
			result = true;
		}
	} else {
		obs_log(LOG_WARNING, "Cannot send data as no UDP socket set!");
		result = false;
	}
	return result;
}

QString NetworkTracking::getIpAddresses()
{
	// Get all network interfaces
	QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
	QString ipAddresses = "";
	for (const QHostAddress &address : addresses) {
		// We only want IPv4 addresses, and ignore loopback (127.0.0.1)
		if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback()) {
			if (ipAddresses.isEmpty()) {
				ipAddresses += address.toString();
			} else {
				ipAddresses += ", " + address.toString();
			}
		}
	}
	return ipAddresses;
}

bool NetworkTracking::startConnection()
{

	if (udpSocket)
		udpSocket->close();

	if (!udpSocket->bind(QHostAddress::Any, port)) {
		obs_log(LOG_WARNING, "Failed to bind to port: %d", port);
		return false;
	}

	connect(udpSocket.data(), &QUdpSocket::readyRead, this, &NetworkTracking::processReceivedTrackingData);

	resetConnectionTimer();
	setSendPeriodicData();

	obs_log(LOG_INFO, "Server listening on port: %d", port);
	return true;
}

void NetworkTracking::processReceivedTrackingData()
{
	while (udpSocket->hasPendingDatagrams()) {
		QByteArray datagram;
		datagram.resize(int(udpSocket->pendingDatagramSize()));
		QHostAddress sender;
		quint16 senderPort;

		udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

		QJsonParseError parseError;
		QJsonDocument jsonDocument = QJsonDocument::fromJson(datagram.data(), &parseError);

		if (parseError.error == QJsonParseError::NoError && jsonDocument.isObject()) {
			QJsonObject jsonObj = jsonDocument.object();
			if (isConnected == false) {
				isConnected = true;
				emit connectionToggle(isConnected);
			}

			VTubeStudioData receivedTrackingData = VTubeStudioData::fromJson(jsonObj);

			if (receivedTrackingData.getFaceFound() == true) {
				emit receivedData(receivedTrackingData);
			}
		}
	}
}

void NetworkTracking::resetConnectionTimer()
{
	// We send out the signal periodically as required by vTubeStudio
	if (!connectionTimer) {
		connectionTimer = QSharedPointer<QTimer>::create();
		QObject::connect(connectionTimer.data(), &QTimer::timeout, [this]() {
			isConnected = false;
			emit connectionToggle(isConnected);
		});
	}
	//Reset timer
	connectionTimer->stop();
	int timePeriod = TRACKINGDATAPERIODINSECS - 1 >= 0 ? (TRACKINGDATAPERIODINSECS - 1) * 1000 : 1000;
	connectionTimer->start(timePeriod);
}

void NetworkTracking::setSendPeriodicData()
{
	// We send out the signal periodically as required by vTubeStudio to continue
	// receiving tracking data
	if (!networkTrackingDataRequestTimer) {
		networkTrackingDataRequestTimer = QSharedPointer<QTimer>::create();
		QObject::connect(networkTrackingDataRequestTimer.data(), &QTimer::timeout, this,
				 [this]() { requestTrackingData(destPort, destIpAddress); });
	}
	//Reset timer
	networkTrackingDataRequestTimer->stop();
	int timePeriod = TRACKINGDATAPERIODINSECS - 1 >= 0 ? (TRACKINGDATAPERIODINSECS - 1) * 1000 : 1000;
	networkTrackingDataRequestTimer->start(timePeriod);

	networkErrorCount = 0;
}

void NetworkTracking::requestTrackingData(quint16 dPort, QString dIpAddress)
{
	if (udpSocket) {
		QJsonArray ports;
		ports.append(port);
		initiateTrackingObject["ports"] = ports;

		QJsonDocument jsonDoc(initiateTrackingObject);
		QByteArray jsonString = jsonDoc.toJson(QJsonDocument::Indented);
		bool result = sendUDPData(dIpAddress, dPort, jsonString);
		if (!result) {
			networkErrorCount++;
		} else {
			networkErrorCount = 0;
		}
		if (networkErrorCount >= MAXERRORCOUNT) {
			emit connectionErrorToggle(true);
			obs_log(LOG_WARNING, "Send connection max errors reached, cancelling send messages");
			networkTrackingDataRequestTimer->stop();
		}
	}
}
