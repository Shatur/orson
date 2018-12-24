#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "packagemanager.h"

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QFileSystemModel>

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

private:
    Ui::MainWindow *ui;
    PackageManager packageManager;
};

#endif // MAINWINDOW_H
