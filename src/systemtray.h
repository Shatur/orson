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
    enum TrayStatus {
        NoUpdates,
        Updating,
        UpdatesAvailable
    };

    SystemTray(MainWindow *parent);

    void showNotification(const QString &message, int interval = 10000);
    void setTrayStatus(TrayStatus trayStatus, int updatesCount = 0);
    TrayStatus trayStatus() const;

#ifndef KDE
private slots:
    void processTrayActivation(QSystemTrayIcon::ActivationReason reason);
#endif

private:
    TrayStatus m_trayStatus = Updating;
};

#endif // SYSTEMTRAY_H
