#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H

#include <optional>
#include <QWidget>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QHostAddress>

#include <obs.h>

#include "plugin-support.h"

class NetworkConnection : public QWidget {
	Q_OBJECT

public:
	NetworkConnection(QWidget *parent = nullptr, quint16 port = 21214);
	~NetworkConnection();

    bool UpdateConnection(quint16 newPort);
    void SendUDPData(QString destIpAddress, quint16 destPort, QByteArray data);
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

#endif // NETWORKCONNECTION_H
