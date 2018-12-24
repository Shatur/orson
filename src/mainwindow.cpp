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

    // Load packages list
    for (int i = 0; i < packageManager.packages().size(); ++i) {
        const Package *package = &packageManager.packages().at(i);
        auto item = new QTreeWidgetItem;
        item->setData(0, Qt::UserRole, i);
        item->setText(1, package->name());
        item->setText(2, package->version());
        item->setText(3, QString::number(package->size()));
        item->setText(4, package->repo());
        if (package->isInstalled())
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
void MainWindow::on_packagesTreeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous)
    const int index = current->data(0, Qt::UserRole).toInt();
    const Package *package = &packageManager.packages().at(index);

    // Icon
    QIcon icon;
    if (QIcon::hasThemeIcon(package->name()))
        icon = QIcon::fromTheme(package->name());
    else
        icon = QIcon::fromTheme("package-x-generic");
    ui->iconLabel->setPixmap(icon.pixmap(64, 64));

    // General info
    ui->nameLabel->setText(package->name() + " " + package->version());
    ui->descriptionLabel->setText(package->description());
    ui->archLabel->setText(package->arch());
    ui->urlLabel->setText("<a href=\"" + package->url() + "\">" + package->url() + "</a>");
    ui->sizeLabel->setText(QString::number(package->size()));
    ui->packagerLabel->setText(package->packager());
    ui->buildDateLabel->setText(package->buildDate().toString("ddd dd MMM yyyy HH:mm:ss"));

    // Licenses
    QString licenses = package->licenses().at(0);
    for (int i = 1; i < package->licenses().size(); ++i)
        licenses.append(", " + package->licenses().at(i));
    ui->licensesLabel->setText(licenses);

    // Groups
    if (!package->groups().isEmpty()) {
        QString groups = package->groups().at(0);
        for (int i = 1; i < package->groups().size(); ++i)
            groups.append(", " + package->groups().at(i));
        ui->groupslabel->setText(groups);
    } else {
        ui->groupslabel->setText(tr("No"));
    }

    // Install-specific info
    if (package->isInstalled()) {
        // Build date
        ui->installDateLabel->setText(package->installDate().toString("ddd dd MMM yyyy HH:mm:ss"));

        // Reason
        if (package->reason() == ALPM_PKG_REASON_EXPLICIT)
            ui->reasonLabel->setText("Installed explicitly");
        else
            ui->reasonLabel->setText("Installed as dependency");

        // Install script
        if (package->hasScript())
            ui->scriptLabel->setText(tr("Yes"));
        else
            ui->scriptLabel->setText(tr("No"));

        // Files
        ui->filesTreeWidget->clear();
        foreach (const QString &path, package->files())
            ui->filesTreeWidget->addPath(path);
        ui->filesTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    } else {
        ui->installDateLabel->setText("-");
        ui->reasonLabel->setText("-");
        ui->scriptLabel->setText("-");
    }
}
