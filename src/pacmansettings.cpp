#include "pacmansettings.h"

PacmanSettings::PacmanSettings(QObject *parent) :
    QSettings("/etc/pacman.conf", QSettings::IniFormat, parent)
{
}

QString PacmanSettings::rootDir() const
{
    return value("RootDir", "/").toString();
}

QString PacmanSettings::databasesPath() const
{
    return value("DBPath", "/var/lib/pacman/").toString();
}

QString PacmanSettings::cacheDir() const
{
    return value("CacheDir", "/var/cache/pacman/pkg/").toString();
}

QString PacmanSettings::logFile() const
{
    return value("LogFile", "/var/log/pacman.log").toString();
}

QString PacmanSettings::gpgDir() const
{
    return value("GPGDir", "/etc/pacman.d/gnupg/").toString();
}

QString PacmanSettings::hookDir() const
{
    return value("GPGDir", "/etc/pacman.d/hooks/").toString();
}

QStringList PacmanSettings::repositories() const
{
    QStringList repositories = childGroups();

    // Remove "options" field
    const int index = repositories.indexOf("options");
    repositories.removeAt(index);

    return repositories;
}

QStringList PacmanSettings::ignoredPackages() const
{
    const QString ignoredPackages = value("ignorePkg").toString();
    return ignoredPackages.split(' ', QString::SkipEmptyParts);
}
