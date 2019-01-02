#ifndef PACKAGESMODEL_H
#define PACKAGESMODEL_H

#include "package.h"

#include <QAbstractItemModel>

class PackagesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit PackagesModel(QObject *parent = nullptr);
    ~PackagesModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &) const override;
    QModelIndex parent(const QModelIndex &) const override;
    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QVector<Package *> packages() const;
    alpm_errno_t error() const;

private:
    void loadPackages(const char *databaseName);
    void addPackage(Package *package);

    alpm_handle_t *m_handle;
    alpm_errno_t m_error = ALPM_ERR_OK;
    QVector<Package *> m_packages;
};

#endif // PACKAGESMODEL_H
