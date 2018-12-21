#include "package.h"

Package::Package(alpm_pkg_t *packageData) :
    m_packageData(packageData)
{
}

QString Package::name() const
{
    return alpm_pkg_get_name(m_packageData);
}

QString Package::repo() const
{
    return alpm_db_get_name(alpm_pkg_get_db(m_packageData));
}

QString Package::version() const
{
    return alpm_pkg_get_version(m_packageData);
}

QString Package::description() const
{
    return alpm_pkg_get_desc(m_packageData);
}

QString Package::arch() const
{
    return alpm_pkg_get_arch(m_packageData);
}

QString Package::url() const
{
    return alpm_pkg_get_url(m_packageData);
}

QString Package::packager() const
{
    return alpm_pkg_get_packager(m_packageData);
}

QStringList Package::licenses() const
{
    QStringList licenses;
    alpm_list_t *licensesList = alpm_pkg_get_licenses(m_packageData);
    while (licensesList != nullptr) {
        licenses.append(static_cast<const char*>(licensesList->data));
        licensesList = licensesList->next;
    }
    return licenses;
}

alpm_pkgreason_t Package::reason() const
{
    return alpm_pkg_get_reason(m_packageData);
}

bool Package::hasScript() const
{
    return alpm_pkg_has_scriptlet(m_packageData);
}

long Package::size() const
{
    return alpm_pkg_get_size(m_packageData);
}

bool Package::isInstalled() const
{
    return m_isInstalled;
}

void Package::setInstalled(bool installed)
{
    m_isInstalled = installed;
}
