#include "package.h"

Package::Package(alpm_pkg_t *syncPackageData, alpm_pkg_t *localPackageData) :
    m_syncPackageData(syncPackageData),
    m_localPackageData(localPackageData)
{
    if (m_localPackageData != nullptr)
        m_isInstalled = true;
    else
        m_isInstalled = false;
}

QString Package::name() const
{
    return alpm_pkg_get_name(m_syncPackageData);
}

QString Package::repo() const
{
    return alpm_db_get_name(alpm_pkg_get_db(m_syncPackageData));
}

QString Package::version() const
{
    return alpm_pkg_get_version(m_syncPackageData);
}

QString Package::description() const
{
    return alpm_pkg_get_desc(m_syncPackageData);
}

QString Package::arch() const
{
    return alpm_pkg_get_arch(m_syncPackageData);
}

QString Package::url() const
{
    return alpm_pkg_get_url(m_syncPackageData);
}

QString Package::packager() const
{
    return alpm_pkg_get_packager(m_syncPackageData);
}

QStringList Package::licenses() const
{
    QStringList licenses;
    alpm_list_t *licensesList = alpm_pkg_get_licenses(m_syncPackageData);
    while (licensesList != nullptr) {
        licenses.append(static_cast<const char*>(licensesList->data));
        licensesList = licensesList->next;
    }
    return licenses;
}

QStringList Package::groups() const
{
    QStringList groups;
    alpm_list_t *groupsList = alpm_pkg_get_groups(m_syncPackageData);
    while (groupsList != nullptr) {
        groups.append(static_cast<const char*>(groupsList->data));
        groupsList = groupsList->next;
    }
    return groups;
}

QStringList Package::files() const
{
    QStringList files;
    alpm_filelist_t *filesList = alpm_pkg_get_files(m_localPackageData);
    for (unsigned i = 0; i < filesList->count; ++i)
        files.append(filesList->files[i].name);
    return files;
}

QList<alpm_depend_t*> Package::provides() const
{
    QList<alpm_depend_t*> provides;
    alpm_list_t *providesList = alpm_pkg_get_provides(m_syncPackageData);
    while (providesList != nullptr) {
        provides.push_back(static_cast<alpm_depend_t*>(providesList->data));
        providesList = providesList->next;
    }
    return provides;
}

QList<alpm_depend_t *> Package::replaces() const
{
    QList<alpm_depend_t*> replaces;
    alpm_list_t *replacesList = alpm_pkg_get_replaces(m_syncPackageData);
    while (replacesList != nullptr) {
        replaces.push_back(static_cast<alpm_depend_t*>(replacesList->data));
        replacesList = replacesList->next;
    }
    return replaces;
}

QList<alpm_depend_t *> Package::conflicts() const
{
    QList<alpm_depend_t*> conflicts;
    alpm_list_t *conflictsList = alpm_pkg_get_conflicts(m_syncPackageData);
    while (conflictsList != nullptr) {
        conflicts.push_back(static_cast<alpm_depend_t*>(conflictsList->data));
        conflictsList = conflictsList->next;
    }
    return conflicts;
}

QList<alpm_depend_t *> Package::depends() const
{
    QList<alpm_depend_t*> depends;
    alpm_list_t *dependsList = alpm_pkg_get_depends(m_syncPackageData);
    while (dependsList != nullptr) {
        depends.push_back(static_cast<alpm_depend_t*>(dependsList->data));
        dependsList = dependsList->next;
    }
    return depends;
}

QList<alpm_depend_t *> Package::optdepends() const
{
    QList<alpm_depend_t*> optdepends;
    alpm_list_t *optdependsList = alpm_pkg_get_optdepends(m_syncPackageData);
    while (optdependsList != nullptr) {
        optdepends.push_back(static_cast<alpm_depend_t*>(optdependsList->data));
        optdependsList = optdependsList->next;
    }
    return optdepends;
}

QList<alpm_depend_t *> Package::checkdepends() const
{
    QList<alpm_depend_t*> checkdepends;
    alpm_list_t *checkdependsList = alpm_pkg_get_checkdepends(m_syncPackageData);
    while (checkdependsList != nullptr) {
        checkdepends.push_back(static_cast<alpm_depend_t*>(checkdependsList->data));
        checkdependsList = checkdependsList->next;
    }
    return checkdepends;
}

QList<alpm_depend_t *> Package::makedepends() const
{
    QList<alpm_depend_t*> makedepends;
    alpm_list_t *makedependsList = alpm_pkg_get_makedepends(m_syncPackageData);
    while (makedependsList != nullptr) {
        makedepends.push_back(static_cast<alpm_depend_t*>(makedependsList->data));
        makedependsList = makedependsList->next;
    }
    return makedepends;
}

QDateTime Package::buildDate() const
{
    return QDateTime::fromSecsSinceEpoch(alpm_pkg_get_builddate(m_syncPackageData));
}

QDateTime Package::installDate() const
{
    return QDateTime::fromSecsSinceEpoch(alpm_pkg_get_installdate(m_localPackageData));
}

alpm_pkgreason_t Package::reason() const
{
    return alpm_pkg_get_reason(m_localPackageData);
}

bool Package::hasScript() const
{
    return alpm_pkg_has_scriptlet(m_localPackageData);
}

long Package::size() const
{
    return alpm_pkg_get_size(m_syncPackageData);
}

bool Package::isInstalled() const
{
    return m_isInstalled;
}

void Package::setInstalled(bool installed)
{
    m_isInstalled = installed;
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
