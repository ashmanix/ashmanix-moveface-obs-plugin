#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QImage>
#include <QSharedPointer>
#include <QMutex>
#include "network-tracking.h" // Ensure this path is correct
#include "vtube-studio-data.h"  // Include your data class

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(quint16 port,
                    const QString &destIpAddress,
                    quint16 destPort,
                    QObject *parent = nullptr);
    ~Worker();

public slots:
    void start(); // Slot to start the worker
    void stop();  // Slot to stop the worker
    void processTrackingData(const VTubeStudioData &data); // Slot to process incoming data

    // New slots to update settings
    void updateConnection(quint16 newPort, const QString &newDestIpAddress, quint16 newDestPort);

signals:
    void imageReady(const QImage &image); // Signal emitted when image is ready
    void finished();                       // Signal emitted when worker finishes
    void errorOccurred(const QString &error); // Signal emitted on error

private:
    QSharedPointer<NetworkTracking> networkTracking;
    bool running;
    QMutex m_mutex; // Mutex to protect settings

    // Add any additional private members or helper functions here
};

#endif // WORKER_H
