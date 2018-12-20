#ifndef PACKAGE_H
#define PACKAGE_H

#include <QString>

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

    long size() const;
    void setSize(long size);

    bool installed() const;
    void setInstalled(bool installed);

private:
    QString m_name;
    QString m_repo;
    QString m_version;
    long m_size = 0;
    bool m_installed = false;

};

#endif // PACKAGE_H
