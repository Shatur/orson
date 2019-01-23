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

signals:
    void taskOpened(const QString &packageName);

private slots:
    void showCategory(Task::Category category);
    void hideCategory(Task::Category category);

    // Context menu actions
    void removeCurrentTask();
    void openCurrentTask();

private:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void setModel(QAbstractItemModel *model) override;

    QMenu *m_menu;
};

#endif // TASKSVIEW_H
