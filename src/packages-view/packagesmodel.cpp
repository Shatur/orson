#include "packagesmodel.h"

#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonArray>

constexpr char AUR_API_URL[] = "https://aur.archlinux.org/rpc/";

PackagesModel::PackagesModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_manager = new QNetworkAccessManager(this);
    m_handle = alpm_initialize("/", "/var/lib/pacman", &m_error);
    if (m_error != ALPM_ERR_OK)
        return;

    // Load sync packages
    loadDatabase("core");
    loadDatabase("extra");
    loadDatabase("community");
    loadDatabase("multilib");

    // Load local packages
    alpm_db_t *database = alpm_get_localdb(m_handle);
    alpm_list_t *cache = alpm_db_get_pkgcache(database);
    while (cache != nullptr) {
        auto *packageData = static_cast<alpm_pkg_t *>(cache->data);
        bool found = false;

        // Search package with the same name first (to add installation information for an existing package)
        const char *name = alpm_pkg_get_name(packageData);
        for (Package *package : m_repoPackages) {
            if (package->name() == name) {
                package->setLocalData(packageData);
                found = true;
                break;
            }
        }

        if (!found) {
            // Add local package
            auto *package = new Package;
            package->setLocalData(packageData);

            // Try to load info from AUR (temporary disabled)
//            QUrl url(AUR_URL);
//            url.setQuery("v=5&type=info&arg[]=" + QLatin1String(name));

//            QNetworkReply *reply = m_manager.get(QNetworkRequest(url));
//            QEventLoop waitForReply;
//            connect(reply, &QNetworkReply::finished, &waitForReply, &QEventLoop::quit);
//            waitForReply.exec();

//            if (reply->error() != QNetworkReply::NoError) {
//                qDebug() << reply->errorString();
//                return;
//            }

//            const QJsonObject jsonReply = QJsonDocument::fromJson(reply->readAll()).object();
//            if (jsonReply.value("resultcount").toInt() != 0) {
//                package->setAurData(jsonReply.value("results").toArray().at(0));
//            }

            m_repoPackages.append(package);
        }

        cache = cache->next;
    }
}

PackagesModel::~PackagesModel()
{
    qDeleteAll(m_repoPackages);
    alpm_release(m_handle);
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
                return "";
            return "Installed";
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
                return "";
            return "Installed";
        case 1:
            return package->name();
        case 2:
            return package->version();
        case 3:
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
            return "Status";
        case 1:
            return "Name";
        case 2:
            return "Version";
        case 3:
            return "Size";
        case 4:
            return "Repo";
        }
        break;
    case AUR:
        switch (section) {
        case 0:
            return "Status";
        case 1:
            return "Name";
        case 2:
            return "Version";
        case 3:
            return "Popularity";
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
    switch (m_mode) {
    case Repo:
        return 5;
    case AUR:
        return 4;
    }

    qFatal("Unknown mode");
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
    foreach (auto oldIndex, oldIndexes) {
        const auto package = static_cast<Package *>(oldIndex.internalPointer()); // Get package from old index

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

alpm_errno_t PackagesModel::error() const
{
    return m_error;
}

void PackagesModel::aurSearch(const QString &text, const QString &queryType)
{
    alpm_db_t *localDatabase = alpm_get_localdb(m_handle);

    // Generate API URL
    QUrl url(AUR_API_URL);
    url.setQuery("v=5&type=search&by=" + queryType + "&arg=" + text);

    // Get request
    QNetworkReply *reply = m_manager->get(QNetworkRequest(url));
    QEventLoop waitForReply;
    connect(reply, &QNetworkReply::finished, &waitForReply, &QEventLoop::quit);
    waitForReply.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << reply->errorString();
        return;
    }

    // Parse data
    const QJsonObject jsonData = QJsonDocument::fromJson(reply->readAll()).object();
    const QJsonObject packageData = jsonData.value("results").toArray().at(0).toObject();

    beginResetModel();
    m_aurPackages.clear();
    foreach (const QJsonValue &aurPackageData, jsonData.value("results").toArray()) {
        // Get AUR data
        const auto package = new Package;
        package->setAurData(aurPackageData);

        // Get local data
        alpm_pkg_t *localPackageData = alpm_db_get_pkg(localDatabase, qPrintable(package->name()));
        package->setLocalData(localPackageData);

        m_aurPackages.append(package);
    }
    endResetModel();
}

void PackagesModel::loadMoreAurInfo(Package *package)
{
    QUrl url(AUR_API_URL);
    url.setQuery("v=5&type=info&arg[]=" + package->name());

    QNetworkReply *reply = m_manager->get(QNetworkRequest(url));
    QEventLoop waitForReply;
    connect(reply, &QNetworkReply::finished, &waitForReply, &QEventLoop::quit);
    waitForReply.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << reply->errorString();
        return;
    }

    // Parse data
    const QJsonObject jsonData = QJsonDocument::fromJson(reply->readAll()).object();
    const QJsonObject packageData = jsonData.value("results").toArray().at(0).toObject();
    package->setAurData(packageData);
}

void PackagesModel::loadDatabase(const char *databaseName)
{
    alpm_db_t *database = alpm_register_syncdb(m_handle, databaseName, 0);
    if (database == nullptr)
        return;

    alpm_list_t *cache = alpm_db_get_pkgcache(database);
    while (cache != nullptr) {
        auto *packageData = static_cast<alpm_pkg_t *>(cache->data);
        auto package = new Package;
        package->setSyncData(packageData);
        m_repoPackages.append(package);
        cache = cache->next;
    }
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
