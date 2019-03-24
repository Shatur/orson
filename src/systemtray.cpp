#include "systemtray.h"
#include "appsettings.h"
#include "singleapplication.h"

#include <QFile>
#include <QDateTime>
#ifndef KDE
#include <QDBusInterface>
#endif

SystemTray::SystemTray(MainWindow *parent) :
#ifdef KDE
    KStatusNotifierItem(parent)
#else
    QSystemTrayIcon(parent)
#endif
{
#ifdef KDE
    setStandardActionsEnabled(false);
    setToolTipTitle(SingleApplication::applicationName());
    setToolTipIconByName(parent->windowIcon().name());
    setCategory(KStatusNotifierItem::SystemServices);
    connect(this, &KStatusNotifierItem::secondaryActivateRequested, &KStatusNotifierItem::activate);
#else
    connect(this, &QSystemTrayIcon::activated, this, &SystemTray::processTrayActivation);
#endif
}

MainWindow *SystemTray::parent() const
{
    return qobject_cast<MainWindow *>(QObject::parent());
}

void SystemTray::showNotification(const QString &message, int interval)
{
#ifdef KDE
    showMessage(SingleApplication::applicationName(), message, toolTipIconName(), interval);
#else
    QDBusInterface notify("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QVariantList notifyArguments;
    notifyArguments << SingleApplication::applicationName(); // Set program name
    notifyArguments << QVariant(QVariant::UInt);
    notifyArguments << icon().name(); // Icon
    notifyArguments << SingleApplication::applicationName(); // Title
    notifyArguments << message; // Body
    notifyArguments << QStringList();
    notifyArguments << QVariantMap();
    notifyArguments << interval; // Show interval
    notify.callWithArgumentList(QDBus::AutoDetect, "Notify", notifyArguments);
#endif
}

void SystemTray::showMainWindow()
{
#ifdef KDE
    activate();
#else
    MainWindow *window = parent();
    window->show();
    window->activateWindow();
    window->raise();
#endif
}


void SystemTray::setTrayStatus(PackagesModel::DatabaseStatus status, int updatesCount)
{
    // Set icon
    AppSettings settings;
    QString statusIconName = settings.statusIconName(status);
#ifdef KDE
    if (!QIcon::hasThemeIcon(statusIconName) && !QFile::exists(statusIconName)) {
        statusIconName = AppSettings::defaultStatusIconName(status);
        settings.setStatusIconName(status, statusIconName);
        showNotification(tr("The specified icon '%1' for the current database state is invalid. The default icon will be used.").arg(statusIconName));
    }
    setIconByName(statusIconName);
#else
    QIcon statusIcon = trayIcon(statusIconName);
    if (statusIcon.isNull()) {
        const QString defaultIconName = AppSettings::defaultStatusIconName(status);
        statusIcon = trayIcon(defaultIconName);
        settings.setStatusIconName(status, defaultIconName);
        showNotification(tr("The specified icon '%1' for the current database state is invalid. The default icon will be used.").arg(statusIconName));
    }
    setIcon(statusIcon);

    if (!isVisible())
        show();
#endif

    // Show notification and set KDE tooltip
    switch (status) {
    case PackagesModel::NoUpdates:
    {
        const QString message = tr("No updates available");
        showNotification(message);
#ifdef KDE
        setToolTipSubTitle(message + '\n' + lastSyncString(settings.lastSync()));
        setStatus(KStatusNotifierItem::Passive);
#endif
        break;
    }
    case PackagesModel::Loading:
#ifdef KDE
        setToolTipSubTitle("Synchronizing databases");
        setStatus(KStatusNotifierItem::Active);
#endif
        break;
    case PackagesModel::UpdatesAvailable:
    {
        const QString message = QString::number(updatesCount) + tr(" updates available");
        showNotification(message);
#ifdef KDE
        setToolTipSubTitle(message + '\n' + lastSyncString(settings.lastSync()));
        setStatus(KStatusNotifierItem::NeedsAttention);
#endif
    }
    }
}

QIcon SystemTray::trayIcon(const QString &iconName)
{
    if (QIcon::hasThemeIcon(iconName))
        return QIcon::fromTheme(iconName);

    if (QFileInfo::exists(iconName))
        return QIcon(iconName);

    return QIcon();
}

#ifndef KDE
void SystemTray::processTrayActivation(QSystemTrayIcon::ActivationReason reason)
{
    if (reason != QSystemTrayIcon::Trigger)
        return;

    MainWindow *window = parent();
    if (!window->isVisible())
        showMainWindow();
    else
        window->hide();
}
#endif

QString SystemTray::lastSyncString(QDateTime dateTime)
{
    QString syncString = tr("Last sync: ");
    if (dateTime.isNull())
        syncString += tr("never");
    else
        syncString += dateTime.toString("dd.mm.yy hh:mm");

    return syncString;
}
