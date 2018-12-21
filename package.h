#ifndef PACKAGE_H
#define PACKAGE_H

#include "alpm.h"

#include <QString>
#include <QUrl>

class Package
{
public:
    Package(alpm_pkg_t *packageData);

    QString name() const;
    QString repo() const;
    QString version() const;
    QString description() const;
    QString arch() const;
    QString url() const;
    QString packager() const;
    QStringList licenses() const;
    alpm_pkgreason_t reason() const;
    bool hasScript() const;
    long size() const;

    bool isInstalled() const;
    void setInstalled(bool installed);

private:
    alpm_pkg_t *m_packageData;
    QString m_repo;

    bool m_isInstalled = false;

};

#endif // PACKAGE_H
