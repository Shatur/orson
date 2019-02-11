#include "appsettings.h"

#include "singleapplication.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>

QTranslator AppSettings::m_appTranslator;

AppSettings::AppSettings(QObject *parent) :
    QSettings(parent)
{
}

bool AppSettings::isNoConfirm() const
{
    return value("NoConfirm", true).toBool();
}

void AppSettings::setNoConfirm(bool noConfirm)
{
    setValue("NoConfirm", noConfirm);
}

Pacman::AfterCompletion AppSettings::afterCompletion() const
{
    return value("AfterCompletion", Pacman::WaitForInput).value<Pacman::AfterCompletion>();
}

void AppSettings::setAfterCompletion(Pacman::AfterCompletion afterCompletion)
{
    setValue("AfterCompletion", afterCompletion);
}

void AppSettings::setupLocale()
{
    loadLocale(locale());
    SingleApplication::installTranslator(&m_appTranslator);
}

QLocale::Language AppSettings::locale() const
{
    return value("Locale", QLocale::AnyLanguage).value<QLocale::Language>();
}

void AppSettings::setLocale(QLocale::Language lang)
{
    if (lang != locale()) {
        setValue("Locale", lang);
        loadLocale(lang);
    }
}

void AppSettings::loadLocale(QLocale::Language lang)
{
    if (lang == QLocale::AnyLanguage)
        QLocale::setDefault(QLocale::system());
    else
        QLocale::setDefault(QLocale(lang));

    m_appTranslator.load(QLocale(), "orson", "_", ":/translations");
}

bool AppSettings::isTrayIconVisible() const
{
    return value("TrayIconVisible", true).toBool();
}

void AppSettings::setTrayIconVisible(bool visible)
{
    setValue("TrayIconVisible", visible);
}

bool AppSettings::isStartMinimized() const
{
    return value("StartMinimized", true).toBool();
}

void AppSettings::setStartMinimized(bool minimized)
{
    setValue("StartMinimized", minimized);
}

bool AppSettings::isAutostartEnabled() const
{
    return QFileInfo::exists(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart/orson.desktop");
}

void AppSettings::setAutostartEnabled(bool enabled)
{
    QFile autorunFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart/orson.desktop");

    if (enabled) {
        // Create autorun file
        if (!autorunFile.exists()) {
            constexpr char desktopFileName[] = "/usr/share/applications/orson.desktop";

            if (!QFile::copy(desktopFileName, autorunFile.fileName()))
                qCritical() << tr("Unable to create autorun file from ") + desktopFileName;
        }
    } else {
        // Remove autorun file
        if(autorunFile.exists())
            autorunFile.remove();
    }
}

QString AppSettings::pacmanTool() const
{
    return value("PacmanTool", "pacman").toString();
}

void AppSettings::setPacmanTool(const QString &programName)
{
    setValue("PacmanTool", programName);
}

QString AppSettings::trayIconName(MainWindow::TrayStatus trayStatus) const
{
    switch (trayStatus) {
    case MainWindow::NoUpdates:
        return value("NoUpdatesIcon", defaultTrayIconName(trayStatus)).toString();
    case MainWindow::Updating:
        return value("UpdatingIcon", defaultTrayIconName(trayStatus)).toString();
    case MainWindow::UpdatesAvailable:
        return value("UpdatesAvailableIcon", defaultTrayIconName(trayStatus)).toString();
    }

    return QString();
}

void AppSettings::setTrayIconName(MainWindow::TrayStatus trayStatus, const QString &name)
{
    switch (trayStatus) {
    case MainWindow::NoUpdates:
        setValue("NoUpdatesIcon", name);
        break;
    case MainWindow::Updating:
        setValue("UpdatingIcon", name);
        break;
    case MainWindow::UpdatesAvailable:
        setValue("UpdatesAvailableIcon", name);
        break;
    }
}

QNetworkProxy::ProxyType AppSettings::proxyType() const
{
    return static_cast<QNetworkProxy::ProxyType>(value("Connection/ProxyType", QNetworkProxy::DefaultProxy).toInt());
}

void AppSettings::setProxyType(QNetworkProxy::ProxyType type)
{
    setValue("Connection/ProxyType", type);
}

QString AppSettings::proxyHost() const
{
    return value("Connection/ProxyHost").toString();
}

void AppSettings::setProxyHost(const QString &hostName)
{
    setValue("Connection/ProxyHost", hostName);
}

quint16 AppSettings::proxyPort() const
{
    return value("Connection/ProxyPort", 8080).value<quint16>();
}

void AppSettings::setProxyPort(quint16 port)
{
    setValue("Connection/ProxyPort", port);
}

bool AppSettings::isProxyAuthEnabled() const
{
    return value("Connection/ProxyAuthEnabled", false).toBool();
}

void AppSettings::setProxyAuthEnabled(bool enabled)
{
    setValue("Connection/ProxyAuthEnabled", enabled);
}

QString AppSettings::proxyUsername() const
{
    return value("Connection/ProxyUsername").toString();
}

void AppSettings::setProxyUsername(const QString &username)
{
    setValue("Connection/ProxyUsername", username);
}

QString AppSettings::proxyPassword() const
{
    return value("Connection/ProxyPassword").toString();
}

void AppSettings::setProxyPassword(const QString &password)
{
    setValue("Connection/ProxyPassword", password);
}

QByteArray AppSettings::mainWindowGeometry() const
{
    return value("WindowGeometry").toByteArray();
}

void AppSettings::setMainWindowGeometry(const QByteArray &geometry)
{
    setValue("WindowGeometry", geometry);
}
