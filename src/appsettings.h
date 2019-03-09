﻿#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "pacman.h"
#include "systemtray.h"
#include "autosynctimer.h"

#include <QSettings>
#include <QLocale>
#include <QNetworkProxy>
#include <QTranslator>

class AppSettings : private QSettings
{
    Q_OBJECT
public:
    AppSettings(QObject *parent = nullptr);

    // Main window
    bool isNoConfirm() const;
    void setNoConfirm(bool noConfirm);

    Pacman::AfterCompletion afterCompletion() const;
    void setAfterCompletion(Pacman::AfterCompletion afterCompletion);

    // General settings
    void setupLocale();
    QLocale::Language locale() const;
    void setLocale(QLocale::Language lang);
    void loadLocale(QLocale::Language lang);

    bool isMinimizeToTray() const;
    void setMinimizeToTray(bool visible);

    bool isStartMinimized() const;
    void setStartMinimized(bool minimized);

    bool isAutostartEnabled() const;
    void setAutostartEnabled(bool enabled);

    // Pacman settings
    QString terminal() const;
    QStringList availableTerminals() const;
    void setTerminal(const QString &terminal);

    QStringList terminalArguments(const QString &terminal) const;
    QStringList defaultArguments(const QString &terminal) const;
    void setTerminalArguments(const QString &terminal, const QStringList &arguments);

    QString pacmanTool() const;
    QStringList availablePacmanTools() const;
    void setPacmanTool(const QString &programName);
    static constexpr const char *defaultPacmanTool()
    { return "sudo pacman"; }

    AutosyncTimer::AutosyncType autosyncType() const;
    void setAutosyncType(AutosyncTimer::AutosyncType type);
    static constexpr AutosyncTimer::AutosyncType defaultAutosyncType()
    { return AutosyncTimer::Interval; }

    int autosyncInterval() const;
    void setAutosyncInterval(int hours);
    static constexpr int defaultAutosyncInterval()
    { return 3; }

    QTime autosyncTime() const;
    void setAutosyncTime(const QTime &time);
    static QTime defaultAutosyncTime();

    QDateTime lastSync() const;
    void setLastSync(QDateTime dateTime);

    // Interface settings
    QString trayIconName(SystemTray::TrayStatus trayStatus) const;
    void setTrayIconName(SystemTray::TrayStatus trayStatus, const QString &name);
    static constexpr const char *defaultTrayIconName(SystemTray::TrayStatus trayStatus);

    // Connection settings
    QNetworkProxy::ProxyType proxyType() const;
    void setProxyType(QNetworkProxy::ProxyType type);

    QString proxyHost() const;
    void setProxyHost(const QString &hostName);

    quint16 proxyPort() const;
    void setProxyPort(quint16 port);

    bool isProxyAuthEnabled() const;
    void setProxyAuthEnabled(bool enabled);

    QString proxyUsername() const;
    void setProxyUsername(const QString &username);

    QString proxyPassword() const;
    void setProxyPassword(const QString &password);

    // Main window settings
    QByteArray mainWindowGeometry() const;
    void setMainWindowGeometry(const QByteArray &geometry);

private:
    static QTranslator m_appTranslator;
};

constexpr const char *AppSettings::defaultTrayIconName(SystemTray::TrayStatus trayStatus)
{
    switch (trayStatus) {
    case SystemTray::NoUpdates:
        return "update-none";
    case SystemTray::Updating:
        return "state-sync";
    case SystemTray::UpdatesAvailable:
        return "update-high";
    }

    return nullptr;
}

#endif // APPSETTINGS_H
