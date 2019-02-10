#include "tasksview.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QDebug>

#include "tasksmodel.h"

TasksView::TasksView(QWidget *parent) :
    QTreeView(parent)
{
    setModel(new TasksModel(this));

    // Setup context menu
    m_menu = new QMenu(this);
    m_menu->addAction(QIcon::fromTheme("remove"), "Remove", this, &TasksView::removeCurrentTask);
}

TasksModel *TasksView::model() const
{
    return qobject_cast<TasksModel *>(QTreeView::model());
}

Task *TasksView::currentTask()
{
    return static_cast<Task *>(currentIndex().internalPointer());
}

void TasksView::removeCurrentTask()
{
    Task *task = currentTask();
    const Task::Type category = task->type();

    // Do not allow to remove packages from upgrade all category
    if (category == Task::Item && task->parent()->type() == Task::UpgradeAll)
        return;

    model()->removeTask(task);
}

void TasksView::contextMenuEvent(QContextMenuEvent *event)
{
    const auto *task = static_cast<Task *>(indexAt(event->pos()).internalPointer());
    if (task == nullptr)
        return;

    // Do not show context menu for categories
    if (task->parent()->type() == Task::UpgradeAll)
        m_menu->actions().at(0)->setEnabled(false);
    else
        m_menu->actions().at(0)->setEnabled(true);

    m_menu->exec(event->globalPos());
}

void TasksView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
        removeCurrentTask();
    else
        QTreeView::keyPressEvent(event);

}

void TasksView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
}
