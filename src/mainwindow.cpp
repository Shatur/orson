#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->packagesTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->setModel(&filesModel);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Select package when clicking on dependencies
    connect(&depsButtonGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this, &MainWindow::selectPackage);

    // Load packages list
    for (int i = 0; i < packageManager.packages().size(); ++i) {
        const Package &package = packageManager.package(i);
        auto item = new QTreeWidgetItem;
        item->setData(0, Qt::UserRole, i);
        item->setText(1, package.name());
        item->setText(2, package.version());
        item->setText(3, QString::number(package.downloadSize()));
        item->setText(4, package.repo());
        if (package.isInstalled())
            item->setText(0, tr("Installed"));

        ui->packagesTreeWidget->addTopLevelItem(item);
    }

    // Select first package
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
            if (item->text(1).contains(text) || packageManager.package(packageIndex).description().contains(text))
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
            if (packageManager.package(packageIndex).description().contains(text))
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
    const Package &package = packageManager.package(packageIndex);

    // Reset loaded tabs information
    ui->infoTab->setProperty("loaded", false);
    ui->depsTab->setProperty("loaded", false);
    ui->filesTab->setProperty("loaded", false);

    // Load package info header
    QIcon icon;
    if (QIcon::hasThemeIcon(package.name()))
        icon = QIcon::fromTheme(package.name());
    else
        icon = QIcon::fromTheme("package-x-generic");
    ui->iconLabel->setPixmap(icon.pixmap(64, 64));
    ui->nameLabel->setText(package.name() + " " + package.version());
    ui->descriptionLabel->setText(package.description());

    // Disable the tab with files for uninstalled packages
    if (!package.isInstalled()) {
        ui->packageTabsWidget->setTabEnabled(2, false);
        ui->packageTabsWidget->setCurrentIndex(0);
    } else {
        ui->packageTabsWidget->setTabEnabled(2, true);
    }

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
    const Package &package = packageManager.package(packageIndex);

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

void MainWindow::selectPackage(QAbstractButton *button)
{
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
        foreach (const alpm_depend_t *package, packageManager.package(packageIndex).provides()) {
            if (package->name == button->toolTip()) {
                ui->packagesTreeWidget->setCurrentItem(item);
                ui->packagesTreeWidget->scrollToItem(item);
                return;
            }
        }
        ++it;
    }
}

void MainWindow::loadPackageInfo(const Package &package)
{
    // General info
    ui->archLabel->setText(package.arch());
    ui->urlLabel->setText("<a href=\"" + package.url() + "\">" + package.url() + "</a>");
    ui->packagerLabel->setText(package.packager());
    ui->buildDateLabel->setText(package.buildDate().toString("ddd dd MMM yyyy HH:mm:ss"));
    ui->licensesLabel->setText(package.licenses().join(", "));

    // Groups
    const QStringList groups = package.groups();
    if (!groups.isEmpty())
        ui->groupslabel->setText(groups.join(", "));
    else
        ui->groupslabel->setText(tr("No"));

    // Install-specific info
    if (package.isInstalled()) {
        ui->installDateLabel->setText(package.installDate().toString("ddd dd MMM yyyy HH:mm:ss"));
        ui->installedSizeLabel->setText(QString::number(package.installedSize()));

        // Reason
        if (package.reason() == ALPM_PKG_REASON_EXPLICIT)
            ui->reasonLabel->setText(tr("Installed explicitly"));
        else
            ui->reasonLabel->setText(tr("Installed as dependency"));

        // Install script
        if (package.hasScript())
            ui->scriptLabel->setText(tr("Yes"));
        else
            ui->scriptLabel->setText(tr("No"));

        // Do not show download size for local package
        if (package.repo() == "local")
            ui->downloadSizeLabel->setText("-");
        else
            ui->downloadSizeLabel->setText(QString::number(package.downloadSize()));
    } else {
        ui->installDateLabel->setText("-");
        ui->installedSizeLabel->setText("-");
        ui->reasonLabel->setText("-");
        ui->scriptLabel->setText("-");
    }

    ui->infoTab->setProperty("loaded", true);
}

void MainWindow::loadPackageDeps(const Package &package)
{
    loadDepsButtons(0, package.provides());
    loadDepsButtons(1, package.replaces());
    loadDepsButtons(2, package.conflicts());
    loadDepsButtons(3, package.depends());
    loadDepsButtons(4, package.optdepends());

    ui->depsTab->setProperty("loaded", true);
}

void MainWindow::loadPackageFiles(const Package &package)
{
    if (!package.isInstalled())
        return;

    filesModel.setPaths(package.files());
    ui->filesTab->setProperty("loaded", true);
}

void MainWindow::loadDepsButtons(int row, const QList<alpm_depend_t *> &items)
{
    auto depsContentLayout = qobject_cast<QFormLayout*>(ui->depsContentWidget->layout());
    auto packagesLabel = qobject_cast<QLabel*>(depsContentLayout->itemAt(row, QFormLayout::LabelRole)->widget());
    auto packagesLayout = qobject_cast<QVBoxLayout*>(depsContentLayout->itemAt(row, QFormLayout::FieldRole)->layout());

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
        depsButtonGroup.addButton(button);
        packagesLayout->addWidget(button);
    }
    packagesLabel->show();
}
