#include "packagesmodel.h"
#include "package.h"
#include "../pacmansettings.h"

#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonArray>

constexpr char AUR_API_URL[] = "https://aur.archlinux.org/rpc/";

PackagesModel::PackagesModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_manager = new QNetworkAccessManager(this);
    qRegisterMetaType<DatabaseStatus>("DatabaseStatus"); // To allow use databaseStatusChanged signal
    reloadRepoPackages();
}

PackagesModel::~PackagesModel()
{
    m_loadingDatabases.cancel();
    m_loadingDatabases.waitForFinished();

    qDeleteAll(m_repoPackages);
    qDeleteAll(m_aurPackages);
}

QVariant PackagesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const Package *package = static_cast<Package *>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
        switch (m_mode) {
        case Repo:
            switch (index.column()) {
            case 0:
                if (!package->isInstalled())
                    return QString();
                return QStringLiteral("Installed");
            case 1:
                return package->name();
            case 2:
                return package->version();
            case 3:
                return package->formattedInstalledSize();
            case 4:
                return package->repo();
            }
            break;
        case AUR:
            switch (index.column()) {
            case 0:
                if (!package->isInstalled())
                    return QString();
                return QStringLiteral("Installed");
            case 1:
                return package->name();
            case 2:
                return package->version();
            case 3:
                return package->votes();
            case 4:
                QLocale locale;
                return locale.toString(package->popularity(), 'f', 2);
            }
            break;
        }

        qFatal("Unknown column");
    case Qt::BackgroundRole:
        if (!package->availableUpdate().isEmpty())
            return QColor(255, 0, 0, 127);

        [[fallthrough]];
    default:
        return QVariant();
    }
}

QVariant PackagesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    // Headers
    switch (mode()) {
    case Repo:
        switch (section) {
        case 0:
            return QStringLiteral("Status");
        case 1:
            return QStringLiteral("Name");
        case 2:
            return QStringLiteral("Version");
        case 3:
            return QStringLiteral("Size");
        case 4:
            return QStringLiteral("Repo");
        }
        break;
    case AUR:
        switch (section) {
        case 0:
            return QStringLiteral("Status");
        case 1:
            return QStringLiteral("Name");
        case 2:
            return QStringLiteral("Version");
        case 3:
            return QStringLiteral("Votes");
        case 4:
            return QStringLiteral("Popularity");
        }
        break;
    }

    qFatal("Unknown column");
}

QModelIndex PackagesModel::index(int row, int column, const QModelIndex &) const
{
    switch (m_mode) {
    case Repo:
        if (row < m_repoPackages.size())
            return createIndex(row, column, m_repoPackages.at(row));
        break;
    case AUR:
        if (row < m_aurPackages.size())
            return createIndex(row, column, m_aurPackages.at(row));
        break;
    }

    return QModelIndex();
}

QModelIndex PackagesModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int PackagesModel::rowCount(const QModelIndex &) const
{
    switch (m_mode) {
    case Repo:
        return m_repoPackages.size();
    case AUR:
        return m_aurPackages.size();
    }

    qFatal("Unknown mode");
}

int PackagesModel::columnCount(const QModelIndex &) const
{
    return 5;
}

void PackagesModel::sort(int column, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();

    // Save persistent indexes according to docs: https://doc.qt.io/qt-5/qabstractitemmodel.html#layoutChanged
    const QModelIndexList oldIndexes = persistentIndexList();

    switch (m_mode) {
    case Repo:
        switch (column) {
        case 0:
            sortPackages(m_repoPackages, order, &Package::isInstalled, &Package::name);
            break;
        case 1:
            sortPackages(m_repoPackages, order, &Package::name);
            break;
        case 2:
            sortPackages(m_repoPackages, order, &Package::version, &Package::name);
            break;
        case 3:
            sortPackages(m_repoPackages, order, &Package::installedSize);
            break;
        case 4:
            sortPackages(m_repoPackages, order, &Package::repo, &Package::name);
            break;
        }
        break;
    case AUR:
        switch (column) {
        case 0:
            sortPackages(m_aurPackages, order, &Package::isInstalled, &Package::name);
            break;
        case 1:
            sortPackages(m_aurPackages, order, &Package::name);
            break;
        case 2:
            sortPackages(m_aurPackages, order, &Package::version, &Package::name);
            break;
        case 3:
            sortPackages(m_aurPackages, order, &Package::popularity);
            break;
        }
        break;
    }

    // Update indexes
    QModelIndexList newIndexes;
    foreach (const QModelIndex &oldIndex, oldIndexes) {
        auto *package = static_cast<Package *>(oldIndex.internalPointer()); // Get package from old index

        // Find new package position
        int row = 0;
        switch (m_mode) {
        case Repo:
            row = m_repoPackages.indexOf(package);
            break;
        case AUR:
            row = m_aurPackages.indexOf(package);
            break;
        }

        // Save index
        const QModelIndex newIndex = index(row, oldIndex.column());
        newIndexes.append(newIndex);
    }
    changePersistentIndexList(oldIndexes, newIndexes);

    emit layoutChanged();
}

PackagesModel::Mode PackagesModel::mode() const
{
    return m_mode;
}

void PackagesModel::setMode(Mode mode)
{
    beginResetModel();
    m_mode = mode;
    endResetModel();
}

PackagesModel::DatabaseStatus PackagesModel::databaseStatus() const
{
    return m_databaseStatus;
}

QVector<Package *> PackagesModel::packages() const
{
    switch (m_mode) {
    case Repo:
        return m_repoPackages;
    case AUR:
        return m_aurPackages;
    }

    qFatal("Unknown mode");
}

QVector<Package *> PackagesModel::outdatedPackages() const
{
    return m_outdatedPackages;
}

void PackagesModel::reloadRepoPackages()
{
    m_loadingDatabases = QtConcurrent::run(this, &PackagesModel::loadDatabases);
}

void PackagesModel::aurQuery(const QString &text, const QString &searchType)
{
    // Generate API URL
    QUrl url(AUR_API_URL);
    url.setQuery("v=5&type=search&by=" + searchType + "&arg=" + text);

    // Get request
    QScopedPointer reply(m_manager->get(QNetworkRequest(url)));
    QEventLoop waitForReply;
    connect(reply.get(), &QNetworkReply::finished, &waitForReply, &QEventLoop::quit);
    waitForReply.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << reply->errorString();
        return;
    }

    // Parse data
    const QJsonObject jsonData = QJsonDocument::fromJson(reply->readAll()).object();
    const QJsonObject packageData = jsonData.value("results").toArray().at(0).toObject();

    beginResetModel();

    // Clear old data
    qDeleteAll(m_aurPackages);
    m_aurPackages.clear();

    foreach (const QJsonValue &aurPackageData, jsonData.value("results").toArray()) {
        // Check if package already installed
        bool found = false;
        const QString packageName = aurPackageData.toObject().value("Name").toString();
        foreach (Package *package, m_repoPackages) {
            if (!package->isInstalled() || package->name() != packageName)
                continue;

            auto *aurPackage = new Package(*package);
            m_aurPackages.append(aurPackage);
            found = true;
            break;
        }

        if (!found) {
            // Create new package
            auto *package = new Package;
            package->setAurData(aurPackageData.toObject());
            m_aurPackages.append(package);
        }
    }

    endResetModel();
}

void PackagesModel::loadMoreAurInfo(Package *package)
{
    // Check if full AUR info already loaded
    if (package->fullAurInfo())
        return;

    QUrl url(AUR_API_URL);
    url.setQuery("v=5&type=info&arg[]=" + package->name());

    QScopedPointer reply(m_manager->get(QNetworkRequest(url)));
    QEventLoop waitForReply;
    connect(reply.get(), &QNetworkReply::finished, &waitForReply, &QEventLoop::quit);
    waitForReply.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << reply->errorString();
        return;
    }

    // Parse data
    const QJsonObject jsonData = QJsonDocument::fromJson(reply->readAll()).object();
    const QJsonObject packageData = jsonData.value("results").toArray().at(0).toObject();
    package->setAurData(packageData, true);
}

void PackagesModel::loadDatabases()
{
    setDatabaseStatus(Loading);

    if (m_handle != nullptr)
        resetDatabase();

    // Initialize ALPM
    const PacmanSettings settings;
    m_handle = alpm_initialize(qPrintable(settings.rootDir()), qPrintable(settings.databasesPath()), &m_error);
    if (m_error != ALPM_ERR_OK) {
        qDebug() << alpm_strerror(m_error);
        return;
    }

    // Load packages
    loadLocalDatabase();
    foreach (const QString &repo, settings.repositories())
        loadSyncDatabase(repo);
    loadAurDatabase();
    checkForUpdates(settings);

    emit databaseLoadingMessageChanged(QString::number(m_repoPackages.size())
                               + " packages avaible in official repositories, "
                               + QString::number(m_installedPackages.size())
                               + " packages installed, "
                               + (m_outdatedPackages.empty() ? "no" : QString::number(m_outdatedPackages.size()))
                               + " updates available");
}

// Load installed (local) packages
void PackagesModel::loadLocalDatabase()
{
    emit databaseLoadingMessageChanged("Loading installed packages");

    alpm_db_t *database = alpm_get_localdb(m_handle);
    alpm_list_t *cache = alpm_db_get_pkgcache(database);
    while (cache != nullptr) {
        if (m_loadingDatabases.isCanceled())
            return;

        auto *packageData = static_cast<alpm_pkg_t *>(cache->data);
        auto *package = new Package;
        package->setLocalData(packageData);

        beginInsertRows(QModelIndex(), m_repoPackages.size(), m_repoPackages.size());
        m_repoPackages.append(package);
        m_installedPackages.append(package);

        // Emit signal about first package
        if (m_repoPackages.size() == 1)
            emit firstPackageAvailable();

        cache = cache->next;
    }
    endInsertRows();
}

void PackagesModel::loadSyncDatabase(const QString &databaseName)
{
    emit databaseLoadingMessageChanged("Loading " + databaseName + " database");

    alpm_db_t *database = alpm_register_syncdb(m_handle, qPrintable(databaseName), 0);
    if (database == nullptr)
        return;

    alpm_list_t *cache = alpm_db_get_pkgcache(database);
    while (cache != nullptr) {
        if (m_loadingDatabases.isCanceled())
            return;

        auto *packageData = static_cast<alpm_pkg_t *>(cache->data);

        // Check if package installed
        bool found = false;
        for (Package *package : m_repoPackages) {
            if (!package->isInstalled() || !package->sameName(packageData))
                continue;

            package->setSyncData(packageData);
            emit packageChanged(package);
            found = true;
            break;
        }

        // Add new sync package to database
        if (!found) {
            auto *package = new Package;
            package->setSyncData(packageData);

            beginInsertRows(QModelIndex(), m_repoPackages.size(), m_repoPackages.size());
            m_repoPackages.append(package);
        }

        cache = cache->next;
    }
    endInsertRows();
}

void PackagesModel::loadAurDatabase()
{
    emit databaseLoadingMessageChanged("Loading information from AUR");
    QNetworkAccessManager manager;
    QUrl url(AUR_API_URL);

    // Get local packages names for query
    QString query = QStringLiteral("v=5&type=info");
    foreach (Package *package, m_repoPackages) {
        if (package->isInstalled() && package->repo() == "local")
            query.append("&arg[]=" + package->name());
    }
    url.setQuery(query);

    // Make API request
    QScopedPointer reply(manager.get(QNetworkRequest(url)));
    QEventLoop waitForReply;
    connect(reply.get(), &QNetworkReply::finished, &waitForReply, &QEventLoop::quit);
    waitForReply.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << reply->errorString();
        return;
    }

    // Parse data and info for packages
    const QJsonObject jsonReply = QJsonDocument::fromJson(reply->readAll()).object();
    foreach (QJsonValue packageData, jsonReply.value("results").toArray()) {
        for (Package *package : m_repoPackages) {
            if (package->isInstalled() && package->name() == packageData["Name"].toString())
                package->setAurData(packageData.toObject(), true);
        }
    }
}

// Check if updates for local packages is available from sync and aur databases
void PackagesModel::checkForUpdates(const PacmanSettings &settings)
{
    emit databaseLoadingMessageChanged("Checking for updates");

    const QStringList ignoredPackages = settings.ignoredPackages();
    foreach (Package *package, m_repoPackages) {
        if (!package->availableUpdate().isEmpty() && !ignoredPackages.contains(package->name()))
            m_outdatedPackages.append(package);
    }

    if (m_outdatedPackages.empty())
        setDatabaseStatus(NoUpdates);
    else
        setDatabaseStatus(UpdatesAvailable);
}

void PackagesModel::resetDatabase()
{
    beginResetModel();

    qDeleteAll(m_repoPackages);
    m_repoPackages.clear();
    m_outdatedPackages.clear();
    m_installedPackages.clear();
    alpm_release(m_handle);

    endResetModel();
}

void PackagesModel::setDatabaseStatus(DatabaseStatus databaseStatus)
{
    if (m_databaseStatus == databaseStatus)
        return;

    m_databaseStatus = databaseStatus;
    emit databaseStatusChanged(m_databaseStatus);
}

template<typename T1, typename T2>
void PackagesModel::sortPackages(QVector<Package *> &container, Qt::SortOrder order, T1 firstMember, T2 secondMember)
{
    switch (order) {
    case Qt::AscendingOrder:
        std::sort(container.begin(), container.end(), [&](Package *first, Package *second) {
            if ((first->*firstMember)() == (second->*firstMember)())
                return (first->*secondMember)() < (second->*secondMember)();
            return (first->*firstMember)() > (second->*firstMember)();
        });
        break;
    case Qt::DescendingOrder:
        std::sort(container.begin(), container.end(), [&](Package *first, Package *second) {
            if ((first->*firstMember)() == (second->*firstMember)())
                return (first->*secondMember)() < (second->*secondMember)();
            return (first->*firstMember)() < (second->*firstMember)();
        });
        break;
    }
}

template<typename T>
void PackagesModel::sortPackages(QVector<Package *> &container, Qt::SortOrder order, T member)
{
    switch (order) {
    case Qt::AscendingOrder:
        std::sort(container.begin(), container.end(), [&](Package *first, Package *second) {
            return (first->*member)() > (second->*member)();
        });
        break;
    case Qt::DescendingOrder:
        std::sort(container.begin(), container.end(), [&](Package *first, Package *second) {
            return (first->*member)() < (second->*member)();
        });
        break;
    }
}
