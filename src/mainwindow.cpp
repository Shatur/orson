#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Select package when clicking on dependencies
    depsButtonGroup = new QButtonGroup(this);
    connect(depsButtonGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this, &MainWindow::findDepend);

    // Select first package
    ui->packagesView->setCurrentIndex(ui->packagesView->model()->index(0, 0));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_searchEdit_returnPressed()
{
    const auto filterType = static_cast<PackagesView::FilterType>(ui->searchByComboBox->currentIndex());
    ui->packagesView->filter(ui->searchEdit->text(), filterType);
}

void MainWindow::on_packagesView_currentPackageChanged(Package *package)
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
    if (package->isInstalled())
        ui->packageTabsWidget->setTabEnabled(2, true);
    else
        ui->packageTabsWidget->setTabEnabled(2, false);

    // Reload opened tab
    switch (ui->packageTabsWidget->currentIndex()) {
    case 0:
        loadPackageInfo(package);
        return;
    case 1:
        loadPackageDeps(package);
        return;
    case 2:
        if (package->isInstalled())
            loadPackageFiles(package);
        else
            ui->packageTabsWidget->setCurrentIndex(0);
        return;
    default:
        return;
    }
}

void MainWindow::on_packageTabsWidget_currentChanged(int index)
{
    const Package *package = ui->packagesView->currentPackage();

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

void MainWindow::on_searchModeComboBox_currentIndexChanged(int index)
{
    const auto mode = static_cast<PackagesModel::Mode>(index);
    const auto filterType = static_cast<PackagesView::FilterType>(ui->searchByComboBox->currentIndex());

    ui->packagesView->model()->setMode(mode);
    ui->packagesView->filter(ui->searchEdit->text(), filterType);
}

void MainWindow::findDepend(QAbstractButton *button)
{
    // Clear previous search
    if (!ui->searchEdit->text().isEmpty()) {
        ui->searchEdit->clear();
        ui->packagesView->filter("");
    }

    // Search package
    ui->packagesView->find(button->toolTip());
}

void MainWindow::loadPackageInfo(const Package *package)
{
    // General info
    ui->archLabel->setText(package->arch());
    ui->repoLabel->setText(package->repo());
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

    // Do not show download size for local package
    if (package->repo() == "local")
        ui->downloadSizeLabel->setText("-");
    else
        ui->downloadSizeLabel->setText(package->formattedDownloadSize());

    // Install-specific info
    if (package->isInstalled()) {
        ui->installDateLabel->setText(package->installDate().toString("ddd dd MMM yyyy HH:mm:ss"));
        ui->installedSizeLabel->setText(package->formattedInstalledSize());

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
    ui->filesView->model()->setPaths(package->files());
    ui->filesTab->setProperty("loaded", true);
}

void MainWindow::loadDepsButtons(int row, const QVector<alpm_depend_t *> &deps)
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
        depsButtonGroup->addButton(button);
        packagesLayout->addWidget(button);
    }
    packagesLabel->show();
}
