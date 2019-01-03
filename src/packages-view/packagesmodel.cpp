#include "packagesmodel.h"

PackagesModel::PackagesModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_handle = alpm_initialize("/", "/var/lib/pacman", &m_error);
    if (m_error != ALPM_ERR_OK)
        return;

    // Load sync packages
    loadPackages("core");
    loadPackages("extra");
    loadPackages("community");
    loadPackages("multilib");

    // Load local packages
    alpm_db_t *database = alpm_get_localdb(m_handle);
    alpm_list_t *cache = alpm_db_get_pkgcache(database);
    while (cache != nullptr) {
        auto *packageData = static_cast<alpm_pkg_t *>(cache->data);
        bool found = false;

        // Search package with the same name first (to add installation information for an existing package)
        const char *name = alpm_pkg_get_name(packageData);
        for (Package *package : m_packages) {
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
            m_packages.append(package);
        }

        cache = cache->next;
    }
}

PackagesModel::~PackagesModel()
{
    qDeleteAll(m_packages);
    alpm_release(m_handle);
}

QVariant PackagesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    const Package *package = static_cast<Package *>(index.internalPointer());

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
    default:
        qFatal("Unknown column");
    }
}

QVariant PackagesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    // Headers
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
    default:
        qFatal("Unknown column:");
    }
}

QModelIndex PackagesModel::index(int row, int column, const QModelIndex &) const
{
    if (row > m_packages.size())
        return QModelIndex();

    return createIndex(row, column, m_packages.at(row));
}

QModelIndex PackagesModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int PackagesModel::rowCount(const QModelIndex &) const
{
    return m_packages.size();
}

int PackagesModel::columnCount(const QModelIndex &) const
{
    return 5;
}

void PackagesModel::sort(int column, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();
    switch (column) {
    case 0:
        switch (order) {
        case Qt::AscendingOrder:
            std::sort(m_packages.begin(), m_packages.end(), [&](Package *first, Package *second) {
                if (first->isInstalled() == second->isInstalled())
                    return first->name() < second->name();
               return first->isInstalled() > second->isInstalled();
            });
            break;
        case Qt::DescendingOrder:
            std::sort(m_packages.begin(), m_packages.end(), [&](Package *first, Package *second) {
                if (first->isInstalled() == second->isInstalled())
                    return first->name() < second->name();
               return first->isInstalled() < second->isInstalled();
            });
        }
        break;
    case 1:
        switch (order) {
        case Qt::AscendingOrder:
            std::sort(m_packages.begin(), m_packages.end(), [&](Package *first, Package *second) {
               return first->name() < second->name();
            });
            break;
        case Qt::DescendingOrder:
            std::sort(m_packages.begin(), m_packages.end(), [&](Package *first, Package *second) {
               return first->name() > second->name();
            });
        }
        break;
    case 2:
        switch (order) {
        case Qt::AscendingOrder:
            std::sort(m_packages.begin(), m_packages.end(), [&](Package *first, Package *second) {
                if (first->version() == second->version())
                    return first->name() < second->name();
               return first->version() < second->version();
            });
            break;
        case Qt::DescendingOrder:
            std::sort(m_packages.begin(), m_packages.end(), [&](Package *first, Package *second) {
                if (first->version() == second->version())
                    return first->name() < second->name();
               return first->version() > second->version();
            });
        }
        break;
    case 3:
        switch (order) {
        case Qt::AscendingOrder:
            std::sort(m_packages.begin(), m_packages.end(), [&](Package *first, Package *second) {
                if (first->installedSize() == second->installedSize())
                    return first->name() < second->name();
               return first->installedSize() < second->installedSize();
            });
            break;
        case Qt::DescendingOrder:
            std::sort(m_packages.begin(), m_packages.end(), [&](Package *first, Package *second) {
                if (first->installedSize() == second->installedSize())
                    return first->name() < second->name();
               return first->installedSize() > second->installedSize();
            });
        }
        break;
    case 4:
        switch (order) {
        case Qt::AscendingOrder:
            std::sort(m_packages.begin(), m_packages.end(), [&](Package *first, Package *second) {
                if (first->name() == second->name())
                    return first->name() < second->name();
               return first->repo() < second->repo();
            });
            break;
        case Qt::DescendingOrder:
            std::sort(m_packages.begin(), m_packages.end(), [&](Package *first, Package *second) {
                if (first->name() == second->name())
                    return first->name() < second->name();
               return first->repo() > second->repo();
            });
        }
    }
    emit layoutChanged();
}

QVector<Package *> PackagesModel::packages() const
{
    return m_packages;
}

alpm_errno_t PackagesModel::error() const
{
    return m_error;
}

void PackagesModel::loadPackages(const char *databaseName)
{
    alpm_db_t *database = alpm_register_syncdb(m_handle, databaseName, 0);
    if (database == nullptr)
        return;

    alpm_list_t *cache = alpm_db_get_pkgcache(database);
    while (cache != nullptr) {
        auto *packageData = static_cast<alpm_pkg_t *>(cache->data);
        auto package = new Package;
        package->setSyncData(packageData);
        m_packages.append(package);
        cache = cache->next;
    }
}
