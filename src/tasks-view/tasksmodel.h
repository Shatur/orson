#ifndef TASKSMODEL_H
#define TASKSMODEL_H

#include "task.h"

#include <QAbstractItemModel>

class PackagesView;

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

    void setTasks(PackagesView *packagesView);
    void removeTask(Task *task);

signals:
    void taskRemoved(Task *task);

private:
    void addCategory(Task::Type category, const QVector<Package *> &packages = {});

    Task *m_rootItem;
    PackagesView *m_PackagesView;
};

#endif // TASKSMODEL_H
