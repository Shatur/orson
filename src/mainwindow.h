#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "files-view/filesmodel.h"
#include "packages-view/packagesmodel.h"
#include "terminal.h"

#include <QMainWindow>
#include <QButtonGroup>
#include <QLabel>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Menu bar
    void on_installLocalAction_triggered();
    void on_installLocalDependAction_triggered();
    void on_openHistoryFileAction_triggered();
    void on_openHistoryFolderAction_triggered();

    // General
    void activateTray(QSystemTrayIcon::ActivationReason reason);
    void setStatusBarMessage(const QString &text);
    void processLoadedDatabase();
    void processTerminalStart();

    // Packages tab
    void on_searchModeComboBox_currentIndexChanged(int index);
    void on_searchPackagesEdit_returnPressed();
    void on_packagesView_currentPackageChanged(Package *package);
    void on_packageTabsWidget_currentChanged(int index);
    void on_browserButton_clicked();
    void on_updateButton_clicked();
    void on_reloadButton_clicked();

    void findDepend(QAbstractButton* button);
    void selectFirstPackage();
    void showPackagesTab();

    // Tasks tab
    void on_applyButton_clicked();
    void on_noConfirmCheckBox_toggled(bool checked);
    void on_afterCompletionComboBox_currentIndexChanged(int index);

    void processAddingTask();
private:
    // Package info tabs
    void loadPackageInfo(const Package *package);
    void loadPackageDeps(const Package *package);
    void loadPackageFiles(const Package *package);

    // Helper functions
    void displayInfo(bool display, const QString &text, QLabel *titleLabel, QLabel *label);
    void loadDepsButtons(int row, const QVector<Depend> &deps);

    Ui::MainWindow *ui;
    QButtonGroup *depsButtonGroup;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    Terminal m_terminal;
};

#endif // MAINWINDOW_H
