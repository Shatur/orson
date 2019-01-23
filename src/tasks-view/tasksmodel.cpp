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

    Task *childItem = parentItem->children().at(row);
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

    return parentItem->children().size();
}

int TasksModel::columnCount(const QModelIndex &) const
{
    return 1;
}

void TasksModel::addTask(const Package *package, Task::Category destinationCategory)
{
    const QModelIndex destinationParentIndex = index(destinationCategory, 0, QModelIndex());
    Task *destinationParentTask = m_rootItem->children().at(destinationCategory);

    Task *task = find(package->name());
    if (task != nullptr) {
        // Just move task if the package has been added to another category
        const Task::Category sourceCategory = task->parent()->category();
        const QModelIndex sourceParentIndex = index(sourceCategory, 0, QModelIndex());
        Task *sourceParentTask = m_rootItem->children().at(sourceCategory);

        beginMoveRows(sourceParentIndex, task->row(), task->row(), destinationParentIndex, tasks(destinationCategory).size());
        sourceParentTask->removeChild(task);
        emit taskRemoved(sourceCategory);
        destinationParentTask->addChild(task);
        endMoveRows();
    } else {
        // Add new task
        beginInsertRows(destinationParentIndex, destinationParentTask->children().size(), destinationParentTask->children().size());
        destinationParentTask->addChild(new Task(package));
        endInsertRows();
    }

    emit taskAdded(destinationCategory);
}

void TasksModel::removeTask(Task *task)
{
    Task *parentTask = task->parent();
    const QModelIndex parentIndex = index(parentTask->row(), 0, QModelIndex());

    beginRemoveRows(parentIndex, task->row(), task->row());
    parentTask->removeChild(task);
    delete task;
    endRemoveRows();

    emit taskRemoved(parentTask->category());
}

QVector<Task *> TasksModel::categories()
{
    return m_rootItem->children();
}

QVector<Task *> TasksModel::tasks(Task::Category category)
{
    return categories().at(category)->children();
}

Task *TasksModel::find(QString packageName)
{
    foreach (Task *category, m_rootItem->children()) {
        foreach (Task *task, category->children()) {
            if (task->name() == packageName)
                return task;
        }
    }

    return nullptr;
}

int TasksModel::categoriesCount()
{
    return m_rootItem->children().size();
}
