#ifndef TRACKERWORKER_H
#define TRACKERWORKER_H

#include <QObject>
#include <QImage>
#include <QSharedPointer>
#include <QMutex>

#include "network-tracking.h"
#include "../../classes/tracking/vtube-studio-data.h"
#include "../../classes/tracking/tracker-data.h"


// The purpose of this tracker worker is to receive data and process pose images
// on a separate thread to the main UI thread to prevent any lag on the UI
class TrackerWorker : public QObject
{
    Q_OBJECT
public:
    explicit TrackerWorker(quint16 port,
                    const QString &destIpAddress,
                    quint16 destPort,
                    QObject *parent = nullptr);
    ~TrackerWorker();

public slots:
    void start();
    void stop();
    void processTrackingData(const VTubeStudioData &data);

    // New slots to update settings
    void updateConnection(quint16 newPort, const QString &newDestIpAddress, quint16 newDestPort);
    void updateTrackerData(const QSharedPointer<TrackerData> &newTrackerData);

signals:
    void imageReady(const QImage &image); // Signal emitted when image is ready
    void finished();
    void errorOccurred(bool isError);
    void connectionToggle(bool isConnected);

private:
    QSharedPointer<NetworkTracking> networkTracking;
    bool running;
    QMutex m_mutex; // Mutex to protect settings

    QSharedPointer<TrackerData> m_trackerData;
    QSharedPointer<Pose> findAppropriatePose(const VTubeStudioData &data) const;
};

#endif // TRACKERWORKER_H
