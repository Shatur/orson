#ifndef TASKSMODEL_H
#define TASKSMODEL_H

#include "task.h"

#include <QAbstractItemModel>

class TasksModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TasksModel(QObject *parent = nullptr);
    ~TasksModel() override;

    // Model-specific functions
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void addTask(const Package *package, Task::Category destinationCategory);
    void removeTask(Task *task);
    QVector<Task *> categories();
    QVector<Task *> tasks(Task::Category category);
    Task *find(QString packageName);
    int categoriesCount();

signals:
    void taskAdded(Task::Category category);
    void taskRemoved(Task::Category category);

private:
    Task *m_rootItem;
};

#endif // TASKSMODEL_H
