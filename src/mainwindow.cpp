#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Select package when clicking on dependencies
    connect(&depsButtonGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this, &MainWindow::selectPackage);

    // Select first package
    ui->packagesTreeView->setCurrentPackage(ui->packagesTreeView->model()->packages().at(0));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_searchEdit_returnPressed()
{
    const QString searchText = ui->searchEdit->text();
    PackagesModel *model = ui->packagesTreeView->model();

    switch (ui->searchComboBox->currentIndex()) {
    case 0:
        // Search by name and description
        foreach (Package *package, model->packages()) {
            if (package->name().contains(searchText) || package->description().contains(searchText))
                ui->packagesTreeView->setPackageHidden(package, false);
            else
                ui->packagesTreeView->setPackageHidden(package, true);
        }
        break;
    case 1:
        // Search only by name
        foreach (Package *package, model->packages()) {
            if (package->name().contains(searchText))
                ui->packagesTreeView->setPackageHidden(package, false);
            else
                ui->packagesTreeView->setPackageHidden(package, true);
        }
        break;
    case 2:
        // Search only by description
        foreach (Package *package, model->packages()) {
            if (package->description().contains(searchText))
                ui->packagesTreeView->setPackageHidden(package, false);
            else
                ui->packagesTreeView->setPackageHidden(package, true);
        }
    }
}

void MainWindow::on_packagesTreeView_currentPackageChanged(Package *package)
{
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

    // Disable the tab with files for uninstalled packages
    if (!package->isInstalled()) {
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
    const Package *package = ui->packagesTreeView->currentPackage();

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
    ui->packagesTreeView->clearSelection();
    ui->searchEdit->clear();
    PackagesModel *model = ui->packagesTreeView->model();

    // Search by text
    foreach (Package *package, model->packages()) {
        if (package->name() == button->toolTip()) {
            ui->packagesTreeView->setCurrentPackage(package);
            ui->packagesTreeView->scrollToPackage(package);
            return;
        }
    }

    // Search by providing
    foreach (Package *package, model->packages()) {
        foreach (const alpm_depend_t *dependency, package->provides()) {
            if (dependency->name == button->toolTip()) {
                ui->packagesTreeView->setCurrentPackage(package);
                ui->packagesTreeView->scrollToPackage(package);
                return;
            }
        }
    }
}

void MainWindow::loadPackageInfo(const Package *package)
{
    // General info
    ui->archLabel->setText(package->arch());
    ui->urlLabel->setText("<a href=\"" + package->url() + "\">" + package->url() + "</a>");
    ui->packagerLabel->setText(package->packager());
    ui->buildDateLabel->setText(package->buildDate().toString("ddd dd MMM yyyy HH:mm:ss"));
    ui->licensesLabel->setText(package->licenses().join(", "));

    // Groups
    const QStringList groups = package->groups();
    if (!groups.isEmpty())
        ui->groupslabel->setText(groups.join(", "));
    else
        ui->groupslabel->setText(tr("No"));

    // Install-specific info
    if (package->isInstalled()) {
        ui->installDateLabel->setText(package->installDate().toString("ddd dd MMM yyyy HH:mm:ss"));
        ui->installedSizeLabel->setText(QString::number(package->installedSize()));

        // Reason
        if (package->reason() == ALPM_PKG_REASON_EXPLICIT)
            ui->reasonLabel->setText(tr("Installed explicitly"));
        else
            ui->reasonLabel->setText(tr("Installed as dependency"));

        // Install script
        if (package->hasScript())
            ui->scriptLabel->setText(tr("Yes"));
        else
            ui->scriptLabel->setText(tr("No"));

        // Do not show download size for local package
        if (package->repo() == "local")
            ui->downloadSizeLabel->setText("-");
        else
            ui->downloadSizeLabel->setText(QString::number(package->downloadSize()));
    } else {
        ui->installDateLabel->setText("-");
        ui->installedSizeLabel->setText("-");
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

    ui->depsTab->setProperty("loaded", true);
}

void MainWindow::loadPackageFiles(const Package *package)
{
    if (!package->isInstalled())
        return;

    ui->filesTreeView->model()->setPaths(package->files());
    ui->filesTab->setProperty("loaded", true);
}

void MainWindow::loadDepsButtons(int row, const QList<alpm_depend_t *> &deps)
{
    auto depsContentLayout = qobject_cast<QFormLayout *>(ui->depsContentWidget->layout());
    auto packagesLabel = qobject_cast<QLabel *>(depsContentLayout->itemAt(row, QFormLayout::LabelRole)->widget());
    auto packagesLayout = qobject_cast<QVBoxLayout *>(depsContentLayout->itemAt(row, QFormLayout::FieldRole)->layout());

    // Remove old items
    while (QLayoutItem *child = packagesLayout->takeAt(0)) {
        delete child->widget();
        delete child;
    }

    // Hide label if no dependencies
    if (deps.isEmpty()) {
        packagesLabel->hide();
        return;
    }

    // Add new
    foreach (alpm_depend_t *dep, deps) {
        auto button = new QPushButton;
        button->setFlat(true);
        button->setStyleSheet("padding: 6px");
        button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        button->setToolTip(dep->name);
        if (dep->desc == nullptr)
            button->setText(dep->name + Package::depmodString(dep->mod) + dep->version);
        else
            button->setText(dep->name + Package::depmodString(dep->mod) + dep->version + QString(": ") + dep->desc);
        depsButtonGroup.addButton(button);
        packagesLayout->addWidget(button);
    }
    packagesLabel->show();
}
