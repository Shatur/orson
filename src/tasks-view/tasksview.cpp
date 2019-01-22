#include "tasksview.h"

TasksView::TasksView(QWidget *parent) :
    QTreeView(parent)
{
    setModel(new TasksModel(this));

    // Hide all categories by default
    for (int i = 0; i < model()->rowCount(QModelIndex()); ++i)
        setRowHidden(i, QModelIndex(), true);

    // Show category if item added
    connect(model(), &TasksModel::taskAdded, [&](Task::Category category) {
        const QModelIndex index = model()->index(category, 0, QModelIndex());

        setRowHidden(category, QModelIndex(), false);
        setExpanded(index, true);
    });

    // Hide category if it is empty
    connect(model(), &TasksModel::taskRemoved, [&](Task::Category category) {
        const QModelIndex index = model()->index(category, 0, QModelIndex());
        const auto task = static_cast<Task *>(index.internalPointer());

        if (task->children().isEmpty())
            setRowHidden(category, QModelIndex(), true);
    });
}

TasksModel *TasksView::model() const
{
    return qobject_cast<TasksModel *>(QTreeView::model());
}

void TasksView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
}
