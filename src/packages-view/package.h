#ifndef PACKAGE_H
#define PACKAGE_H

#include "depend.h"

#include <QDateTime>
#include <QJsonObject>

class Package
{
public:
    Package() = default;
    void setSyncData(alpm_pkg_t *data);
    void setLocalData(alpm_pkg_t *data);
    void setAurData(const QJsonValue &value, bool full = false);

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
    QStringList keywords() const;
    QVector<Depend> provides() const;
    QVector<Depend> replaces() const;
    QVector<Depend> conflicts() const;
    QVector<Depend> depends() const;
    QVector<Depend> optdepends() const;
    QDateTime buildDate() const;
    QDateTime installDate() const;
    QDateTime firstSubmitted() const;
    QDateTime lastModified() const;
    QDateTime outOfDate() const;
    alpm_pkgreason_t reason() const;
    double popularity() const;
    long downloadSize() const;
    long installedSize() const;
    int votes() const;
    bool hasScript() const;
    bool isInstalled() const;
    bool fullAurInfo() const;

private:
    static QVector<Depend> alpmDeps(alpm_list_t *list);
    static QVector<Depend> aurDeps(const QJsonValue &value);

    alpm_pkg_t *m_syncData = nullptr;
    alpm_pkg_t *m_localData = nullptr;
    QJsonObject m_aurData;
    bool m_installed = false;
    bool m_fullAurInfo = false;
};

#endif // PACKAGE_H
