#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "packages-view/depend.h"

#include <QMainWindow>
#ifdef KDE
#include <KF5/KNotifications/KStatusNotifierItem>
#else
#include <QSystemTrayIcon>
#endif

class QLabel;
class QButtonGroup;
class QAbstractButton;
class QActionGroup;
class Pacman;
class Package;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum AutosyncType {
        Interval,
        SpecifiedTime
    };
    Q_ENUM(AutosyncType)
    enum TrayStatus {
        NoUpdates,
        Updating,
        UpdatesAvailable
    };

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // Menu bar
    void on_installLocalAction_triggered();
    void on_installLocalDependAction_triggered();
    void on_exitAction_triggered();
    void on_noConfirmAction_toggled(bool checked);
    void on_forceAction_toggled(bool checked);
    void on_openHistoryFileAction_triggered();
    void on_openHistoryFolderAction_triggered();
    void on_settingsAction_triggered();

    void setAfterTasksCompletionAction(QAction *action);

    // Tray context menu
    void syncAndUpgrade();
    void syncRepositories();
    void upgrade();

    // Buttons
    void on_applyButton_clicked();
    void on_syncButton_toggled(bool checked);
    void on_upgradeButton_toggled(bool checked);
    void on_reloadButton_clicked();
    void on_browserButton_clicked();

    // General
    void on_searchModeComboBox_currentIndexChanged(int index);
    void on_searchPackagesEdit_returnPressed();
    void on_packagesView_currentPackageChanged(Package *package);
    void on_packageTabsWidget_currentChanged(int index);

#ifndef KDE
    void activateTray(QSystemTrayIcon::ActivationReason reason);
#endif
    void showNotification(const QString &message, int interval);
    void setTrayStatus(TrayStatus trayStatus);
    void setStatusBarMessage(const QString &text);
    void findDepend(QAbstractButton* button);

    void processLoadedDatabase();
    void processFirstPackageAvailable();
    void processOperationsCountChanged(int tasksCount);
    void processTerminalStart();
    void processTerminalFinish(int exitCode);
    void processAutosyncTimerExpires();

private:
    virtual void closeEvent(QCloseEvent *event) override;

    // Package info tabs
    void loadPackageInfo(const Package *package);
    void loadPackageDeps(const Package *package);
    void loadPackageFiles(const Package *package);

    // Helper functions
    void displayInfo(bool display, const QString &text, QLabel *titleLabel, QLabel *label);
    void loadDepsButtons(int row, const QVector<Depend> &deps);

    void loadAppSettings();
    void loadMainWindowSettings();

    int msecsToAutosync(AutosyncType type);

    Ui::MainWindow *ui;
    QButtonGroup *m_depsButtonGroup;
    QMenu *m_trayMenu;
    QActionGroup *m_afterCompletionGroup;
    Pacman *m_pacman;
    TrayStatus m_trayStatus = Updating;
    QTimer *m_autosyncTimer;

#ifdef KDE
    KStatusNotifierItem *m_trayIcon;
#else
    QSystemTrayIcon *m_trayIcon;
#endif
};

#endif // MAINWINDOW_H
