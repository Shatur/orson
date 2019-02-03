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
        if (item->type() == Task::Item)
            return item->package()->name();
        return item->categoryName(item->type());
    case Qt::DecorationRole:
        if (item->type() == Task::Item)
            return item->package()->icon();
        return item->categoryIcon(item->type());
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

void TasksModel::setTasks(PackagesView *packagesView)
{
    beginResetModel();
    m_rootItem->removeChildren();

    if (packagesView->isSyncRepositories())
        addCategory(Task::Sync);

    if (packagesView->isUpgradePackages())
        addCategory(Task::UpgradeAll, packagesView->model()->outdatedPackages());

    if (!packagesView->installExplicity().isEmpty())
        addCategory(Task::InstallExplicity, packagesView->installExplicity());

    if (!packagesView->installAsDepend().isEmpty())
        addCategory(Task::InstallAsDepend, packagesView->installAsDepend());

    if (!packagesView->reinstall().isEmpty())
        addCategory(Task::Reinstall, packagesView->reinstall());

    if (!packagesView->markAsExplicity().isEmpty())
        addCategory(Task::MarkAsExplicity, packagesView->markAsExplicity());

    if (!packagesView->markAsDepend().isEmpty())
        addCategory(Task::MarkAsDepend, packagesView->markAsDepend());

    if (!packagesView->uninstall().isEmpty())
        addCategory(Task::Uninstall, packagesView->uninstall());

    endResetModel();

    connect(this, &TasksModel::taskRemoved, packagesView, &PackagesView::removeOperation);
}

void TasksModel::removeTask(Task *task)
{
    Task *parentTask = task->parent();
    const QModelIndex parentIndex = index(parentTask->row(), 0, QModelIndex());

    beginRemoveRows(parentIndex, task->row(), task->row());
    emit taskRemoved(task);
    parentTask->removeChild(task);
    endRemoveRows();

    // Remove category if it is empty
    if (parentTask != m_rootItem && parentTask->children().isEmpty())
        removeTask(parentTask);
}

void TasksModel::addCategory(Task::Type category, const QVector<Package *> &packages)
{
    auto *taskCategory = new Task(category);
    m_rootItem->addChild(taskCategory);
    foreach (Package *package, packages)
        taskCategory->addChild(new Task(package));
}
