#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pacmansettings.h"
#include "singleapplication.h"

#include <QPushButton>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QMessageBox>
#include <QFileDialog>
#include <QDBusInterface>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->packagesView->setTaskView(ui->tasksView);

    connect(ui->tasksView, &TasksView::taskOpened, this, &MainWindow::showPackagesTab);
    connect(ui->tasksView->model(), &TasksModel::taskAdded, this, &MainWindow::processAddingTask);
    connect(ui->tasksView->model(), &TasksModel::taskRemoved, this, &MainWindow::processAddingTask);
    connect(ui->packagesView->model(), &PackagesModel::databaseStatusChanged, this, &MainWindow::setStatusBarMessage);
    connect(ui->packagesView->model(), &PackagesModel::firstPackageAvailable, this, &MainWindow::selectFirstPackage);
    connect(ui->packagesView->model(), &PackagesModel::databaseLoaded, this, &MainWindow::processLoadedDatabase);
    connect(&m_terminal, &Terminal::finished, this, &MainWindow::on_reloadButton_clicked);
    connect(&m_terminal, &Terminal::started, this, &MainWindow::processTerminalStart);

    // Select package when clicking on dependencies
    depsButtonGroup = new QButtonGroup(this);
    connect(depsButtonGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this, &MainWindow::findDepend);

    // Select the first package if it is not already selected (when first package loaded faster than window)
    if (ui->packagesView->model()->index(0, 0).isValid() && ui->packagesView->selectionModel()->selectedRows().isEmpty())
        selectFirstPackage();

    // System tray menu
    m_trayMenu = new QMenu(this);
    m_trayMenu->addAction(QIcon::fromTheme("window"), tr("Show window"), this, &MainWindow::show);
    m_trayMenu->addAction(ui->updateButton->icon(), ui->updateButton->text(), this, &MainWindow::on_updateButton_clicked);
    m_trayMenu->addAction(ui->reloadButton->icon(), ui->reloadButton->text(), this, &MainWindow::on_reloadButton_clicked);
    m_trayMenu->addAction(QIcon::fromTheme("application-exit"), tr("Exit"), SingleApplication::instance(), &SingleApplication::quit);

    // System tray icon
    m_trayIcon = new QSystemTrayIcon(QIcon::fromTheme("state-sync"), this);
    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::activateTray);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    // Load history dynamically
    if (index == 1 && !ui->historyEdit->property("loaded").toBool()) {
        on_reloadHistoryButton_clicked();
        ui->historyEdit->setProperty("loaded", true);
    }
}

void MainWindow::on_installLocalAction_triggered()
{
    QFileDialog dialog(this, tr("Select package"));
    dialog.setNameFilter(tr("Pacman package (*.pkg.tar.xz);;All files(*)"));
    dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (!dialog.exec())
        return;

    m_terminal.installPackage(dialog.selectedFiles().at(0));
}

void MainWindow::on_installLocalDependAction_triggered()
{
    QFileDialog dialog(this, tr("Select package"));
    dialog.setNameFilter(tr("Pacman package (*.pkg.tar.xz);;All files(*)"));
    dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (!dialog.exec())
        return;

    m_terminal.installPackage(dialog.selectedFiles().at(0), true);
}

void MainWindow::activateTray(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        if (!this->isVisible()) {
            show();
            activateWindow();
            raise();
        } else {
            hide();
        }
    }
}

void MainWindow::setStatusBarMessage(const QString &text)
{
    statusBar()->showMessage(text);
}

void MainWindow::processLoadedDatabase()
{
    QDBusInterface notify("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QVariantList notifyArguments;
    notifyArguments << "Orson"; // Program name
    notifyArguments << QVariant(QVariant::UInt);
    notifyArguments << windowIcon().name(); // Icon
    notifyArguments << "Updates"; // Title

    // Body
    if (ui->packagesView->model()->outdatedPackages().isEmpty()) {
        notifyArguments << "No updates available";
        m_trayIcon->setIcon(QIcon::fromTheme("update-none"));
    } else {
        notifyArguments << QString::number(ui->packagesView->model()->outdatedPackages().size()) + " updates available";
        m_trayIcon->setIcon(QIcon::fromTheme("update-high"));
    }

    notifyArguments << QStringList();
    notifyArguments << QVariantMap();
    notifyArguments << 4000; // Show interval
    notify.callWithArgumentList(QDBus::AutoDetect, "Notify", notifyArguments);

    ui->reloadButton->setEnabled(true);
}

void MainWindow::processTerminalStart()
{
    m_trayIcon->setIcon(QIcon::fromTheme("state-sync"));
    ui->reloadButton->setEnabled(false);
}

void MainWindow::on_searchModeComboBox_currentIndexChanged(int index)
{
    const auto mode = static_cast<PackagesModel::Mode>(index);

    // Disable search by description for AUR
    if (mode == PackagesModel::AUR) {
        qobject_cast<QStandardItemModel *>(ui->searchByComboBox->model())->item(3)->setEnabled(false);
        if (ui->searchByComboBox->currentIndex() == 3)
            ui->searchByComboBox->setCurrentIndex(0);
    } else {
        qobject_cast<QStandardItemModel *>(ui->searchByComboBox->model())->item(3)->setEnabled(true);
    }

    ui->packagesView->model()->setMode(mode);
    on_searchPackagesEdit_returnPressed(); // Search packages
}

void MainWindow::on_searchPackagesEdit_returnPressed()
{
    const auto filterType = static_cast<PackagesView::FilterType>(ui->searchByComboBox->currentIndex());
    ui->packagesView->filter(ui->searchPackagesEdit->text(), filterType);
}

void MainWindow::on_packagesView_currentPackageChanged(Package *package)
{
    // Reset loaded tabs information
    ui->infoTab->setProperty("loaded", false);
    ui->depsTab->setProperty("loaded", false);
    ui->filesTab->setProperty("loaded", false);

    // Load package info header
    ui->iconLabel->setPixmap(package->icon().pixmap(64, 64));
    ui->nameLabel->setText(package->name());
    ui->descriptionLabel->setText(package->description());

    // Show available update in header
    const QString availableUpdate = package->availableUpdate();
    if (availableUpdate.isEmpty())
        ui->versionLabel->setText(package->version());
    else
        ui->versionLabel->setText(R"(<span style="color:red">)" + package->version() + "</span> ⇒ " + availableUpdate);

    // Disable the tab with files for uninstalled packages
    if (package->isInstalled())
        ui->packageTabsWidget->setTabEnabled(2, true);
    else
        ui->packageTabsWidget->setTabEnabled(2, false);

    // Disable "Open in browser" button for local packages
    if (package->repo() == "local")
        ui->browserButton->setDisabled(true);
    else
        ui->browserButton->setDisabled(false);

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

void MainWindow::on_browserButton_clicked()
{
    QUrl url;
    const Package *package = ui->packagesView->currentPackage();
    if (package->repo() == "aur")
        url = "https://aur.archlinux.org/packages/" + package->name();
    else
        url = "https://www.archlinux.org/packages/" + package->repo() + "/" + package->arch() + "/" + package->name();

    QDesktopServices::openUrl(url);
}

void MainWindow::on_updateButton_clicked()
{
    m_terminal.updateDatabase();
}

void MainWindow::on_reloadButton_clicked()
{
    ui->reloadButton->setEnabled(false);
    ui->packagesView->model()->reloadRepoPackages();
}

void MainWindow::findDepend(QAbstractButton *button)
{
    // Clear filter
    ui->searchPackagesEdit->clear();
    if (ui->searchModeComboBox->currentIndex() != PackagesModel::Repo)
        ui->searchModeComboBox->setCurrentIndex(PackagesModel::Repo);
    else
        on_searchPackagesEdit_returnPressed();

    // Search package in repo first
    const bool found = ui->packagesView->find(button->toolTip());
    if (!found) {
        // Search in AUR
        ui->searchPackagesEdit->setText(button->toolTip());
        ui->searchModeComboBox->setCurrentIndex(PackagesModel::AUR);
    }
}

void MainWindow::selectFirstPackage()
{
    ui->packagesView->setCurrentIndex(ui->packagesView->model()->index(0, 0));
    setStatusBarMessage("Loading installed packages");
}

void MainWindow::showPackagesTab()
{
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::on_reloadHistoryButton_clicked()
{
    const PacmanSettings pacmanSettings;
    QFile historyFile(pacmanSettings.logFile());

    if (!historyFile.exists()) {
        QMessageBox errorBox(QMessageBox::Critical, "Error", "File " + pacmanSettings.logFile() + " does not exist");
        errorBox.exec();
        return;
    }

    if (!historyFile.open(QIODevice::ReadOnly)) {
        QMessageBox errorBox(QMessageBox::Critical, "Error", "Unable to read " + pacmanSettings.logFile());
        errorBox.exec();
        return;
    }

    ui->historyEdit->setPlainText(historyFile.readAll());
    ui->historyEdit->moveCursor(QTextCursor::End);
}

void MainWindow::on_findNextButton_clicked()
{
    searchHistory();
}

void MainWindow::on_findPreviousButton_clicked()
{
    searchHistory(true);
}

void MainWindow::on_openHistoryFolderButton_clicked()
{
    const PacmanSettings pacmanSettings;
    const QFileInfo logFile = pacmanSettings.logFile();
    QDesktopServices::openUrl(logFile.dir().path());
}

void MainWindow::on_applyButton_clicked()
{
    m_terminal.executeTasks();
    ui->tasksView->model()->removeAllTasks();
}

void MainWindow::on_noConfirmCheckBox_toggled(bool checked)
{
    m_terminal.setNoConfirm(checked);
    m_terminal.setTasks(ui->tasksView->model());
    ui->commandsEdit->setPlainText(m_terminal.commands());
}

void MainWindow::processAddingTask()
{
    // Count tasks
    int tasksCount = 0;
    foreach (Task *category, ui->tasksView->model()->categories())
        tasksCount += category->children().size();

    // Show count on tab
    if (tasksCount == 0) {
        ui->tabWidget->setTabText(2, "Tasks");
        ui->applyButton->setEnabled(false);
    } else {
        ui->tabWidget->setTabText(2, "Tasks (" + QString::number(tasksCount) + ")");
        ui->applyButton->setEnabled(true);
    }

    m_terminal.setTasks(ui->tasksView->model());
    ui->commandsEdit->setPlainText(m_terminal.commands());
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
        if (package->isInstalledExplicitly())
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
    auto *depsContentLayout = qobject_cast<QFormLayout *>(ui->depsContentWidget->layout());
    auto *packagesLabel = qobject_cast<QLabel *>(depsContentLayout->itemAt(row, QFormLayout::LabelRole)->widget());
    auto *packagesLayout = qobject_cast<QVBoxLayout *>(depsContentLayout->itemAt(row, QFormLayout::FieldRole)->layout());

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
        auto *button = new QPushButton;
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

void MainWindow::searchHistory(bool backward)
{
    if (ui->searchHistoryEdit->text().isEmpty()) {
        ui->searchHistoryEdit->setStyleSheet("");
        return;
    }

    QTextDocument::FindFlags flags;
    if (backward)
        flags |= QTextDocument::FindBackward;

    // Check pressed buttons
    if (ui->searchCaseButton->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (ui->searchExactButton->isChecked())
        flags |= QTextDocument::FindWholeWords;

    if (ui->historyEdit->find(ui->searchHistoryEdit->text(), flags))
        ui->searchHistoryEdit->setStyleSheet("");
    else
        ui->searchHistoryEdit->setStyleSheet("color:red");
}

void MainWindow::on_afterCompletionComboBox_currentIndexChanged(int index)
{
    auto afterCompletion = static_cast<Terminal::AfterCompletion>(index);
    m_terminal.setAfterTasksCompletion(afterCompletion);
}
