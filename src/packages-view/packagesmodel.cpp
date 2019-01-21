#include "packagesmodel.h"
#include "../pacmansettings.h"

#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonArray>

constexpr char AUR_API_URL[] = "https://aur.archlinux.org/rpc/";

PackagesModel::PackagesModel(QObject *parent) :
    QAbstractItemModel(parent)
{
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
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const Package *package = static_cast<Package *>(index.internalPointer());

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
            return package->popularity();
        }
        break;
    }

    qFatal("Unknown column");
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
        if (row < m_repoPackages.size() - 1)
            return createIndex(row, column, m_repoPackages.at(row));
        break;
    case AUR:
        if (row < m_aurPackages.size() - 1)
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
    QModelIndexList oldIndexes = persistentIndexList();

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

void PackagesModel::reloadRepoPackages()
{
    m_loadingDatabases = QtConcurrent::run(this, &PackagesModel::loadRepoPackages);
}

void PackagesModel::aurSearch(const QString &text, const QString &queryType)
{
    // Generate API URL
    QUrl url(AUR_API_URL);
    url.setQuery("v=5&type=search&by=" + queryType + "&arg=" + text);

    // Get request
    QScopedPointer reply(m_manager.get(QNetworkRequest(url)));
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

    QScopedPointer reply(m_manager.get(QNetworkRequest(url)));
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

void PackagesModel::loadRepoPackages()
{
    // Reset old data
    if (!m_repoPackages.isEmpty()) {
        beginResetModel();

        qDeleteAll(m_repoPackages);
        m_repoPackages.clear();
        alpm_release(m_handle);

        endResetModel();
    }

    // Initialize ALPM
    const PacmanSettings settings;
    m_handle = alpm_initialize(qPrintable(settings.rootDir()), qPrintable(settings.databasesPath()), &m_error);
    if (m_error != ALPM_ERR_OK) {
        qDebug() << alpm_strerror(m_error);
        return;
    }

    // Load local database
    const int installedPackages = loadLocalDatabase();

    // Load sync packages
    foreach (const QString &repo, settings.repositories())
        loadSyncDatabase(repo);

    // Load information from packages installed from AUR
//    QNetworkAccessManager manager;
//    for (Package *package : m_repoPackages) {
//        if (!package->isInstalled() || package->repo() != "local")
//            continue;

//        if (m_loadingDatabases.isCanceled())
//            return;

//        const QString packageName = package->name();
//        emit databaseStatusChanged("Loading information from AUR for " + packageName);

//        QUrl url(AUR_API_URL);
//        url.setQuery("v=5&type=info&arg[]=" + packageName);

//        QScopedPointer reply(manager.get(QNetworkRequest(url)));
//        QEventLoop waitForReply;
//        connect(reply.get(), &QNetworkReply::finished, &waitForReply, &QEventLoop::quit);
//        waitForReply.exec();

//        if (reply->error() != QNetworkReply::NoError) {
//            qDebug() << reply->errorString();
//            break;
//        }

//        const QJsonObject jsonReply = QJsonDocument::fromJson(reply->readAll()).object();
//        if (jsonReply.value("resultcount").toInt() != 0)
//            package->setAurData(jsonReply.value("results").toArray().at(0).toObject());
//    }

    emit databaseStatusChanged(QString::number(m_repoPackages.size())
                               + " packages avaible in official repositories, "
                               + QString::number(installedPackages)
                               + " packages installed");
    emit databaseLoaded();
}

// Load installed (local) packages
int PackagesModel::loadLocalDatabase()
{
    alpm_db_t *database = alpm_get_localdb(m_handle);
    alpm_list_t *cache = alpm_db_get_pkgcache(database);
    while (cache != nullptr) {
        if (m_loadingDatabases.isCanceled())
            return 0;

        auto *packageData = static_cast<alpm_pkg_t *>(cache->data);
        auto *package = new Package;
        package->setLocalData(packageData);

        // Emit signal about first package
        if (m_repoPackages.size() == 0)
            emit firstPackageAvailable();

        beginInsertRows(QModelIndex(), m_repoPackages.size(), m_repoPackages.size());
        m_repoPackages.append(package);

        cache = cache->next;
    }
    endInsertRows();

    return m_repoPackages.size();
}

void PackagesModel::loadSyncDatabase(const QString &databaseName)
{
    emit databaseStatusChanged("Loading " + databaseName + " database");

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

template<typename T1, typename T2>
void PackagesModel::sortPackages(QVector<Package *> &container, Qt::SortOrder order,
                                 Comparator<T1> firstMember, Comparator<T2> secondMember)
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
void PackagesModel::sortPackages(QVector<Package *> &container, Qt::SortOrder order, Comparator<T> member)
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
