#ifndef PACMANSETTINGS_H
#define PACMANSETTINGS_H

#include <QSettings>

class PacmanSettings : private QSettings
{
public:
    PacmanSettings();

    QString rootDir() const;
    QString databasesPath() const;
    QString cacheDir() const;
    QString logFile() const;
    QString gpgDir() const;
    QString hookDir() const;

    QStringList repositories() const;
};

#endif // PACMANSETTINGS_H
