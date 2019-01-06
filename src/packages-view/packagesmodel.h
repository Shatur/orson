#ifndef PACKAGESMODEL_H
#define PACKAGESMODEL_H

#include "package.h"

#include <QAbstractItemModel>
#include <QNetworkAccessManager>

class PackagesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Mode {
        Repo,
        AUR
    };

    explicit PackagesModel(QObject *parent = nullptr);
    ~PackagesModel() override;

    // Model-specific functions
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    // Show AUR or local database
    Mode mode() const;
    void setMode(Mode mode);

    // Other
    QVector<Package *> packages() const;
    alpm_errno_t error() const;
    void aurSearch(const QString &text, const QString &queryType);

private:
    void loadDatabase(const char *databaseName);

    // Repo ALPM specific members
    alpm_handle_t *m_handle;
    alpm_errno_t m_error = ALPM_ERR_OK;

    Mode m_mode = Repo;
    QVector<Package *> m_repoPackages;
    QVector<Package *> m_aurPackages;
    QNetworkAccessManager *m_manager;
};

#endif // PACKAGESMODEL_H
