#include "packagemanager.h"

#include "alpm_list.h"

#include <QDebug>

PackageManager::PackageManager()
{
    alpm_handle_t *handle = alpm_initialize("/", "/var/lib/pacman", &m_error);

    if (m_error == ALPM_ERR_OK) {
        loadPackages(handle, "core");
        loadPackages(handle, "extra");
        loadPackages(handle, "community");
        loadPackages(handle, "multilib");

        alpm_release(handle);
    }
}

PackageManager::~PackageManager()
{

}

alpm_errno_t PackageManager::error() const
{
    return m_error;
}

void PackageManager::loadPackages(alpm_handle_t *handle, const char *databaseName)
{
    alpm_db_t *remoteDatabase = alpm_register_syncdb(handle, databaseName, 0);
    if (remoteDatabase == nullptr)
        return;

    alpm_db_t *localDatabase = alpm_get_localdb(handle);

    alpm_list_t *packageCache = alpm_db_get_pkgcache(remoteDatabase);
    for (int i = 0; packageCache != nullptr; ++i) {
        alpm_pkg_t *packageData = static_cast<alpm_pkg_t *>(packageCache->data);

        Package package;
        package.setName(alpm_pkg_get_name(packageData));
        package.setVersion(alpm_pkg_get_version(packageData));
        package.setRepo(databaseName);
        package.setSize(alpm_pkg_get_size(packageData));

        // Check if package exists in local (installed) packages
        if (alpm_db_get_pkg(localDatabase, alpm_pkg_get_name(packageData)) != nullptr)
            package.setInstalled(true);

        m_packages.append(package);
        packageCache = packageCache->next;
    }
}

QList<Package> PackageManager::packages() const
{
    return m_packages;
}
