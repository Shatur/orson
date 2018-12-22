#include "package.h"

Package::Package(alpm_pkg_t *syncPackageData, alpm_pkg_t *localPackageData) :
    m_syncPackageData(syncPackageData),
    m_localPackageData(localPackageData)
{
    if (m_localPackageData != nullptr)
        m_isInstalled = true;
    else
        m_isInstalled = false;
}

QString Package::name() const
{
    return alpm_pkg_get_name(m_syncPackageData);
}

QString Package::repo() const
{
    return alpm_db_get_name(alpm_pkg_get_db(m_syncPackageData));
}

QString Package::version() const
{
    return alpm_pkg_get_version(m_syncPackageData);
}

QString Package::description() const
{
    return alpm_pkg_get_desc(m_syncPackageData);
}

QString Package::arch() const
{
    return alpm_pkg_get_arch(m_syncPackageData);
}

QString Package::url() const
{
    return alpm_pkg_get_url(m_syncPackageData);
}

QString Package::packager() const
{
    return alpm_pkg_get_packager(m_syncPackageData);
}

QStringList Package::licenses() const
{
    QStringList licenses;
    alpm_list_t *licensesList = alpm_pkg_get_licenses(m_syncPackageData);
    while (licensesList != nullptr) {
        licenses.append(static_cast<const char*>(licensesList->data));
        licensesList = licensesList->next;
    }
    return licenses;
}

QStringList Package::groups() const
{
    QStringList groups;
    alpm_list_t *groupsList = alpm_pkg_get_groups(m_syncPackageData);
    while (groupsList != nullptr) {
        groups.append(static_cast<const char*>(groupsList->data));
        groupsList = groupsList->next;
    }
    return groups;
}

QStringList Package::files() const
{
    QStringList files;
    alpm_filelist_t *filesList = alpm_pkg_get_files(m_localPackageData);
    for (unsigned i = 0; i < filesList->count; ++i)
        files.append(filesList->files[i].name);
    return files;
}

QDateTime Package::buildDate() const
{
    return QDateTime::fromSecsSinceEpoch(alpm_pkg_get_builddate(m_syncPackageData));
}

QDateTime Package::installDate() const
{
    return QDateTime::fromSecsSinceEpoch(alpm_pkg_get_installdate(m_localPackageData));
}

alpm_pkgreason_t Package::reason() const
{
    return alpm_pkg_get_reason(m_localPackageData);
}

bool Package::hasScript() const
{
    return alpm_pkg_has_scriptlet(m_localPackageData);
}

long Package::size() const
{
    return alpm_pkg_get_size(m_syncPackageData);
}

bool Package::isInstalled() const
{
    return m_isInstalled;
}

void Package::setInstalled(bool installed)
{
    m_isInstalled = installed;
}
