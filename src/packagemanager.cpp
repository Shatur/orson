#include "packagemanager.h"

PackageManager::PackageManager()
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
        for (Package &package : m_packages) {
            if (package.name() == name) {
                package.setLocalData(packageData);
                found = true;
                break;
            }
        }

        if (!found) {
            // Add local package
            Package package;
            package.setLocalData(packageData);
            m_packages.append(package);
        }

        cache = cache->next;
    }
}

PackageManager::~PackageManager()
{
    alpm_release(m_handle);
}

alpm_errno_t PackageManager::error() const
{
    return m_error;
}

void PackageManager::loadPackages(const char *databaseName)
{
    alpm_db_t *database = alpm_register_syncdb(m_handle, databaseName, 0);
    if (database == nullptr)
        return;

    alpm_list_t *cache = alpm_db_get_pkgcache(database);
    while (cache != nullptr) {
        auto *packageData = static_cast<alpm_pkg_t *>(cache->data);
        Package package;
        package.setSyncData(packageData);
        m_packages.append(package);
        cache = cache->next;
    }
}

QList<Package> PackageManager::packages() const
{
    return m_packages;
}

Package PackageManager::package(int index) const
{
    return m_packages.at(index);
}
