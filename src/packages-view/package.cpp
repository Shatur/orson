#include "package.h"

#include <QMimeDatabase>
#include <QLocale>
#include <QDateTime>
#include <QJsonArray>

// Copy all data from ALPM
void Package::setSyncData(alpm_pkg_t *data)
{
    // General info
    if (!m_installed) {
        m_name = alpm_pkg_get_name(data);
        m_version = alpm_pkg_get_version(data);
        m_description = alpm_pkg_get_desc(data);
        m_arch = alpm_pkg_get_arch(data);
        m_url = alpm_pkg_get_url(data);
        m_maintainer = alpm_pkg_get_packager(data);
        m_buildDate = QDateTime::fromSecsSinceEpoch(alpm_pkg_get_builddate(data));
        m_installedSize = alpm_pkg_get_isize(data);

        // Dependencies
        m_provides = alpmDeps(alpm_pkg_get_provides(data));
        m_replaces = alpmDeps(alpm_pkg_get_replaces(data));
        m_conflicts = alpmDeps(alpm_pkg_get_conflicts(data));
        m_depends = alpmDeps(alpm_pkg_get_depends(data));
        m_optdepends = alpmDeps(alpm_pkg_get_optdepends(data));

        // Licenses
        alpm_list_t *licensesList = alpm_pkg_get_licenses(data);
        while (licensesList != nullptr) {
            m_licenses.append(static_cast<const char *>(licensesList->data));
            licensesList = licensesList->next;
        }

        // Groups
        alpm_list_t *groupsList = alpm_pkg_get_groups(data);
        while (groupsList != nullptr) {
            m_groups.append(static_cast<const char *>(groupsList->data));
            groupsList = groupsList->next;
        }
    }

    m_downloadSize = alpm_pkg_get_size(data);
    m_repo = alpm_db_get_name(alpm_pkg_get_db(data));
}

// Copy all data from ALPM
void Package::setLocalData(alpm_pkg_t *data)
{
    // General info
    m_name = alpm_pkg_get_name(data);
    m_repo = QStringLiteral("local");
    m_version = alpm_pkg_get_version(data);
    m_description = alpm_pkg_get_desc(data);
    m_arch = alpm_pkg_get_arch(data);
    m_url = alpm_pkg_get_url(data);
    m_maintainer = alpm_pkg_get_packager(data);
    m_buildDate = QDateTime::fromSecsSinceEpoch(alpm_pkg_get_builddate(data));
    m_installDate = QDateTime::fromSecsSinceEpoch(alpm_pkg_get_installdate(data));
    m_installedSize = alpm_pkg_get_isize(data);
    m_hasScript = alpm_pkg_has_scriptlet(data);
    m_explicitly = alpm_pkg_get_reason(data) == ALPM_PKG_REASON_EXPLICIT;
    m_installed = true;

    // Dependencies
    m_provides = alpmDeps(alpm_pkg_get_provides(data));
    m_replaces = alpmDeps(alpm_pkg_get_replaces(data));
    m_conflicts = alpmDeps(alpm_pkg_get_conflicts(data));
    m_depends = alpmDeps(alpm_pkg_get_depends(data));
    m_optdepends = alpmDeps(alpm_pkg_get_optdepends(data));

    // Licenses
    alpm_list_t *licensesList = alpm_pkg_get_licenses(data);
    while (licensesList != nullptr) {
        m_licenses.append(static_cast<const char *>(licensesList->data));
        licensesList = licensesList->next;
    }

    // Groups
    alpm_list_t *groupsList = alpm_pkg_get_groups(data);
    while (groupsList != nullptr) {
        m_groups.append(static_cast<const char *>(groupsList->data));
        groupsList = groupsList->next;
    }

    // Files
    alpm_filelist_t *filesList = alpm_pkg_get_files(data);
    for (unsigned i = 0; i < filesList->count; ++i)
        m_files.append(filesList->files[i].name);
}

void Package::setAurData(const QJsonObject &object, bool full)
{
    m_fullAurInfo = full;

    if (!m_installed) {
        m_name = object.value("Name").toString();
        m_version = object.value("Version").toString();
        m_description = object.value("Description").toString();
        m_url = object.value("URL").toString();
        m_maintainer = object.value("Maintainer").toString();

        // Dependencies
        m_provides = aurDeps(object.value("Provides"));
        m_replaces = aurDeps(object.value("Replaces"));
        m_conflicts = aurDeps(object.value("Conflicts"));
        m_depends = aurDeps(object.value("Depends"));
        m_optdepends = aurDeps(object.value("OptDepends"));

        // Licenses
        foreach (const QJsonValue &value, object.value("License").toArray())
            m_licenses.append(value.toString());
    }

    m_repo = QStringLiteral("aur");
    m_firstSubmitted = QDateTime::fromSecsSinceEpoch(object.value("FirstSubmitted").toInt());
    m_lastModified = QDateTime::fromSecsSinceEpoch(object.value("LastModified").toInt());
    m_popularity = object.value("Popularity").toDouble();
    m_votes = object.value("NumVotes").toInt();

    // Check for out of date
    if (!object.value("OutOfDate").isNull())
        m_outOfDate = QDateTime::fromSecsSinceEpoch(object.value("OutOfDate").toInt());

    // Keywords
    foreach (const QJsonValue &word, object.value("Keywords").toArray())
        m_keywords.append(word.toString());
}

QString Package::name() const
{
    return m_name;
}

QString Package::repo() const
{
    return m_repo;
}

QString Package::version() const
{
    return m_version;
}

QString Package::description() const
{
    return m_description;
}

QString Package::arch() const
{
    return m_arch;
}

QString Package::url() const
{
    return m_url;
}

QString Package::maintainer() const
{
    return m_maintainer;
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
    return m_licenses;
}

QStringList Package::groups() const
{
    return m_groups;
}

QStringList Package::files() const
{
    return m_files;
}

QStringList Package::keywords() const
{
    return m_keywords;
}

QVector<Depend> Package::provides() const
{
    return m_provides;
}

QVector<Depend> Package::replaces() const
{
    return m_replaces;
}

QVector<Depend> Package::conflicts() const
{
    return m_conflicts;
}

QVector<Depend> Package::depends() const
{
    return m_depends;
}

QVector<Depend> Package::optdepends() const
{
    return m_optdepends;
}

QDateTime Package::buildDate() const
{
    return m_buildDate;
}

// Can be obtained only from local data
QDateTime Package::installDate() const
{
    return m_installDate;
}

QDateTime Package::firstSubmitted() const
{
    return m_firstSubmitted;
}

QDateTime Package::lastModified() const
{
    return m_lastModified;
}

QDateTime Package::outOfDate() const
{
    return m_outOfDate;
}

double Package::popularity() const
{
    return m_popularity;
}

// Can be obtained only from sync data
long Package::downloadSize() const
{
    return m_downloadSize;
}

long Package::installedSize() const
{
    return m_installedSize;
}

int Package::votes() const
{
    return m_votes;
}

bool Package::isInstalled() const
{
    return m_installed;
}

// Can be obtained only from local data
bool Package::isExplicitly() const
{
    return m_explicitly;
}

// Can be obtained only from local data
bool Package::hasScript() const
{
    return m_hasScript;
}

bool Package::fullAurInfo() const
{
    return m_fullAurInfo;
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
