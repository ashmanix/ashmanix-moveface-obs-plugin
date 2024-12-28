#ifndef NETWORKTRACKING_H
#define NETWORKTRACKING_H

#include <optional>
#include <QWidget>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QJsonDocument>
#include <QTimer>

#include <obs.h>

#include "plugin-support.h"
#include "tracker-utils.hpp"

class NetworkTracking : public QWidget {
	Q_OBJECT

public:
	NetworkTracking(QWidget *parent = nullptr, quint16 port = 21412, QString destIpAddress = "",
			quint16 destPort = 21412);
	~NetworkTracking();

	bool UpdateConnection(quint16 newPort, std::optional<QString> in_destIpAddress = std::nullopt,
			      std::optional<quint16> in_destPort = std::nullopt);
	bool SendUDPData(QString destIpAddress, quint16 destPort, QByteArray data);
	static QString GetIpAddresses();

	void StopDataRequestTimer();

signals:
	void ReceivedData(VTubeStudioTrackingData data);
	void ConnectionToggle(bool isConnected);
	void ConnectionErrorToggle(bool isError);

private slots:
	void ProcessReceivedTrackingData();

private:
	QTimer *networkTrackingDataRequestTimer = nullptr;
	const int MAXERRORCOUNT = 3;
	int networkErrorCount = 0;

	QTimer *connectionTimer = nullptr;
	const int TRACKINGDATAPERIODINSECS = 5;

	QUdpSocket *udpSocket = nullptr;
	quint16 port;
	QString destIpAddress;
	quint16 destPort;

	bool isConnected = false;

	QJsonObject initiateTrackingObject = {
		{"messageType", "iOSTrackingDataRequest"},
		{"time", TRACKINGDATAPERIODINSECS},
		{"sentBy", "vTuberApp"},
	};

	bool StartConnection();
	void ResetConnectionTimer();
	void SetSendPeriodicData();
	void RequestTrackingData(quint16 destPort, QString destIpAddress);
};

#endif // NETWORKTRACKING_H
