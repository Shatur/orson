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
    void loadTrayStatus(const PackagesModel *model);

#ifndef KDE
private slots:
    void processTrayActivation(QSystemTrayIcon::ActivationReason reason);
#endif

private:
    static QString lastSyncString(QDateTime dateTime);
};

#endif // SYSTEMTRAY_H
