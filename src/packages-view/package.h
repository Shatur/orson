#ifndef PACKAGE_H
#define PACKAGE_H

#include "depend.h"

#include <QDateTime>
#include <QJsonObject>
#include <QVector>

class Package
{
public:
    Package() = default;
    Package(const Package &other) = default;

    void setSyncData(alpm_pkg_t *data);
    void setLocalData(alpm_pkg_t *data);
    void setAurData(const QJsonObject &object, bool full = false);

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
    double popularity() const;
    long downloadSize() const;
    long installedSize() const;
    int votes() const;
    bool isInstalled() const;
    bool isExplicitly() const;
    bool hasScript() const;
    bool fullAurInfo() const;

    static QString name(alpm_pkg_t *packageData);

private:
    static QVector<Depend> alpmDeps(alpm_list_t *list);
    static QVector<Depend> aurDeps(const QJsonValue &value);

    QString m_name;
    QString m_repo;
    QString m_version;
    QString m_description;
    QString m_arch;
    QString m_url;
    QString m_maintainer;
    QStringList m_licenses;
    QStringList m_groups;
    QStringList m_files;
    QStringList m_keywords;
    QVector<Depend> m_provides;
    QVector<Depend> m_replaces;
    QVector<Depend> m_conflicts;
    QVector<Depend> m_depends;
    QVector<Depend> m_optdepends;
    QDateTime m_buildDate;
    QDateTime m_installDate;
    QDateTime m_firstSubmitted;
    QDateTime m_lastModified;
    QDateTime m_outOfDate;
    double m_popularity;
    long m_downloadSize = -1;
    long m_installedSize = -1;
    int m_votes = 0;
    bool m_explicitly = false;
    bool m_hasScript = false;
    bool m_installed = false;
    bool m_fullAurInfo = false;
};

#endif // PACKAGE_H
