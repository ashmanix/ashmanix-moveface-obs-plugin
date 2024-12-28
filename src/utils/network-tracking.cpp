#include "network-tracking.hpp"

NetworkTracking::NetworkTracking(QWidget *parent, quint16 in_port, QString in_destIpAddress, quint16 in_destPort)
	: QWidget(parent)
{
	udpSocket = new QUdpSocket(this);
	port = in_port;
	destIpAddress = in_destIpAddress;
	destPort = in_destPort;
	StartConnection();
}

NetworkTracking::~NetworkTracking()
{
	if (udpSocket)
		udpSocket->close();
	if (networkTrackingDataRequestTimer)
		networkTrackingDataRequestTimer->stop();
	if (connectionTimer) {
		connectionTimer->stop();
		emit ConnectionToggle(false);
	}
	obs_log(LOG_INFO, "Closing network connection");
}

bool NetworkTracking::UpdateConnection(quint16 newPort, std::optional<QString> in_destIpAddress,
				       std::optional<quint16> in_destPort)
{
	port = newPort;

	if (in_destIpAddress.has_value())
		destIpAddress = *in_destIpAddress;

	if (in_destPort.has_value())
		destPort = *in_destPort;

	return StartConnection();
}

bool NetworkTracking::SendUDPData(QString destIpAddress, quint16 destPort, QByteArray data)
{
	bool result = false;
	if (udpSocket) {
		QHostAddress destAddress = QHostAddress(destIpAddress);
		qint64 bytesSent = udpSocket->writeDatagram(data, destAddress, destPort);
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

QString NetworkTracking::GetIpAddresses()
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

void NetworkTracking::ProcessReceivedTrackingData()
{
	while (udpSocket->hasPendingDatagrams()) {
		QByteArray datagram;
		datagram.resize(int(udpSocket->pendingDatagramSize()));
		QHostAddress sender;
		quint16 senderPort;

		udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

		// obs_log(LOG_INFO,  (QString("Received datagram from %1 : %1").arg(sender.toString(), senderPort)).toStdString().c_str());
		// obs_log(LOG_INFO, (QString("Data: %1").arg(datagram)).toStdString().c_str());

		QJsonParseError parseError;
		QJsonDocument jsonDocument = QJsonDocument::fromJson(datagram.data(), &parseError);

		if (parseError.error == QJsonParseError::NoError) {

			if (jsonDocument.isObject()) {
				QJsonObject jsonObj = jsonDocument.object();
				if (isConnected == false) {
					isConnected = true;
					emit ConnectionToggle(isConnected);
				}

				VTubeStudioTrackingData receivedTrackingData =
					VTubeStudioTrackingData::fromJson(jsonObj);

				if (receivedTrackingData.faceFound == true) {
					emit ReceivedData(receivedTrackingData);
				}
			}
		}
	}
}

bool NetworkTracking::StartConnection()
{

	if (udpSocket)
		udpSocket->close();

	if (!udpSocket->bind(QHostAddress::Any, port)) {
		obs_log(LOG_WARNING, "Failed to bind to port: %d", port);
		return false;
	}

	connect(udpSocket, &QUdpSocket::readyRead, this, &NetworkTracking::ProcessReceivedTrackingData);

	ResetConnectionTimer();
	SetSendPeriodicData();

	obs_log(LOG_INFO, "Server listening on port: %d", port);
	return true;
}

void NetworkTracking::ResetConnectionTimer()
{
	// We send out the signal periodically as required by vTubeStudio
	if (!connectionTimer) {
		connectionTimer = new QTimer();
		QObject::connect(connectionTimer, &QTimer::timeout, [this]() {
			isConnected = false;
			emit ConnectionToggle(isConnected);
		});
	}
	//Reset timer
	connectionTimer->stop();
	int timePeriod = TRACKINGDATAPERIODINSECS - 1 >= 0 ? (TRACKINGDATAPERIODINSECS - 1) * 1000 : 1000;
	connectionTimer->start(timePeriod);
}

void NetworkTracking::SetSendPeriodicData()
{
	// We send out the signal periodically as required by vTubeStudio to continue
	// receiving tracking data
	if (!networkTrackingDataRequestTimer) {
		networkTrackingDataRequestTimer = new QTimer();
		QObject::connect(networkTrackingDataRequestTimer, &QTimer::timeout, this,
				 [this]() { RequestTrackingData(destPort, destIpAddress); });
	}
	//Reset timer
	networkTrackingDataRequestTimer->stop();
	int timePeriod = TRACKINGDATAPERIODINSECS - 1 >= 0 ? (TRACKINGDATAPERIODINSECS - 1) * 1000 : 1000;
	networkTrackingDataRequestTimer->start(timePeriod);

	networkErrorCount = 0;
}

void NetworkTracking::RequestTrackingData(quint16 destPort, QString destIpAddress)
{
	if (udpSocket) {
		QJsonArray ports;
		ports.append(port);
		initiateTrackingObject["ports"] = ports;

		QJsonDocument jsonDoc(initiateTrackingObject);
		QByteArray jsonString = jsonDoc.toJson(QJsonDocument::Indented);
		bool result = SendUDPData(destIpAddress, destPort, jsonString);
		if (!result) {
			networkErrorCount++;
		} else {
			networkErrorCount = 0;
		}
		if (networkErrorCount >= MAXERRORCOUNT) {
			emit ConnectionErrorToggle(true);
			obs_log(LOG_WARNING, "Send connection max errors reached, cancelling send messages");
			networkTrackingDataRequestTimer->stop();
		}
	}
}
