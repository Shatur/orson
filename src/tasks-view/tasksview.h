#ifndef TASKSVIEW_H
#define TASKSVIEW_H

#include "tasksmodel.h"

#include <QTreeView>

class TasksView : public QTreeView
{
public:
    explicit TasksView(QWidget *parent = nullptr);
    TasksModel *model() const;

private:
    void setModel(QAbstractItemModel *model) override;
};

#endif // TASKSVIEW_H
