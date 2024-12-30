#pragma once

#include <QAbstractListModel>
#include <QVector>
#include "tracker-utils.hpp" 

class PoseListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit PoseListModel(QObject *parent = nullptr);

    // Required overrides:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Optional override if you want custom flags or editing:
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Convenience methods for managing the list:
    void addPose(const Pose &pose);
    void insertPose(int row, const Pose &pose);
    void removePose(int row);

    // Optionally, a way to retrieve the entire list or a single pose:
    Pose getPose(int row) const;
    QVector<Pose> getAllPoses() const;

private:
    QVector<Pose> m_poses;
};
