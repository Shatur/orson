#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include "mainwindow.h"

#ifdef PLASMA
#include <KF5/KNotifications/KStatusNotifierItem>
#else
#include <QSystemTrayIcon>
#endif

#ifdef PLASMA
class SystemTray : public KStatusNotifierItem
#else
class SystemTray : public QSystemTrayIcon
#endif
{
public:
    SystemTray(MainWindow *parent);

    MainWindow *parent() const;
    void showNotification(const QString &message, int interval = 10'000);
    void showMainWindow();
    void setTrayStatus(PackagesModel::DatabaseStatus status, int updatesCount = 0);

    static QIcon trayIcon(const QString &iconName);
    static QString trayIconName(const QString &iconName);

#ifndef PLASMA
private slots:
    void processTrayActivation(QSystemTrayIcon::ActivationReason reason);
#endif

private:
    static QString lastSyncString(const QDateTime& dateTime);
};

#endif // SYSTEMTRAY_H
