#ifndef NETWORKTRACKING_H
#define NETWORKTRACKING_H

#include <optional>
#include <QWidget>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QSharedPointer>

#include "../../classes/tracking/vtube-studio-data.h"

#include "plugin-support.h"
#include <obs.h>

class NetworkTracking : public QWidget {
	Q_OBJECT

public:
	NetworkTracking(QWidget *parent = nullptr, quint16 port = 21412, QString destIpAddress = "",
			quint16 destPort = 21412);
	~NetworkTracking() override;

	bool updateConnection(quint16 newPort, std::optional<QString> in_destIpAddress = std::nullopt,
			      std::optional<quint16> in_destPort = std::nullopt);
	bool sendUDPData(QString destIpAddress, quint16 destPort, QByteArray data);
	static QString getIpAddresses();
	bool startConnection();

signals:
	void receivedData(VTubeStudioData data);
	void connectionToggle(bool isConnected);
	void connectionErrorToggle(bool isError);

private slots:
	void processReceivedTrackingData();

private:
	QSharedPointer<QTimer> networkTrackingDataRequestTimer = nullptr;
	const int MAXERRORCOUNT = 3;
	int networkErrorCount = 0;

	QSharedPointer<QTimer> connectionTimer = nullptr;
	const int TRACKINGDATAPERIODINSECS = 5;

	QSharedPointer<QUdpSocket> udpSocket = nullptr;
	quint16 port;
	QString destIpAddress;
	quint16 destPort;

	bool isConnected = false;

	QJsonObject initiateTrackingObject = {
		{"messageType", "iOSTrackingDataRequest"},
		{"time", TRACKINGDATAPERIODINSECS},
		{"sentBy", "vTuberApp"},
	};

	void resetConnectionTimer();
	void setSendPeriodicData();
	void requestTrackingData(quint16 destPort, QString destIpAddress);
};

#endif // NETWORKTRACKING_H
