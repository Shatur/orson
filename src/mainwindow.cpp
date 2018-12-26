#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "alpm.h"
#include "alpm_list.h"

#include <QDebug>
#include <QPushButton>

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

    // Select first item
    ui->packagesTreeWidget->setCurrentItem(ui->packagesTreeWidget->topLevelItem(0));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_searchEdit_textChanged(const QString &text)
{
    constexpr int NAME_DESCRIPTION_INDEX = 0;
    constexpr int NAME_INDEX = 1;
    constexpr int DESCRIPTION_INDEX = 2;

    QTreeWidgetItemIterator it(ui->packagesTreeWidget);
    switch (ui->searchComboBox->currentIndex()) {
    case NAME_DESCRIPTION_INDEX:
        while (*it) {
            QTreeWidgetItem *item = *it;
            const int packageIndex = item->data(0, Qt::UserRole).toInt();
            if (item->text(1).contains(text) || packageManager.packages().at(packageIndex).description().contains(text))
                item->setHidden(false);
            else
                item->setHidden(true);
            ++it;
        }
        break;
    case NAME_INDEX:
        while (*it) {
            QTreeWidgetItem *item = *it;
            if (item->text(1).contains(text))
                item->setHidden(false);
            else
                item->setHidden(true);
            ++it;
        }
        break;
    case DESCRIPTION_INDEX:
        while (*it) {
            QTreeWidgetItem *item = *it;
            const int packageIndex = item->data(0, Qt::UserRole).toInt();
            if (packageManager.packages().at(packageIndex).description().contains(text))
                item->setHidden(false);
            else
                item->setHidden(true);
            ++it;
        }
    }
}
void MainWindow::on_packagesTreeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous)
    const int packageIndex = current->data(0, Qt::UserRole).toInt();
    const Package *package = &packageManager.packages().at(packageIndex);

    // Reset loaded tabs information
    ui->infoTab->setProperty("loaded", false);
    ui->depsTab->setProperty("loaded", false);
    ui->filesTab->setProperty("loaded", false);

    // Load package info header
    QIcon icon;
    if (QIcon::hasThemeIcon(package->name()))
        icon = QIcon::fromTheme(package->name());
    else
        icon = QIcon::fromTheme("package-x-generic");
    ui->iconLabel->setPixmap(icon.pixmap(64, 64));
    ui->nameLabel->setText(package->name() + " " + package->version());
    ui->descriptionLabel->setText(package->description());

    // Load only opened tab
    switch (ui->packageTabsWidget->currentIndex()) {
    case 0:
        loadPackageInfo(package);
        return;
    case 1:
        loadPackageDeps(package);
        return;
    case 2:
        loadPackageFiles(package);
        return;
    default:
        return;
    }
}

void MainWindow::on_packageTabsWidget_currentChanged(int index)
{
    const int packageIndex = ui->packagesTreeWidget->currentItem()->data(0, Qt::UserRole).toInt();
    const Package *package = &packageManager.packages().at(packageIndex);

    switch (index) {
    case 0:
        if (!ui->infoTab->property("loaded").toBool())
            loadPackageInfo(package);
        return;
    case 1:
        if (!ui->depsTab->property("loaded").toBool())
            loadPackageDeps(package);
        return;
    case 2:
        if (!ui->filesTab->property("loaded").toBool())
            loadPackageFiles(package);
        return;
    default:
        return;
    }
}

void MainWindow::selectPackage()
{
    auto button = qobject_cast<QPushButton*>(sender());
    ui->packagesTreeWidget->clearSelection();
    ui->searchEdit->clear();

    // Search by text
    QTreeWidgetItemIterator it(ui->packagesTreeWidget);
    while (*it) {
        QTreeWidgetItem *item = *it;
        if (item->text(1) == button->toolTip()) {
            ui->packagesTreeWidget->setCurrentItem(item);
            ui->packagesTreeWidget->scrollToItem(item);
            return;
        }
        ++it;
    }

    // Search by providing
    it = QTreeWidgetItemIterator(ui->packagesTreeWidget);
    while (*it) {
        QTreeWidgetItem *item = *it;
        int packageIndex = item->data(0, Qt::UserRole).toInt();
        foreach (const alpm_depend_t *package, packageManager.packages().at(packageIndex).provides()) {
            if (package->name == button->toolTip()) {
                ui->packagesTreeWidget->setCurrentItem(item);
                ui->packagesTreeWidget->scrollToItem(item);
                return;
            }
        }
        ++it;
    }
}

void MainWindow::loadPackageInfo(const Package *package)
{
    // General info
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
    } else {
        ui->installDateLabel->setText("-");
        ui->reasonLabel->setText("-");
        ui->scriptLabel->setText("-");
    }

    ui->infoTab->setProperty("loaded", true);
}

void MainWindow::loadPackageDeps(const Package *package)
{
    loadDepsButtons(0, package->provides());
    loadDepsButtons(1, package->replaces());
    loadDepsButtons(2, package->conflicts());
    loadDepsButtons(3, package->depends());
    loadDepsButtons(4, package->optdepends());
    loadDepsButtons(5, package->checkdepends());
    loadDepsButtons(6, package->makedepends());

    ui->depsTab->setProperty("loaded", true);
}

void MainWindow::loadPackageFiles(const Package *package)
{
    if (!package->isInstalled())
        return;

    ui->filesTreeWidget->clear();
    foreach (const QString &path, package->files())
        ui->filesTreeWidget->addPath(path);
    ui->filesTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->filesTab->setProperty("loaded", true);
}

void MainWindow::loadDepsButtons(int row, const QList<alpm_depend_t *> &items)
{
    auto depsContentLayout = dynamic_cast<QFormLayout*>(ui->depsContentWidget->layout());
    auto packagesLabel = dynamic_cast<QLabel*>(depsContentLayout->itemAt(row, QFormLayout::LabelRole)->widget());
    auto packagesLayout = dynamic_cast<QVBoxLayout*>(depsContentLayout->itemAt(row, QFormLayout::FieldRole)->layout());

    // Remove old items
    while (QLayoutItem *child = packagesLayout->takeAt(0)) {
        delete child->widget();
        delete child;
    }

    // Hide label if no dependencies
    if (items.isEmpty()) {
        packagesLabel->hide();
        return;
    }

    // Add new
    foreach (alpm_depend_t *item, items) {
        auto button = new QPushButton;
        button->setFlat(true);
        button->setStyleSheet("padding: 6px");
        button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        button->setToolTip(item->name);
        if (item->desc == nullptr)
            button->setText(item->name + Package::depmodString(item->mod) + item->version);
        else
            button->setText(item->name + Package::depmodString(item->mod) + item->version + QString(": ") + item->desc);
        connect(button, &QPushButton::clicked, this, &MainWindow::selectPackage);
        packagesLayout->addWidget(button);
    }
    packagesLabel->show();
}
