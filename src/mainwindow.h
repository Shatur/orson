#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "files-view/filesmodel.h"
#include "packages-view/packagesmodel.h"
#include "terminal.h"

#include <QMainWindow>
#include <QButtonGroup>
#include <QLabel>

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
    void on_tabWidget_currentChanged(int index);
    void on_browserButton_clicked();

    void setStatusBarMessage(const QString &text);

    // Packages tab
    void on_searchModeComboBox_currentIndexChanged(int index);
    void on_searchPackagesEdit_returnPressed();
    void on_packagesView_currentPackageChanged(Package *package);
    void on_packageTabsWidget_currentChanged(int index);
    void on_updateButton_clicked();
    void on_reloadButton_clicked();

    void findDepend(QAbstractButton* button);
    void selectFirstPackage();
    void enableReloading();
    void showPackagesTab();

    // History tab
    void on_reloadHistoryButton_clicked();
    void on_findNextButton_clicked();
    void on_findPreviousButton_clicked();
    void on_openHistoryFolderButton_clicked();

    // Tasks tab
    void on_applyButton_clicked();
    void on_noConfirmCheckBox_toggled(bool checked);

    void addTasks();

private:
    // Package info tabs
    void loadPackageInfo(const Package *package);
    void loadPackageDeps(const Package *package);
    void loadPackageFiles(const Package *package);

    // Helper functions
    void displayInfo(bool display, const QString &text, QLabel *titleLabel, QLabel *label);
    void loadDepsButtons(int row, const QVector<Depend> &deps);
    void searchHistory(bool backward = false);

    Ui::MainWindow *ui;
    QButtonGroup *depsButtonGroup;
    Terminal m_terminal;

};

#endif // MAINWINDOW_H
