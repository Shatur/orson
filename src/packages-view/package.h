#ifndef PACKAGE_H
#define PACKAGE_H

#include "depend.h"

#include <QDateTime>
#include <QJsonObject>

class Package
{
public:
    Package();
    void setSyncData(alpm_pkg_t *data);
    void setLocalData(alpm_pkg_t *data);
    void setAurData(const QJsonValue &value);

    QString name() const;
    QString repo() const;
    QString version() const;
    QString description() const;
    QString arch() const;
    QString url() const;
    QString maintainer() const;
    QString formattedInstalledSize() const;
    QString formattedDownloadSize() const;
    QStringList licenses() const;
    QStringList groups() const;
    QStringList files() const;
    QVector<Depend> provides() const;
    QVector<Depend> replaces() const;
    QVector<Depend> conflicts() const;
    QVector<Depend> depends() const;
    QVector<Depend> optdepends() const;
    QDateTime buildDate() const;
    QDateTime installDate() const;
    alpm_pkgreason_t reason() const;
    long downloadSize() const;
    long installedSize() const;
    double popularity() const;
    bool hasScript() const;
    bool isInstalled() const;

    static QString depmodString(alpm_depmod_t mod);

private:
    static QVector<Depend> generateDeps(alpm_list_t *list);

    alpm_pkg_t *m_syncData = nullptr;
    alpm_pkg_t *m_localData = nullptr;
    QJsonObject m_aurData;
    bool m_installed = false;
};

#endif // PACKAGE_H
