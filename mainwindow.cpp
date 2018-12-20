#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "alpm.h"
#include "alpm_list.h"

#include <QDebug>
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->packagesTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    foreach (const Package &package, alpm.packages()) {
        auto item = new QTreeWidgetItem;
        item->setText(1, package.name());
        item->setText(2, package.version());
        item->setText(3, QString::number(package.size()));
        item->setText(4, package.repo());
        if (package.installed())
            item->setText(0, tr("Installed"));

        ui->packagesTreeWidget->addTopLevelItem(item);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_searchEdit_textEdited(const QString &text)
{
    QTreeWidgetItemIterator it(ui->packagesTreeWidget);
    while (*it) {
        QTreeWidgetItem *item = *it;
        if (!item->text(1).contains(text))
            item->setHidden(true);
        else
            item->setHidden(false);
        ++it;
    }
}
