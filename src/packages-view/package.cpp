#include "package.h"

#include <QMimeDatabase>
#include <QLocale>
#include <QVector>
#include <QDateTime>

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
    if (m_localData == nullptr)
        return alpm_pkg_get_arch(m_syncData);
    return alpm_pkg_get_arch(m_localData);
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
    if (m_localData == nullptr)
        licensesList = alpm_pkg_get_licenses(m_syncData);
    else
        licensesList = alpm_pkg_get_licenses(m_localData);

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

QVector<alpm_depend_t *> Package::provides() const
{
    QVector<alpm_depend_t *> provides;
    alpm_list_t *providesList;
    if (m_localData == nullptr)
        providesList = alpm_pkg_get_provides(m_syncData);
    else
        providesList = alpm_pkg_get_provides(m_localData);

    while (providesList != nullptr) {
        provides.push_back(static_cast<alpm_depend_t *>(providesList->data));
        providesList = providesList->next;
    }

    return provides;
}

QVector<alpm_depend_t *> Package::replaces() const
{
    QVector<alpm_depend_t *> replaces;
    alpm_list_t *replacesList;
    if (m_localData == nullptr)
        replacesList = alpm_pkg_get_replaces(m_syncData);
    else
        replacesList = alpm_pkg_get_replaces(m_localData);

    while (replacesList != nullptr) {
        replaces.push_back(static_cast<alpm_depend_t *>(replacesList->data));
        replacesList = replacesList->next;
    }

    return replaces;
}

QVector<alpm_depend_t *> Package::conflicts() const
{
    QVector<alpm_depend_t *> conflicts;
    alpm_list_t *conflictsList;
    if (m_localData == nullptr)
        conflictsList = alpm_pkg_get_conflicts(m_syncData);
    else
        conflictsList = alpm_pkg_get_conflicts(m_localData);

    while (conflictsList != nullptr) {
        conflicts.push_back(static_cast<alpm_depend_t *>(conflictsList->data));
        conflictsList = conflictsList->next;
    }

    return conflicts;
}

QVector<alpm_depend_t *> Package::depends() const
{
    QVector<alpm_depend_t *> depends;
    alpm_list_t *dependsList;
    if (m_localData == nullptr)
        dependsList = alpm_pkg_get_depends(m_syncData);
    else
        dependsList = alpm_pkg_get_depends(m_localData);

    while (dependsList != nullptr) {
        depends.push_back(static_cast<alpm_depend_t *>(dependsList->data));
        dependsList = dependsList->next;
    }

    return depends;
}

QVector<alpm_depend_t *> Package::optdepends() const
{
    QVector<alpm_depend_t *> optdepends;
    alpm_list_t *optdependsList;
    if (m_localData == nullptr)
        optdependsList = alpm_pkg_get_optdepends(m_syncData);
    else
        optdependsList = alpm_pkg_get_optdepends(m_localData);

    while (optdependsList != nullptr) {
        optdepends.push_back(static_cast<alpm_depend_t *>(optdependsList->data));
        optdependsList = optdependsList->next;
    }

    return optdepends;
}

QDateTime Package::buildDate() const
{
    if (m_localData == nullptr)
        return QDateTime::fromSecsSinceEpoch(alpm_pkg_get_builddate(m_syncData));
    return QDateTime::fromSecsSinceEpoch(alpm_pkg_get_builddate(m_localData));
}

// Can be obtained only from local data
QDateTime Package::installDate() const
{
    return QDateTime::fromSecsSinceEpoch(alpm_pkg_get_installdate(m_localData));
}

// Can be obtained only from local data
alpm_pkgreason_t Package::reason() const
{
    return alpm_pkg_get_reason(m_localData);
}

// Can be obtained only from sync data
long Package::downloadSize() const
{
    return alpm_pkg_get_size(m_syncData);
}

long Package::installedSize() const
{
    if (m_localData == nullptr)
        return alpm_pkg_get_isize(m_syncData);
    return alpm_pkg_get_isize(m_localData);
}

double Package::popularity() const
{
    return m_aurData.value("Popularity").toDouble();
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

// String of version constraints in dependency specs
QString Package::depmodString(alpm_depmod_t mod)
{
    switch (mod) {
    case ALPM_DEP_MOD_EQ:
        return " = ";
    case ALPM_DEP_MOD_GE:
        return " >= ";
    case ALPM_DEP_MOD_LE:
        return " <= ";
    case ALPM_DEP_MOD_GT:
        return " > ";
    case ALPM_DEP_MOD_LT:
        return " < ";
    default:
        return "";
    }
}
