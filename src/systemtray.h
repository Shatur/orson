#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include "mainwindow.h"

#ifdef KDE
#include <KF5/KNotifications/KStatusNotifierItem>
#else
#include <QSystemTrayIcon>
#endif

#ifdef KDE
class SystemTray : public KStatusNotifierItem
#else
class SystemTray : public QSystemTrayIcon
#endif
{
public:

    SystemTray(MainWindow *parent);

    MainWindow *parent() const;
    void showNotification(const QString &message, int interval = 10000);
    void showMainWindow();
    void setTrayStatus(PackagesModel::DatabaseStatus status, int updatesCount = 0);

    static QIcon trayIcon(const QString &iconName);
    static QString trayIconName(const QString &iconName);

#ifndef KDE
private slots:
    void processTrayActivation(QSystemTrayIcon::ActivationReason reason);
#endif

private:
    static QString lastSyncString(QDateTime dateTime);
};

#endif // SYSTEMTRAY_H
