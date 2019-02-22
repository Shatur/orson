#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "mainwindow.h"
#include "pacman.h"

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

    bool isTrayIconVisible() const;
    void setTrayIconVisible(bool visible);

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

    bool isAutosyncEnabled() const;
    void setAutosyncEnabled(bool enabled);

    MainWindow::AutosyncType autosyncType() const;
    void setAutosyncType(MainWindow::AutosyncType type);

    QTime autosyncTime() const;
    void setAutosyncTime(const QTime &time);
    QTime defaultAutosyncTime() const;

    // Interface settings
    QString trayIconName(MainWindow::TrayStatus trayStatus) const;
    void setTrayIconName(MainWindow::TrayStatus trayStatus, const QString &name);
    static constexpr const char *defaultTrayIconName(MainWindow::TrayStatus trayStatus);

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

constexpr const char *AppSettings::defaultTrayIconName(MainWindow::TrayStatus trayStatus)
{
    switch (trayStatus) {
    case MainWindow::NoUpdates:
        return "update-none";
    case MainWindow::Updating:
        return "state-sync";
    case MainWindow::UpdatesAvailable:
        return "update-high";
    }

    return nullptr;
}

#endif // APPSETTINGS_H
