#ifndef PACKAGE_H
#define PACKAGE_H

#include "alpm.h"

#include <QString>
#include <QDateTime>
#include <QUrl>

class Package
{
public:
    Package(alpm_pkg_t *syncPackageData, alpm_pkg_t *localPackageData);

    QString name() const;
    QString repo() const;
    QString version() const;
    QString description() const;
    QString arch() const;
    QString url() const;
    QString packager() const;
    QStringList licenses() const;
    QStringList groups() const;
    QStringList files() const;
    QList<alpm_depend_t*> provides() const;
    QList<alpm_depend_t*> replaces() const;
    QList<alpm_depend_t*> conflicts() const;
    QList<alpm_depend_t*> depends() const;
    QList<alpm_depend_t*> optdepends() const;
    QList<alpm_depend_t*> checkdepends() const;
    QList<alpm_depend_t*> makedepends() const;
    QDateTime buildDate() const;
    QDateTime installDate() const;
    alpm_pkgreason_t reason() const;
    bool hasScript() const;
    long size() const;

    bool isInstalled() const;
    void setInstalled(bool installed);

    static QString depmodString(alpm_depmod_t mod);

private:
    alpm_pkg_t *m_syncPackageData;
    alpm_pkg_t *m_localPackageData;
    bool m_isInstalled;

};

#endif // PACKAGE_H
