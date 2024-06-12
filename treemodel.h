/*
 * Thanks for
 * https://stackoverflow.com/questions/59989087/treeview-for-filesystem-with-checkboxes-in-qt
 *
 */

#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QFileSystemModel>
#include <QSet>
#include <QMap>

class TreeModel : public QFileSystemModel {
    Q_OBJECT
public:
    explicit TreeModel(QObject *parent = Q_NULLPTR);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;

    QMap<QPersistentModelIndex, Qt::CheckState> checklist;

signals:
    void setProgressBarRange(int, int);
    void setProgressBarValue(int);

private:
    bool setNodeCheckState(const QModelIndex & index, const QVariant & value, int role);
    bool setChildNodesCheck (const QModelIndex & index, const QVariant & value);

    bool setParentNodeCheckState(const QModelIndex& currentNodeIndex);
    Qt::CheckState getCheckStateAccordingToChildren(const QModelIndex& currentNodeIndex);
};

#endif // TREEMODEL_H
