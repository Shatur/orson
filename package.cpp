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
