#ifndef PACMANSETTINGS_H
#define PACMANSETTINGS_H

#include <QSettings>

class PacmanSettings : public QSettings
{
    Q_DISABLE_COPY(PacmanSettings)

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
