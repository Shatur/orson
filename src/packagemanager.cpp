#include "packagemanager.h"

#include "alpm_list.h"

#include <QDebug>

PackageManager::PackageManager()
{
    m_handle = alpm_initialize("/", "/var/lib/pacman", &m_error);

    if (m_error == ALPM_ERR_OK) {
        loadPackages("core");
        loadPackages("extra");
        loadPackages("community");
        loadPackages("multilib");
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
    alpm_db_t *remoteDatabase = alpm_register_syncdb(m_handle, databaseName, 0);
    if (remoteDatabase == nullptr)
        return;

    alpm_db_t *localDatabase = alpm_get_localdb(m_handle);

    alpm_list_t *packageCache = alpm_db_get_pkgcache(remoteDatabase);
    for (int i = 0; packageCache != nullptr; ++i) {
        auto *packageData = static_cast<alpm_pkg_t *>(packageCache->data);
        alpm_pkg_t *localPackageData = alpm_db_get_pkg(localDatabase, alpm_pkg_get_name(packageData));
        Package package(packageData, localPackageData);

        m_packages.append(package);
        packageCache = packageCache->next;
    }
}

QList<Package> PackageManager::packages() const
{
    return m_packages;
}
