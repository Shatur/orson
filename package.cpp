#include "package.h"

Package::Package()
{
}

QString Package::name() const
{
    return m_name;
}

void Package::setName(const QString &name)
{
    m_name = name;
}

QString Package::repo() const
{
    return m_repo;
}

void Package::setRepo(const QString &repo)
{
    m_repo = repo;
}

QString Package::version() const
{
    return m_version;
}

void Package::setVersion(const QString &version)
{
    m_version = version;
}

QString Package::description() const
{
    return m_description;
}

void Package::setDescription(const QString &description)
{
    m_description = description;
}

QString Package::arch() const
{
    return m_arch;
}

void Package::setArch(const QString &arch)
{
    m_arch = arch;
}

QString Package::url() const
{
    return m_url;
}

void Package::setUrl(const QString &url)
{
    m_url = url;
}

QStringList Package::licenses() const
{
    return m_licenses;
}

void Package::addLicense(const QString &license)
{
    m_licenses.append(license);
}

void Package::setLicenses(const QStringList &licenses)
{
    m_licenses = licenses;
}

QString Package::packager() const
{
    return m_packager;
}

void Package::setPackager(const QString &packager)
{
    m_packager = packager;
}

QString Package::groups() const
{
    return m_groups;
}

void Package::setGroups(const QString &groups)
{
    m_groups = groups;
}

QString Package::provides() const
{
    return m_provides;
}

void Package::setProvides(const QString &provides)
{
    m_provides = provides;
}

alpm_pkgreason_t Package::reason() const
{
    return m_reason;
}

void Package::setReason(alpm_pkgreason_t reason)
{
    m_reason = reason;
}

long Package::size() const
{
    return m_size;
}

void Package::setSize(long size)
{
    m_size = size;
}

bool Package::installed() const
{
    return m_installed;
}

void Package::setInstalled(bool installed)
{
    m_installed = installed;
}

bool Package::hasScript() const
{
    return m_hasScript;
}

void Package::setHasScript(bool hasScript)
{
    m_hasScript = hasScript;
}
