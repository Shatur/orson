#include "tasksview.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QDebug>

TasksView::TasksView(QWidget *parent) :
    QTreeView(parent)
{
    setModel(new TasksModel(this));
    connect(model(), &TasksModel::taskAdded, this, &TasksView::showCategory);
    connect(model(), &TasksModel::taskRemoved, this, &TasksView::hideCategory);

    // Hide all categories by default
    for (int i = 0; i < model()->rowCount(QModelIndex()); ++i)
        setRowHidden(i, QModelIndex(), true);

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

void TasksView::showCategory(Task::Category category)
{
    const QModelIndex index = model()->index(category, 0, QModelIndex());

    setRowHidden(category, QModelIndex(), false);
    setExpanded(index, true);
}

// Hide category only if it is empty
void TasksView::hideCategory(Task::Category category)
{
    if (model()->tasks(category).isEmpty())
        setRowHidden(category, QModelIndex(), true);
}

void TasksView::removeCurrentTask()
{
    Task *task = currentTask();
    const Task::Category category = task->categoryType();
    if (category == Task::Null) {
        // Do not allow to remove tasks from "Upgrade all" category
        if (task->parent()->categoryType() != Task::UpgradeAll)
            model()->removeTask(task);
    } else {
        model()->removeTasks(category);
    }
}

void TasksView::contextMenuEvent(QContextMenuEvent *event)
{
    const auto *task = static_cast<Task *>(indexAt(event->pos()).internalPointer());
    if (task == nullptr)
        return;

    // Do not show context menu for categories
    if (task->parent()->categoryType() == Task::UpgradeAll)
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
