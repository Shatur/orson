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

void MainWindow::on_searchModeComboBox_currentIndexChanged(int index)
{
    const auto mode = static_cast<PackagesModel::Mode>(index);

    ui->packagesView->model()->setMode(mode);
    on_searchEdit_returnPressed(); // Search packages
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

void MainWindow::findDepend(QAbstractButton *button)
{
    // Search in repo first
    ui->searchEdit->clear();
    ui->searchModeComboBox->setCurrentIndex(PackagesModel::Repo);

    // Search package
    const bool found = ui->packagesView->find(button->toolTip());
    if (!found) {
        // Search in AUR
        ui->searchEdit->setText(button->toolTip());
        ui->searchModeComboBox->setCurrentIndex(PackagesModel::AUR);
    }
}

void MainWindow::loadPackageInfo(const Package *package)
{
    // Licenses
    const QStringList licenses = package->licenses();
    displayInfo(!licenses.isEmpty(), licenses.join(", "), ui->licensesTitleLabel, ui->licensesLabel);

    // URL
    const QString url = package->url();
    displayInfo(!url.isEmpty(), QString("<a href=\"" + url + "\">" + url + "</a>"), ui->urlTitleLabel, ui->urlLabel);

    // Keywords (AUR)
    const QStringList keywords = package->keywords();
    displayInfo(!keywords.isEmpty(), keywords.join(", "), ui->keywordsTitleLabel, ui->keywordsLabel);

    // Arch
    const QString arch = package->arch();
    displayInfo(!arch.isEmpty(), arch, ui->archTitleLabel, ui->archLabel);

    // Groups
    const QStringList groups = package->groups();
    displayInfo(!groups.isEmpty(), groups.join(", "), ui->groupsTitlelabel, ui->groupsLabel);

    // Sizes
    displayInfo(package->downloadSize() != -1, package->formattedDownloadSize(), ui->downloadSizeTitleLabel, ui->downloadSizeLabel);
    displayInfo(package->installedSize() != -1, package->formattedInstalledSize(), ui->installedSizeTitleLabel, ui->installedSizeLabel);

    // Build date
    const QDateTime buildDate = package->buildDate();
    displayInfo(buildDate.isValid(), buildDate.toString("ddd dd MMM yyyy HH:mm:ss"), ui->buildDateTitleLabel, ui->buildDateLabel);

    // Install date
    const QDateTime installDate = package->installDate();
    displayInfo(installDate.isValid(), installDate.toString("ddd dd MMM yyyy HH:mm:ss"), ui->installDateTitleLabel, ui->installDateLabel);

    // First submitted date (AUR)
    const QDateTime firstSubmitted = package->firstSubmitted();
    displayInfo(firstSubmitted.isValid(), firstSubmitted.toString("ddd dd MMM yyyy HH:mm:ss"), ui->firstSubmittedTitleLabel, ui->firstSubmittedLabel);

    // Last submitted date (AUR)
    const QDateTime lastSubmitted = package->lastModified();
    displayInfo(lastSubmitted.isValid(), lastSubmitted.toString("ddd dd MMM yyyy HH:mm:ss"), ui->lastModifiedTitleLabel, ui->lastModifiedLabel);

    // Out of date (AUR)
    const QDateTime outOfDate = package->outOfDate();
    displayInfo(outOfDate.isValid(), outOfDate.toString("ddd dd MMM yyyy HH:mm:ss"), ui->outOfDateTitleLabel, ui->outOfDateLabel);

    // Maintainer
    const QString maintainer = package->maintainer();
    if (!maintainer.isEmpty())
        ui->maintainerLabel->setText(package->maintainer());
    else
        ui->maintainerLabel->setText("None");

    // Other install-specific info
    if (package->isInstalled()) {
        // Reason
        ui->reasonLabel->setVisible(true);
        ui->reasonTitleLabel->setVisible(true);
        if (package->reason() == ALPM_PKG_REASON_EXPLICIT)
            ui->reasonLabel->setText(tr("Installed explicitly"));
        else
            ui->reasonLabel->setText(tr("Installed as dependency"));

        // Install script
        ui->scriptLabel->setVisible(true);
        ui->scriptTitleLabel->setVisible(true);
        if (package->hasScript())
            ui->scriptLabel->setText(tr("Yes"));
        else
            ui->scriptLabel->setText(tr("No"));
    } else {
        ui->reasonLabel->setVisible(false);
        ui->reasonTitleLabel->setVisible(false);
        ui->scriptLabel->setVisible(false);
        ui->scriptTitleLabel->setVisible(false);
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

void MainWindow::displayInfo(bool display, const QString &text, QLabel *titleLabel, QLabel *label)
{
    if (display) {
        // Spacing in layout must be disabled to hide elements without empty space.
        // So need to add spacing between items dynamically
        label->setMargin(6);
        titleLabel->setMargin(6);

        label->setText(text);
        label->setVisible(true);
        titleLabel->setVisible(true);
    } else {
        label->setMargin(0);
        titleLabel->setMargin(0);

        label->setVisible(false);
        titleLabel->setVisible(false);
    }
}

void MainWindow::loadDepsButtons(int row, const QVector<Depend> &deps)
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
    foreach (const Depend &depend, deps) {
        auto button = new QPushButton;
        button->setFlat(true);
        button->setStyleSheet("padding: 6px");
        button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        button->setToolTip(depend.name());
        if (depend.description().isEmpty())
            button->setText(depend.name() + depend.mod() + depend.version());
        else
            button->setText(depend.name() + depend.mod() + depend.version() + ": " + depend.description());
        depsButtonGroup->addButton(button);
        packagesLayout->addWidget(button);
    }
    packagesLabel->show();
}
