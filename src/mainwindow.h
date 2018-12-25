#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "packagemanager.h"

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
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
    void on_searchEdit_textEdited(const QString &text);
    void on_packagesTreeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_packageTabsWidget_currentChanged(int index);

private:
    // Package info tabs
    void loadPackageInfo(const Package *package);
    void loadPackageDeps(const Package *package);
    void loadPackageFiles(const Package *package);

    void loadDepsButtons(int row, const QList<alpm_depend_t*> &items);

    Ui::MainWindow *ui;
    PackageManager packageManager;
};

#endif // MAINWINDOW_H
