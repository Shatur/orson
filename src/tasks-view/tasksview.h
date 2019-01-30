#ifndef TASKSVIEW_H
#define TASKSVIEW_H

#include "tasksmodel.h"

#include <QTreeView>

class TasksView : public QTreeView
{
    Q_OBJECT

public:
    explicit TasksView(QWidget *parent = nullptr);

    TasksModel *model() const;
    Task *currentTask();

private slots:
    void showCategory(Task::Category category);
    void hideCategory(Task::Category category);
    void removeCurrentTask();

private:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void setModel(QAbstractItemModel *model) override;

    QMenu *m_menu;
};

#endif // TASKSVIEW_H
