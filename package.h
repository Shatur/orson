#ifndef PACKAGE_H
#define PACKAGE_H

#include "alpm.h"

#include <QString>
#include <QUrl>

class Package
{
public:
    Package();

    QString name() const;
    void setName(const QString &name);

    QString repo() const;
    void setRepo(const QString &repo);

    QString version() const;
    void setVersion(const QString &version);

    QString description() const;
    void setDescription(const QString &description);

    QString arch() const;
    void setArch(const QString &arch);

    QString url() const;
    void setUrl(const QString &url);

    QStringList licenses() const;
    void addLicense(const QString &license);
    void setLicenses(const QStringList &licenses);

    QString packager() const;
    void setPackager(const QString &packager);

    QString groups() const;
    void setGroups(const QString &groups);

    QString provides() const;
    void setProvides(const QString &provides);

    alpm_pkgreason_t reason() const;
    void setReason(alpm_pkgreason_t reason);

    long size() const;
    void setSize(long size);

    bool installed() const;
    void setInstalled(bool installed);

    bool hasScript() const;
    void setHasScript(bool hasScript);

private:
    QString m_name;
    QString m_repo;
    QString m_version;
    QString m_description;
    QString m_arch;
    QString m_groups;
    QString m_provides;
    QString m_url;
    QString m_packager;
    QStringList m_licenses;
    alpm_pkgreason_t m_reason;
    long m_size = 0;
    bool m_installed = false;
    bool m_hasScript = false;

};

#endif // PACKAGE_H
