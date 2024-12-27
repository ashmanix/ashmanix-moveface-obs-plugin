#include "network-tracking.hpp"

NetworkTracking::NetworkTracking(QWidget *parent, quint16 in_port) : QWidget(parent)
{
	udpSocket = new QUdpSocket(this);
	port = in_port;
	StartConnection();
}

NetworkTracking::~NetworkTracking()
{
	if (udpSocket)
		udpSocket->close();
	obs_log(LOG_INFO, "Closing network connection");
}

bool NetworkTracking::UpdateConnection(quint16 newPort)
{
	port = newPort;
	return StartConnection();
}

bool NetworkTracking::SendUDPData(QString destIpAddress, quint16 destPort, QByteArray data)
{
	if (udpSocket) {
		QHostAddress destAddress = QHostAddress(destIpAddress);
		qint64 bytesSent = udpSocket->writeDatagram(data, destAddress, destPort);
		if (bytesSent == -1) {
			// An error occurred while sending
			QUdpSocket::SocketError socketError = udpSocket->error();
			QString errorString = udpSocket->errorString();
			obs_log(LOG_ERROR, "Failed to send UDP datagram: %s (Error Code: %d)",
				errorString.toStdString().c_str(), static_cast<int>(socketError));
			return false;
		}
		// else {
		// 	// Data sent successfully
		// 	obs_log(LOG_INFO, "Sent %lld bytes to %s:%d", bytesSent, destIpAddress.toStdString().c_str(),
		// 		destPort);
		// }
	} else {
		obs_log(LOG_WARNING, "Cannot send data as no UDP socket set!");
		return false;
	}
	return true;
}

QString NetworkTracking::GetIpAddress()
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

void NetworkTracking::ProcessPendingDatagrams()
{
	while (udpSocket->hasPendingDatagrams()) {
		QByteArray datagram;
		datagram.resize(int(udpSocket->pendingDatagramSize()));
		QHostAddress sender;
		quint16 senderPort;

		udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

		// obs_log(LOG_INFO,  (QString("Received datagram from %1 : %1").arg(sender.toString(), senderPort)).toStdString().c_str());
		// obs_log(LOG_INFO, (QString("Data: %1").arg(datagram)).toStdString().c_str());

		emit ReceivedData(datagram);

		// Respond to the client
		// QByteArray response = "Hello, Client!";
		// udpSocket->writeDatagram(response, sender, senderPort);
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

	connect(udpSocket, &QUdpSocket::readyRead, this, &NetworkTracking::ProcessPendingDatagrams);
	obs_log(LOG_INFO, "Server listening on port: %d", port);
	return true;
}
