#include "tasksmodel.h"

TasksModel::TasksModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_rootItem = new Task;
}

TasksModel::~TasksModel()
{
    delete m_rootItem;
}

QVariant TasksModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Task *item = static_cast<Task *>(index.internalPointer());
    switch (role) {
    case Qt::DisplayRole:
        return item->name();
    case Qt::DecorationRole:
        return item->icon();
    default:
        return QVariant();
    }
}

QVariant TasksModel::headerData(int, Qt::Orientation, int) const
{
    return QVariant();
}

QModelIndex TasksModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    Task *parentItem;
    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<Task *>(parent.internalPointer());

    Task *childItem = parentItem->child(row);
    if (!childItem)
        return QModelIndex();

    return createIndex(row, column, childItem);
}

QModelIndex TasksModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Task *childItem = static_cast<Task *>(index.internalPointer());
    Task *parentItem = childItem->parent();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TasksModel::rowCount(const QModelIndex &parent) const
{
    Task *parentItem;
    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<Task *>(parent.internalPointer());

    return parentItem->childCount();
}

int TasksModel::columnCount(const QModelIndex &) const
{
    return 1;
}

void TasksModel::addTask(const Package *package, Task::Category category)
{
    Task *parent = m_rootItem->child(category);
    QModelIndex parentIndex = index(category, 0, QModelIndex());

    beginInsertRows(parentIndex, parent->childCount(), parent->childCount());
    parent->addChild(new Task(package));
    endInsertRows();
}
