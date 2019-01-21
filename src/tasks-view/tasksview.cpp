#include "tasksview.h"

TasksView::TasksView(QWidget *parent) :
    QTreeView(parent)
{
    setModel(new TasksModel(this));

    // Hide all categories by default
    for (int i = 0; i < model()->rowCount(QModelIndex()); ++i) {
        setRowHidden(i, QModelIndex(), true);
    }

    // Show category if item added
    connect(model(), &TasksModel::taskAdded, [&](Task::Category category) {
       setRowHidden(category, QModelIndex(), false);
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
