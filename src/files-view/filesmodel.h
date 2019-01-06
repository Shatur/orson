#ifndef FILESMODEL_H
#define FILESMODEL_H

#include "file.h"

#include <QAbstractItemModel>

class FilesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit FilesModel(QObject *parent = nullptr);
    ~FilesModel() override;

    // Model-specific functions
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Set paths to display in model
    void setPaths(const QStringList &paths);

private:
    void addPath(const QStringList &path);

    File *rootItem;
};

#endif // FILESMODEL_H
