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


void SystemTray::loadTrayStatus(const PackagesModel *model)
{
    // Set icon
    const AppSettings settings;
    const QString iconName = settings.trayIconName(model->databaseStatus());
#ifdef KDE
    if (QIcon::hasThemeIcon(iconName))
        setIconByName(iconName);
    else if (QFile::exists(iconName))
        setIconByPixmap(QIcon(iconName));
    else
        setIconByName("dialog-error");
#else
    if (QIcon::hasThemeIcon(iconName))
        setIcon(QIcon::fromTheme(iconName));
    else if (QFile::exists(iconName))
        setIcon(QIcon(iconName));
    else
        setIcon(QIcon::fromTheme("dialog-error"));

    if (!isVisible())
        show();
#endif

    // Show notification and set KDE tooltip
    switch (model->databaseStatus()) {
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
        const QString message = QString::number(model->outdatedPackages().size()) + tr(" updates available");
        showNotification(message);
#ifdef KDE
        setToolTipSubTitle(message + '\n' + lastSyncString(settings.lastSync()));
        setStatus(KStatusNotifierItem::NeedsAttention);
#endif
    }
    }
}

#ifndef KDE
void SystemTray::processTrayActivation(QSystemTrayIcon::ActivationReason reason)
{
    if (reason != QSystemTrayIcon::Trigger)
        return;

    auto window = qobject_cast<MainWindow *>(parent());
    if (!window->isVisible()) {
        window->show();
        window->activateWindow();
        window->raise();
    } else {
        window->hide();
    }
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
