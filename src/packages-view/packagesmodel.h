#ifndef PACKAGESMODEL_H
#define PACKAGESMODEL_H

#include "package.h"

#include <QAbstractItemModel>
#include <QNetworkAccessManager>
#include <QtConcurrent>

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
    void loadMoreAurInfo(Package *package);

signals:
    void databaseStatusChanged(const QString &text);

private:
    template<typename T>
    using Comparator = T (Package::*)() const;

    void loadDatabases();
    void loadDatabase(const char *databaseName);

    template<typename T1, typename T2>
    void sortPackages(QVector<Package *> &container, Qt::SortOrder order, Comparator<T1> firstMember, Comparator<T2> secondMember);

    template<typename T>
    void sortPackages(QVector<Package *> &container, Qt::SortOrder order, Comparator<T> member);

    // Repo ALPM specific members
    alpm_handle_t *m_handle = nullptr;
    alpm_errno_t m_error = ALPM_ERR_OK;

    Mode m_mode = Repo;
    QVector<Package *> m_repoPackages;
    QVector<Package *> m_aurPackages;
    QNetworkAccessManager *m_manager;
    QFuture<void> m_loadingDatabases;
};

#endif // PACKAGESMODEL_H
