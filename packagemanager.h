#ifndef PACKAGEMANAGER_H
#define PACKAGEMANAGER_H

#include "package.h"

#include <QStringList>

class PackageManager
{
public:
    PackageManager();
    ~PackageManager();

    alpm_errno_t error() const;
    QList<Package> packages() const;

private:
    void loadPackages(const char *databaseName);

    alpm_handle_t *m_handle;
    alpm_errno_t m_error = ALPM_ERR_OK;
    QList<Package> m_packages;
};

#endif // PACKAGEMANAGER_H
