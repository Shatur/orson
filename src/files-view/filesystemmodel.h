#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include "filesystemitem.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class FileSystemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit FileSystemModel(QObject *parent = nullptr);
    ~FileSystemModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void setPaths(const QStringList &paths);

private:
    void addPath(const QStringList &path);

    FileSystemItem *rootItem = new FileSystemItem;
};

#endif // FILESYSTEMMODEL_H
