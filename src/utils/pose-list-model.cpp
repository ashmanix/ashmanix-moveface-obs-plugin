#include "pose-list-model.hpp"

PoseListModel::PoseListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int PoseListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_poses.size();
}

QVariant PoseListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_poses.size())
        return QVariant();

    // For a simple list, typically DisplayRole = the textual label you want to see:
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        // For example, display the Pose ID in the list:
        return m_poses[index.row()].poseId;
    }

    // Other roles (ToolTipRole, UserRole, etc.) can go here if needed
    return QVariant();
}

Qt::ItemFlags PoseListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);
    if (index.isValid()) {
        // Make it editable
        return defaultFlags | Qt::ItemIsEditable;
    }
    return defaultFlags;
}

// ------------------- Public API for modifying the data ------------------ //

void PoseListModel::addPose(const Pose &pose)
{
    // Insert at the end
    beginInsertRows(QModelIndex(), m_poses.size(), m_poses.size());
    m_poses.append(pose);
    endInsertRows();
}

void PoseListModel::insertPose(int row, const Pose &pose)
{
    if (row < 0) row = 0;
    if (row > m_poses.size()) row = m_poses.size();

    beginInsertRows(QModelIndex(), row, row);
    m_poses.insert(row, pose);
    endInsertRows();
}

void PoseListModel::removePose(int row)
{
    if (row < 0 || row >= m_poses.size())
        return;
    beginRemoveRows(QModelIndex(), row, row);
    m_poses.removeAt(row);
    endRemoveRows();
}

Pose PoseListModel::getPose(int row) const
{
    if (row < 0 || row >= m_poses.size())
        return Pose();
    return m_poses[row];
}

QVector<Pose> PoseListModel::getAllPoses() const
{
    return m_poses;
}
