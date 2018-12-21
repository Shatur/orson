#ifndef PACKAGEMANAGER_H
#define PACKAGEMANAGER_H

#include "package.h"

#include <QStringList>

class PackageManager
{
public:
    PackageManager();

    alpm_errno_t error() const;
    QList<Package> packages() const;

private:
    void loadPackages(alpm_handle_t *handle, const char *databaseName);

    alpm_errno_t m_error = ALPM_ERR_OK;
    QList<Package> m_packages;
};

#endif // PACKAGEMANAGER_H
