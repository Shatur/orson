#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "packages-view/depend.h"
#include "packages-view/packagesmodel.h"

#include <QMainWindow>

class QLabel;
class QButtonGroup;
class QAbstractButton;
class QActionGroup;
class Pacman;
class Package;
class AutosyncTimer;
class SystemTray;
class QShortcut;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // Menu bar
    void on_installLocalAction_triggered();
    void on_installLocalDependAction_triggered();
    void on_exitAction_triggered();
    void on_instantSearchAction_toggled(bool checked);
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
    void on_searchPackagesEdit_textSearched();
    void on_packagesView_currentPackageChanged(Package *package);
    void on_packageTabsWidget_currentChanged(int index);

    void setStatusBarMessage(const QString &text);
    void changeSearchMode();
    void findDepend(QAbstractButton* button);
    void showAppRunningMessage();

    void processDatabaseStatusChanged(PackagesModel::DatabaseStatus status);
    void processFirstPackageAvailable();
    void processOperationsCountChanged(int tasksCount);
    void processTerminalStart();
    void processTerminalFinish(int exitCode);

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

    Ui::MainWindow *ui;
    QButtonGroup *m_depsButtonGroup;
    QMenu *m_trayMenu;
    QActionGroup *m_afterCompletionGroup;
    QShortcut *m_changeModeShortcut;

    Pacman *m_pacman;
    AutosyncTimer *m_autosyncTimer;
    SystemTray *m_trayIcon;
};

#endif // MAINWINDOW_H
