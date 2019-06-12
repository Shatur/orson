#ifndef PACKAGESMODEL_H
#define PACKAGESMODEL_H

#include <QAbstractItemModel>
#include <QtConcurrent>

#include <alpm.h>

class Package;
class QNetworkAccessManager;

class PackagesModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY(PackagesModel)

public:
    enum Mode {
        Repo,
        AUR
    };
    enum DatabaseStatus {
        Loading,
        UpdatesAvailable,
        NoUpdates
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

    // Switch between AUR and ALPM
    Mode mode() const;
    void setMode(Mode mode);

    // Other
    DatabaseStatus databaseStatus() const;
    QVector<Package *> packages() const;
    QVector<Package *> outdatedPackages() const;
    void reloadRepoPackages();
    void aurQuery(const QString &text, const QString &searchType);
    void loadMoreAurInfo(Package *package);

signals:
    void databaseStatusChanged(PackagesModel::DatabaseStatus status);
    void databaseLoadingMessageChanged(const QString &text);
    void firstPackageAvailable();
    void packageChanged(Package *package);

private:
    void setDatabaseStatus(DatabaseStatus databaseStatus);
    void loadDatabases();

    // Helper functions for loading all databases
    int loadLocalDatabase();
    void loadSyncDatabase(const QString &databaseName);
    void loadAurDatabase();

    // Sorting
    template<typename T1, typename T2>
    void sortPackages(QVector<Package *> &container, Qt::SortOrder order, T1 firstMember, T2 secondMember);

    template<typename T>
    void sortPackages(QVector<Package *> &container, Qt::SortOrder order, T member);

    // ALPM stuff
    alpm_handle_t *m_handle = nullptr;
    alpm_errno_t m_error = ALPM_ERR_OK;

    Mode m_mode = Repo;
    DatabaseStatus m_databaseStatus = Loading;
    QFuture<void> m_loadingDatabases;

    QVector<Package *> m_repoPackages;
    QVector<Package *> m_aurPackages;
    QVector<Package *> m_outdatedPackages;

    QNetworkAccessManager *m_manager;
};

Q_DECLARE_METATYPE(PackagesModel::DatabaseStatus)

#endif // PACKAGESMODEL_H
