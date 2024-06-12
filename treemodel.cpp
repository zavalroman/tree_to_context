#include "treemodel.h"
#include <QApplication>

#include <QTimer>

TreeModel::TreeModel(QObject *parent) : QFileSystemModel(parent)
{

}

void TreeModel::selectAll(const QModelIndex& rootIndex)
{
    int rootItemsCount = rowCount(rootIndex);
    //emit setProgressBarRange(0, rootItemsCount);

    totalCount = 0;
    currentStep = 0;
    emit setProgressBarValue(0);

    for(int i = 0; i < rootItemsCount; i++) {
        //emit setProgressBarValue(i + 1);
        setNodeCheckState(QFileSystemModel::index(i, 0, rootIndex), Qt::Checked, Qt::CheckStateRole);
    }
}

void TreeModel::deselectAll(const QModelIndex &rootIndex)
{
    int rootItemsCount = rowCount(rootIndex);
    for(int i = 0; i < rootItemsCount; i++) {
        setNodeCheckState(QFileSystemModel::index(i, 0, rootIndex), Qt::Unchecked, Qt::CheckStateRole);
    }
}

QVariant TreeModel::data(const QModelIndex& index, int role) const {
    if (role == Qt::CheckStateRole) {
        if(checklist.contains(index)) {
            return checklist[index];
        } else {
            return Qt::Unchecked;
        }
    }
    return QFileSystemModel::data(index, role);
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const {
    if (index.column() > 0) {
        return QFileSystemModel::flags(index);
    } else {
        return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
    }
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (role == Qt::CheckStateRole) {
        totalCount = 0;
        currentStep = 0;
        emit setProgressBarValue(0);

        setNodeCheckState(index, value, role);
        setParentNodeCheckState(index);
    }
    this->setReadOnly(false);
    QApplication::restoreOverrideCursor();

    emit setProgressBarRange(0, 1);
    emit setProgressBarValue(1);
    return QFileSystemModel::setData(index, value, role);
}

bool TreeModel::setNodeCheckState(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole) {
        Qt::CheckState currentNodeCheckState = static_cast<Qt::CheckState>(value.toUInt());
        if(checklist.contains(index)) {
            checklist[index] = currentNodeCheckState;
        } else {
            checklist.insert(index, currentNodeCheckState);
        }

        setChildNodesCheck(index, value);

        emit dataChanged(index, index);
        return true;
    }
    return true;//QFileSystemModel::setData(index, value, role);
}

bool TreeModel::setChildNodesCheck(const QModelIndex& index, const QVariant& value)
{
    QTimer timer;
    QEventLoop eventLoop;

    while (canFetchMore(index)) {
        QObject::connect(this, SIGNAL(directoryLoaded(QString)), &eventLoop, SLOT(quit()));

        fetchMore(index);
        eventLoop.exec();
    }
    int childrenCount = rowCount(index);
    //qDebug() << "Children count:" << childrenCount;

    totalCount += childrenCount;

    emit setProgressBarRange(0, totalCount);

    if (childrenCount > 0) {
        // Need to wait because signal directoryLoaded does not guarantee
        // that all childs have already been loaded
        timer.singleShot(10, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();
    }

    for(int i = 0; i < childrenCount; i++) {
        setNodeCheckState(QFileSystemModel::index(i, 0, index), value, Qt::CheckStateRole);
        currentStep++;
        emit setProgressBarValue(currentStep);
    }
    return true;
}

bool TreeModel::setParentNodeCheckState(const QModelIndex& currentNodeIndex)
{
    QModelIndex parentNodeIndex = parent(currentNodeIndex);
    if(parentNodeIndex.isValid()) {
        Qt::CheckState parentNodeCheckState = getCheckStateAccordingToChildren(parentNodeIndex);
        if(checklist.contains(parentNodeIndex)) {
            checklist[parentNodeIndex] = parentNodeCheckState;
        } else {
            checklist.insert(parentNodeIndex, parentNodeCheckState);
        }
        emit dataChanged(parentNodeIndex, parentNodeIndex);
        return setParentNodeCheckState(parentNodeIndex);
    }
    return true;
}

Qt::CheckState TreeModel::getCheckStateAccordingToChildren(const QModelIndex & currentNodeIndex)
{
    Qt::CheckState checkState = Qt::Unchecked;
    int childrenCheckCount = 0;
    bool isAnyChildChecked = false;
    bool isAnyChildPartiallyChecked = false;
    if(hasChildren(currentNodeIndex)) {
        int childrenCount = rowCount(currentNodeIndex);
        for(int i = 0; i < childrenCount; i++) {
            QModelIndex childIndex = QFileSystemModel::index(i, 0, currentNodeIndex);
            if(checklist.contains(childIndex)) {
                Qt::CheckState currentCheckState = checklist[childIndex];
                if(currentCheckState == Qt::Checked) {
                    childrenCheckCount++;
                    isAnyChildChecked = true;
                } else if(currentCheckState == Qt::PartiallyChecked) {
                    isAnyChildPartiallyChecked = true;
                }
            }
        }
        if(childrenCount == childrenCheckCount) {
            checkState = Qt::Checked;
        } else if(isAnyChildChecked || isAnyChildPartiallyChecked) {
            checkState = Qt::PartiallyChecked;
        }
    }
    return checkState;
}


