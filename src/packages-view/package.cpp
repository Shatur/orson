#include "package.h"

#include <QMimeDatabase>
#include <QLocale>
#include <QVector>
#include <QDateTime>
#include <QJsonArray>

Package::Package()
{
}

void Package::setSyncData(alpm_pkg_t *data)
{
    m_syncData = data;
}

void Package::setLocalData(alpm_pkg_t *data)
{
    m_localData = data;
    if (m_localData != nullptr)
        m_installed = true;
}

void Package::setAurData(const QJsonValue &value)
{
    m_aurData = value.toObject();
}

QString Package::name() const
{
    if (m_localData != nullptr)
        return alpm_pkg_get_name(m_localData);

    if (m_syncData != nullptr)
        return alpm_pkg_get_name(m_syncData);

    return m_aurData.value("Name").toString();
}

QString Package::repo() const
{
    if (m_syncData != nullptr)
        return alpm_db_get_name(alpm_pkg_get_db(m_syncData));

    if (!m_aurData.isEmpty())
        return "aur";

    return "local";
}

QString Package::version() const
{
    if (m_localData != nullptr)
        return alpm_pkg_get_version(m_localData);

    if (m_syncData != nullptr)
        return alpm_pkg_get_version(m_syncData);

    return m_aurData.value("Version").toString();
}

QString Package::description() const
{
    if (m_localData != nullptr)
        return alpm_pkg_get_desc(m_localData);

    if (m_syncData != nullptr)
        return alpm_pkg_get_desc(m_syncData);

    return m_aurData.value("Description").toString();
}

QString Package::arch() const
{
    if (m_localData != nullptr)
        return alpm_pkg_get_arch(m_localData);
    return alpm_pkg_get_arch(m_syncData);
}

QString Package::url() const
{
    if (m_localData != nullptr)
        return alpm_pkg_get_url(m_localData);

    if (m_syncData != nullptr)
        return alpm_pkg_get_url(m_syncData);

    return m_aurData.value("URL").toString();
}

QString Package::maintainer() const
{
    if (m_localData != nullptr)
        return alpm_pkg_get_packager(m_localData);

    if (m_syncData != nullptr)
        return alpm_pkg_get_packager(m_syncData);

    return m_aurData.value("Maintainer").toString();
}

QString Package::formattedInstalledSize() const
{
    QLocale locale;
    return locale.formattedDataSize(installedSize(), 2, QLocale::DataSizeTraditionalFormat);
}

QString Package::formattedDownloadSize() const
{
    QLocale locale;
    return locale.formattedDataSize(downloadSize(), 2, QLocale::DataSizeTraditionalFormat);
}

QStringList Package::licenses() const
{
    QStringList licenses;
    alpm_list_t *licensesList;
    if (m_localData != nullptr)
        licensesList = alpm_pkg_get_licenses(m_localData);
    else if (m_syncData != nullptr)
        licensesList = alpm_pkg_get_licenses(m_syncData);
    else {
        // Load AUR info
        foreach (const QJsonValue &value, m_aurData.value("License").toArray())
            licenses.append(value.toString());
        return licenses;
    }

    while (licensesList != nullptr) {
        licenses.append(static_cast<const char *>(licensesList->data));
        licensesList = licensesList->next;
    }

    return licenses;
}

QStringList Package::groups() const
{
    QStringList groups;
    alpm_list_t *groupsList;
    if (m_localData == nullptr)
        groupsList = alpm_pkg_get_groups(m_syncData);
    else
        groupsList = alpm_pkg_get_groups(m_localData);

    while (groupsList != nullptr) {
        groups.append(static_cast<const char *>(groupsList->data));
        groupsList = groupsList->next;
    }

    return groups;
}

QStringList Package::files() const
{
    QStringList files;
    alpm_filelist_t *filesList;
    if (m_localData == nullptr)
        filesList = alpm_pkg_get_files(m_syncData);
    else
        filesList = alpm_pkg_get_files(m_localData);

    for (unsigned i = 0; i < filesList->count; ++i)
        files.append(filesList->files[i].name);

    return files;
}

QVector<Depend> Package::provides() const
{
    if (m_localData != nullptr)
        return alpmDeps(alpm_pkg_get_provides(m_localData));
    else if (m_syncData != nullptr)
        return alpmDeps(alpm_pkg_get_provides(m_syncData));

    return aurDeps(m_aurData.value("Provides"));
}

QVector<Depend> Package::replaces() const
{
    if (m_localData != nullptr)
        return alpmDeps(alpm_pkg_get_replaces(m_localData));
    else if (m_syncData != nullptr)
        return alpmDeps(alpm_pkg_get_replaces(m_syncData));

    return aurDeps(m_aurData.value("Replaces"));
}

QVector<Depend> Package::conflicts() const
{
    if (m_localData != nullptr)
        return alpmDeps(alpm_pkg_get_conflicts(m_localData));
    else if (m_syncData != nullptr)
        return alpmDeps(alpm_pkg_get_conflicts(m_syncData));

    return aurDeps(m_aurData.value("Conflicts"));
}

QVector<Depend> Package::depends() const
{
    if (m_localData != nullptr)
        return alpmDeps(alpm_pkg_get_depends(m_localData));
    else if (m_syncData != nullptr)
        return alpmDeps(alpm_pkg_get_depends(m_syncData));

    return aurDeps(m_aurData.value("Depends"));
}

QVector<Depend> Package::optdepends() const
{
    if (m_localData != nullptr)
        return alpmDeps(alpm_pkg_get_optdepends(m_localData));
    else if (m_syncData != nullptr)
        return alpmDeps(alpm_pkg_get_optdepends(m_syncData));

    return aurDeps(m_aurData.value("OptDepends"));
}

QDateTime Package::buildDate() const
{
    if (m_localData != nullptr)
        return QDateTime::fromSecsSinceEpoch(alpm_pkg_get_builddate(m_localData));
    else if (m_syncData != nullptr)
        return QDateTime::fromSecsSinceEpoch(alpm_pkg_get_builddate(m_syncData));
    else
        return QDateTime();
}

// Can be obtained only from local data
QDateTime Package::installDate() const
{
    if (m_localData != nullptr)
        return QDateTime::fromSecsSinceEpoch(alpm_pkg_get_installdate(m_localData));
    else
        return QDateTime();
}

// Can be obtained only from local data
alpm_pkgreason_t Package::reason() const
{
    return alpm_pkg_get_reason(m_localData);
}

double Package::popularity() const
{
    return m_aurData.value("Popularity").toDouble();
}

// Can be obtained only from sync data
long Package::downloadSize() const
{
    return alpm_pkg_get_size(m_syncData);
}

long Package::installedSize() const
{
    if (m_localData != nullptr)
        return alpm_pkg_get_isize(m_localData);
    else if (m_syncData != nullptr)
        return alpm_pkg_get_isize(m_syncData);
    else
        return -1;
}

int Package::votes() const
{
    return m_aurData.value("NumVotes").toInt();
}

// Can be obtained only from local data
bool Package::hasScript() const
{
    return alpm_pkg_has_scriptlet(m_localData);
}

bool Package::isInstalled() const
{
    return m_installed;
}

// Generate QVector from alpm list
QVector<Depend> Package::alpmDeps(alpm_list_t *list)
{
    QVector<Depend> deps;
    while (list != nullptr) {
        deps.append(Depend(static_cast<alpm_depend_t *>(list->data)));
        list = list->next;
    }
    return deps;
}

QVector<Depend> Package::aurDeps(const QJsonValue &array)
{
    QVector<Depend> deps;
    foreach(const QJsonValue &value, array.toArray())
        deps.append(Depend(value.toString()));
    return deps;
}
