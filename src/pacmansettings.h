#ifndef PACMANSETTINGS_H
#define PACMANSETTINGS_H

#include <QSettings>

class PacmanSettings : public QSettings
{
    Q_DISABLE_COPY(PacmanSettings)

public:
    explicit PacmanSettings(QObject *parent = nullptr);

    QString rootDir() const;
    QString databasesPath() const;
    QString cacheDir() const;
    QString logFile() const;
    QString gpgDir() const;
    QString hookDir() const;

    QStringList repositories() const;
    QStringList ignoredPackages() const;
};

#endif // PACMANSETTINGS_H
