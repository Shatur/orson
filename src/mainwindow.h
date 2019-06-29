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
    Q_DISABLE_COPY(MainWindow)

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // Menu bar
    void installLocalPackage();
    void installLocalPackageAsDepend();
    void exit();
    void setInstantSearch(bool enabled);
    void setNoConfirm(bool enabled);
    void setForce(bool enabled);
    void openHistoryFile();
    void openHistoryFileFolder();
    void openSettings();
    void setAfterTasksCompletionAction(QAction *action);

    // Tray context menu
    void syncAndUpgrade();
    void syncRepositories();
    void upgrade();

    // Buttons
    void applyTasks();
    void setSyncDatabase(bool sync);
    void setUpgradeDatabase(bool upgrade);
    void reloadDatabase();
    void openInBrowser();

    // General
    void setSearchMode(int mode);
    void searchPackages(const QString &text);
    void displayPackage(Package *package);
    void setPackageTab(int index);
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
    void closeEvent(QCloseEvent *event) override;

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
    QShortcut *m_searchPackagesShortcut;

    Pacman *m_pacman;
    AutosyncTimer *m_autosyncTimer;
    SystemTray *m_trayIcon;
};

#endif // MAINWINDOW_H
