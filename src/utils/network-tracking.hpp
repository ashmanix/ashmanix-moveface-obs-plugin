#ifndef NETWORKTRACKING_H
#define NETWORKTRACKING_H

#include <optional>
#include <QWidget>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QHostAddress>

#include <obs.h>

#include "plugin-support.h"

class NetworkTracking : public QWidget {
	Q_OBJECT

public:
	NetworkTracking(QWidget *parent = nullptr, quint16 port = 21214);
	~NetworkTracking();

	bool UpdateConnection(quint16 newPort);
	bool SendUDPData(QString destIpAddress, quint16 destPort, QByteArray data);
	static QString GetIpAddress();

signals:
	void ReceivedData(QString data);

private slots:
	void ProcessPendingDatagrams();

private:
	QUdpSocket *udpSocket = nullptr;
	quint16 port;

	bool StartConnection();
	// QString GetCleanAddress(const QHostAddress &address);
};

#endif // NETWORKTRACKING_H
