#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "packagemanager.h"

#include <QMainWindow>

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

private:
    PackageManager alpm;

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
